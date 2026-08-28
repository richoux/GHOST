#include "capi/include/ghost_c_api.h"

// GHOST Core Headers
#include "include/variable.hpp"
#include "include/constraint.hpp"
#include "include/objective.hpp"
#include "include/model.hpp"
#include "include/model_builder.hpp"
#include "include/options.hpp"
#include "include/search_unit.hpp"
#include "include/auxiliary_data.hpp"
// Specific GHOST Constraints/Objectives/Algorithms needed
#include "include/global_constraints/linear_equation_eq.hpp"
#include "include/global_constraints/all_different.hpp"
#include "include/algorithms/uniform_variable_heuristic.hpp"
#include "include/algorithms/adaptive_search_variable_candidates_heuristic.hpp"
#include "include/algorithms/adaptive_search_value_heuristic.hpp"
#include "include/algorithms/adaptive_search_error_projection_algorithm.hpp"
// Include others as needed...

// C++ Standard Library Headers
#include <vector>
#include <string>
#include <memory>
#include <map>
#include <exception>
#include <limits> // For numeric_limits
#include <chrono> // For timeout in solve
#include <thread> // If implementing parallel solve within C API
#include <future> // If implementing parallel solve within C API
#include <numeric> // For std::iota

// --- Internal Data Structures ---

// Structure to hold parameters for variable creation
struct VariableParams {
    enum class DomainType { CONTIGUOUS, LIST };
    DomainType type;
    int min_val; // For CONTIGUOUS
    int max_val; // For CONTIGUOUS
    std::vector<int> domain_list; // For LIST
    std::string name;
    // Store initial value index if needed? GHOST Variable constructors take it. Default is 0.
    int initial_value_index = 0;
};

// Forward declarations for the internal objective classes
class InternalLinearMinimizeObjective;
class InternalLinearMaximizeObjective;

// Helper function to create a model from our data
ghost::Model create_model_from_data(
    const std::vector<VariableParams>& variable_params,
    const std::vector<std::shared_ptr<ghost::Constraint>>& constraints,
    const std::shared_ptr<ghost::Objective>& objective,
    bool permutation_problem
) {
    // Create variables from params
    std::vector<ghost::Variable> variables;
    variables.reserve(variable_params.size());

    for (const auto& params : variable_params) {
        if (params.type == VariableParams::DomainType::CONTIGUOUS) {
            size_t domain_size = static_cast<size_t>(params.max_val) - static_cast<size_t>(params.min_val) + 1;
            variables.emplace_back(params.min_val, domain_size, params.initial_value_index, params.name);
        } else { // LIST
            variables.emplace_back(params.domain_list, params.initial_value_index, params.name);
        }
    }

    // Create auxiliary data
    auto auxiliary_data = std::make_shared<ghost::NullAuxiliaryData>();

    // Create a model directly
    return ghost::Model(std::move(variables), constraints, objective, auxiliary_data, permutation_problem);
}


struct GhostSessionData {
    bool permutation_problem = false;
    // Store variable parameters instead of Variable objects
    std::vector<VariableParams> variable_params;
    std::vector<std::shared_ptr<ghost::Constraint>> constraints;
    std::shared_ptr<ghost::Objective> objective; // Can hold InternalLinearMinimizeObjective, InternalLinearMaximizeObjective, or others
    // std::shared_ptr<ghost::AuxiliaryData> auxiliary_data; // Add if needed

    // Mappings (optional but potentially useful)
    // std::map<int, int> c_api_var_id_to_internal_index;

    // Last error message
    std::string last_error_message;

    // Last solution results
    GhostSolutionStatus last_solution_status = GHOST_SOLUTION_STATUS_UNKNOWN;
    double last_objective_value = std::numeric_limits<double>::quiet_NaN();
    double last_sat_error = std::numeric_limits<double>::quiet_NaN();
    std::vector<int> last_solution_values; // Stores final values after solve

    // Internal objective data (if using InternalLinearObjective)
    // Note: objective_var_ids now refer to indices in variable_params
    bool objective_maximize = false;
    std::vector<int> objective_var_ids;
    std::vector<double> objective_coeffs;

    // Helper to set error message safely
    void set_error(const std::string& msg) {
        last_error_message = msg;
    }
};

struct GhostOptionsData {
    ghost::Options options;
    // Store any other option-related state if needed
};

// --- Helper Functions (Internal) ---

// Safely get session data pointer, set error if handle is null
inline GhostSessionData* get_session_data(GhostSessionHandle handle) {
    if (!handle) {
        // Cannot set error message here as we don't have a valid handle
        return nullptr;
    }
    return reinterpret_cast<GhostSessionData*>(handle);
}

// Safely get options data pointer, set error if handle is null
inline GhostOptionsData* get_options_data(GhostOptionsHandle handle) {
    // Note: Options handle can be legitimately NULL in ghost_solve
    if (!handle) {
        return nullptr;
    }
    return reinterpret_cast<GhostOptionsData*>(handle);
}

// --- Internal Objective Classes Definition ---

// Linear minimization objective managed internally by the C API
class InternalLinearMinimizeObjective : public ghost::Minimize {
private:
    const GhostSessionData* session_data; // Non-owning pointer to access coeffs/ids

public:
    InternalLinearMinimizeObjective(const std::vector<int>& var_indices, const GhostSessionData* data)
        : ghost::Minimize(var_indices, "InternalLinearMinimizeObjective"), session_data(data) {
        fprintf(stderr, "DEBUG: Created InternalLinearMinimizeObjective with %zu variables\n", var_indices.size());
    }

    double required_cost(const std::vector<ghost::Variable*>& current_variables) const override {
        fprintf(stderr, "DEBUG: required_cost called with %zu variables\n", current_variables.size());

        if (!session_data) {
            fprintf(stderr, "DEBUG: session_data is null\n");
            return std::numeric_limits<double>::quiet_NaN(); // Should not happen
        }

        double cost = 0.0;

        // Map from variable ID to its index in current_variables
        std::map<int, size_t> var_indices;
        for (size_t i = 0; i < current_variables.size(); ++i) {
            if (current_variables[i]) {
                int id = current_variables[i]->get_id();
                var_indices[id] = i;
                fprintf(stderr, "DEBUG: Mapped variable ID %d to index %zu\n", id, i);
            } else {
                fprintf(stderr, "DEBUG: current_variables[%zu] is null\n", i);
            }
        }

        // Calculate cost using the mapped indices
        for (size_t i = 0; i < session_data->objective_var_ids.size(); ++i) {
            int var_id = session_data->objective_var_ids[i];
            double coeff = session_data->objective_coeffs[i];
            fprintf(stderr, "DEBUG: Looking for variable ID %d with coefficient %f\n", var_id, coeff);

            auto it = var_indices.find(var_id);
            if (it != var_indices.end()) {
                size_t idx = it->second;
                if (idx < current_variables.size()) {
                    if (current_variables[idx]) {
                        int value = current_variables[idx]->get_value();
                        cost += coeff * value;
                        fprintf(stderr, "DEBUG: Added %f * %d = %f to cost\n", coeff, value, coeff * value);
                    } else {
                        fprintf(stderr, "DEBUG: current_variables[%zu] is null\n", idx);
                        return std::numeric_limits<double>::quiet_NaN();
                    }
                } else {
                    // This indicates an internal inconsistency
                    fprintf(stderr, "DEBUG: Index %zu out of bounds for current_variables size %zu\n", idx, current_variables.size());
                    return std::numeric_limits<double>::quiet_NaN();
                }
            } else {
                // Variable ID from objective data not found in objective's scope - error
                fprintf(stderr, "DEBUG: Variable ID %d not found in var_indices\n", var_id);
                return std::numeric_limits<double>::quiet_NaN();
            }
        }
        fprintf(stderr, "DEBUG: Final cost: %f\n", cost);
        return cost;
    }
};

// Linear maximization objective managed internally by the C API
class InternalLinearMaximizeObjective : public ghost::Maximize {
private:
    const GhostSessionData* session_data; // Non-owning pointer to access coeffs/ids

public:
    InternalLinearMaximizeObjective(const std::vector<int>& var_indices, const GhostSessionData* data)
        : ghost::Maximize(var_indices, "InternalLinearMaximizeObjective"), session_data(data) {
        fprintf(stderr, "DEBUG: Created InternalLinearMaximizeObjective with %zu variables\n", var_indices.size());
    }

    double required_cost(const std::vector<ghost::Variable*>& current_variables) const override {
        fprintf(stderr, "DEBUG: required_cost called with %zu variables\n", current_variables.size());

        if (!session_data) {
            fprintf(stderr, "DEBUG: session_data is null\n");
            return std::numeric_limits<double>::quiet_NaN(); // Should not happen
        }

        double cost = 0.0;

        // Map from variable ID to its index in current_variables
        std::map<int, size_t> var_indices;
        for (size_t i = 0; i < current_variables.size(); ++i) {
            if (current_variables[i]) {
                int id = current_variables[i]->get_id();
                var_indices[id] = i;
                fprintf(stderr, "DEBUG: Mapped variable ID %d to index %zu\n", id, i);
            } else {
                fprintf(stderr, "DEBUG: current_variables[%zu] is null\n", i);
            }
        }

        // Calculate cost using the mapped indices
        for (size_t i = 0; i < session_data->objective_var_ids.size(); ++i) {
            int var_id = session_data->objective_var_ids[i];
            double coeff = session_data->objective_coeffs[i];
            fprintf(stderr, "DEBUG: Looking for variable ID %d with coefficient %f\n", var_id, coeff);

            auto it = var_indices.find(var_id);
            if (it != var_indices.end()) {
                size_t idx = it->second;
                if (idx < current_variables.size()) {
                    if (current_variables[idx]) {
                        int value = current_variables[idx]->get_value();
                        cost += coeff * value;
                        fprintf(stderr, "DEBUG: Added %f * %d = %f to cost\n", coeff, value, coeff * value);
                    } else {
                        fprintf(stderr, "DEBUG: current_variables[%zu] is null\n", idx);
                        return std::numeric_limits<double>::quiet_NaN();
                    }
                } else {
                    // This indicates an internal inconsistency
                    fprintf(stderr, "DEBUG: Index %zu out of bounds for current_variables size %zu\n", idx, current_variables.size());
                    return std::numeric_limits<double>::quiet_NaN();
                }
            } else {
                // Variable ID from objective data not found in objective's scope - error
                fprintf(stderr, "DEBUG: Variable ID %d not found in var_indices\n", var_id);
                return std::numeric_limits<double>::quiet_NaN();
            }
        }
        fprintf(stderr, "DEBUG: Final cost: %f\n", cost);
        return cost;
    }
};


// --- API Implementation ---

extern "C" {

// === Session Management ===

GhostSessionHandle ghost_create_session(bool permutation_problem) {
    try {
        GhostSessionData* data = new GhostSessionData();
        data->permutation_problem = permutation_problem;
        return reinterpret_cast<GhostSessionHandle>(data);
    } catch (const std::bad_alloc&) {
        // Cannot set error message as handle creation failed
        return nullptr;
    } catch (...) {
        // Catch any other unexpected C++ exception during creation
        return nullptr;
    }
}

void ghost_destroy_session(GhostSessionHandle handle) {
    if (handle) {
        delete reinterpret_cast<GhostSessionData*>(handle);
    }
}

const char* ghost_get_last_error(GhostSessionHandle handle) {
    GhostSessionData* data = get_session_data(handle);
    if (!data) {
        // Or return a static string like "Invalid handle"?
        return nullptr;
    }
    // Return NULL if no error message is set
    return data->last_error_message.empty() ? nullptr : data->last_error_message.c_str();
}

// === Variable Addition ===

int ghost_add_variable(GhostSessionHandle handle, int min_val, int max_val, const char* name) {
    GhostSessionData* data = get_session_data(handle);
    if (!data) return GHOST_ERROR_NULL_HANDLE;
    data->set_error(""); // Clear previous error

    if (min_val > max_val) {
        data->set_error("Invalid domain: min_val cannot be greater than max_val.");
        return GHOST_ERROR_INVALID_ARG;
    }

    try {
        // Store parameters instead of creating Variable object
        VariableParams params;
        params.type = VariableParams::DomainType::CONTIGUOUS;
        params.min_val = min_val;
        params.max_val = max_val;
        params.name = (name != nullptr) ? name : "";
        params.initial_value_index = 0; // Default initial value index

        data->variable_params.push_back(params);

        // The ID returned by the C API will be the index in the variable_params vector
        int var_id = static_cast<int>(data->variable_params.size()) - 1;
        return var_id;

    } catch (const std::bad_alloc&) {
        data->set_error("Memory allocation failed while storing variable parameters.");
        return GHOST_ERROR_MEMORY;
    } catch (const std::exception& e) {
        data->set_error("GHOST C++ exception during variable parameter storage: " + std::string(e.what()));
        return GHOST_ERROR_UNKNOWN;
    } catch (...) {
        data->set_error("Unknown C++ exception during variable parameter storage.");
        return GHOST_ERROR_UNKNOWN;
    }
}

int ghost_add_variable_domain(GhostSessionHandle handle, const int* domain_values, size_t domain_size, const char* name) {
    GhostSessionData* data = get_session_data(handle);
    if (!data) return GHOST_ERROR_NULL_HANDLE;
    data->set_error(""); // Clear previous error

    if (domain_size > 0 && !domain_values) {
        data->set_error("domain_values cannot be NULL if domain_size > 0.");
        return GHOST_ERROR_INVALID_ARG;
    }

    try {
        // Store parameters instead of creating Variable object
        VariableParams params;
        params.type = VariableParams::DomainType::LIST;
        if (domain_size > 0) {
            params.domain_list.assign(domain_values, domain_values + domain_size);
        }

        if (params.domain_list.empty()) {
             data->set_error("Variable domain cannot be empty.");
             return GHOST_ERROR_INVALID_ARG;
        }

        params.name = (name != nullptr) ? name : "";
        params.initial_value_index = 0; // Default initial value index

        data->variable_params.push_back(params);

        // The ID returned by the C API will be the index in the variable_params vector
        int var_id = static_cast<int>(data->variable_params.size()) - 1;
        return var_id;

    } catch (const std::bad_alloc&) {
        data->set_error("Memory allocation failed while storing variable parameters with custom domain.");
        return GHOST_ERROR_MEMORY;
    } catch (const std::exception& e) {
        data->set_error("GHOST C++ exception during variable parameter storage: " + std::string(e.what()));
        return GHOST_ERROR_UNKNOWN;
    } catch (...) {
        data->set_error("Unknown C++ exception during variable parameter storage.");
        return GHOST_ERROR_UNKNOWN;
    }
}


// === Constraint Addition ===

// Helper to validate variable IDs
inline bool validate_var_ids(GhostSessionData* data, const int* var_ids, size_t num_vars) {
    if (num_vars > 0 && !var_ids) {
        data->set_error("var_ids cannot be NULL if num_vars > 0.");
        return false;
    }
    // Validate against the size of variable_params now
    for (size_t i = 0; i < num_vars; ++i) {
        if (var_ids[i] < 0 || static_cast<size_t>(var_ids[i]) >= data->variable_params.size()) {
            data->set_error("Invalid variable ID (index) provided: " + std::to_string(var_ids[i]));
            return false;
        }
    }
    return true;
}

int ghost_add_linear_eq_constraint(GhostSessionHandle handle, const int* var_ids, const double* coeffs, size_t num_vars, double rhs) {
    GhostSessionData* data = get_session_data(handle);
    if (!data) return GHOST_ERROR_NULL_HANDLE;
    data->set_error(""); // Clear previous error

    if (!validate_var_ids(data, var_ids, num_vars)) {
        return GHOST_ERROR_INVALID_ID;
    }
    // Note: coeffs can be NULL, constructor handles this.

    try {
        std::vector<int> var_indices_vec;
        if (num_vars > 0) {
            var_indices_vec.assign(var_ids, var_ids + num_vars);
        }

        std::shared_ptr<ghost::Constraint> constraint_ptr;
        if (coeffs) {
            std::vector<double> coeffs_vec(coeffs, coeffs + num_vars);
            // Use constructor: LinearEquationEq(const std::vector<int>& variables_index, double rhs, const std::vector<double>& coefficients)
            constraint_ptr = std::make_shared<ghost::global_constraints::LinearEquationEq>(var_indices_vec, rhs, coeffs_vec);
        } else {
            // Use constructor: LinearEquationEq(const std::vector<int>& variables_index, double rhs)
             constraint_ptr = std::make_shared<ghost::global_constraints::LinearEquationEq>(var_indices_vec, rhs);
        }

        data->constraints.push_back(constraint_ptr);
        int constraint_id = static_cast<int>(data->constraints.size()) - 1;
        return constraint_id;

    } catch (const std::bad_alloc&) {
        data->set_error("Memory allocation failed while adding linear equality constraint.");
        return GHOST_ERROR_MEMORY;
    } catch (const std::exception& e) {
        data->set_error("GHOST C++ exception during constraint creation: " + std::string(e.what()));
        return GHOST_ERROR_UNKNOWN;
    } catch (...) {
        data->set_error("Unknown C++ exception during constraint creation.");
        return GHOST_ERROR_UNKNOWN;
    }
}

int ghost_add_alldifferent_constraint(GhostSessionHandle handle, const int* var_ids, size_t num_vars) {
    GhostSessionData* data = get_session_data(handle);
    if (!data) return GHOST_ERROR_NULL_HANDLE;
    data->set_error(""); // Clear previous error

    if (!validate_var_ids(data, var_ids, num_vars)) {
        return GHOST_ERROR_INVALID_ID;
    }
     if (num_vars == 0) {
        // An AllDifferent constraint with no variables is trivially satisfied, but maybe warn or disallow?
        // Let's allow it for now, GHOST might handle it gracefully.
    }

    try {
        // Use the var_ids directly as the variable indices (indices into variable_params)
        std::vector<int> var_indices_vec;
        if (num_vars > 0) {
            var_indices_vec.assign(var_ids, var_ids + num_vars);
        }

        // Use constructor: AllDifferent(const std::vector<int>& variables_index)
        auto constraint_ptr = std::make_shared<ghost::global_constraints::AllDifferent>(var_indices_vec);

        data->constraints.push_back(constraint_ptr);
        int constraint_id = static_cast<int>(data->constraints.size()) - 1;
        return constraint_id;

    } catch (const std::bad_alloc&) {
        data->set_error("Memory allocation failed while adding alldifferent constraint.");
        return GHOST_ERROR_MEMORY;
    } catch (const std::exception& e) {
        data->set_error("GHOST C++ exception during constraint creation: " + std::string(e.what()));
        return GHOST_ERROR_UNKNOWN;
    } catch (...) {
        data->set_error("Unknown C++ exception during constraint creation.");
        return GHOST_ERROR_UNKNOWN;
    }
}


// === Objective Setting ===

GhostStatus ghost_set_linear_objective(GhostSessionHandle handle, bool maximize, const int* var_ids, const double* coeffs, size_t num_vars) {
    GhostSessionData* data = get_session_data(handle);
    if (!data) return GHOST_ERROR_NULL_HANDLE;
    data->set_error(""); // Clear previous error

    if (!validate_var_ids(data, var_ids, num_vars)) {
        return GHOST_ERROR_INVALID_ID;
    }
    if (num_vars > 0 && !coeffs) {
         data->set_error("coeffs cannot be NULL if num_vars > 0 for objective.");
        return GHOST_ERROR_INVALID_ARG;
    }

    try {
        fprintf(stderr, "DEBUG: Setting linear objective with maximize=%d, num_vars=%zu\n", maximize, num_vars);

        // Store objective details for the internal objective classes
        data->objective_maximize = maximize;
        data->objective_var_ids.clear();
        data->objective_coeffs.clear();

        for (size_t i = 0; i < num_vars; ++i) {
            fprintf(stderr, "DEBUG: Adding variable ID %d with coefficient %f\n", var_ids[i], coeffs[i]);
            data->objective_var_ids.push_back(var_ids[i]);
            data->objective_coeffs.push_back(coeffs[i]);
        }

        // Create the internal objective object.
        std::vector<int> objective_scope_indices(var_ids, var_ids + num_vars);

        // Create either a minimize or maximize objective based on the flag
        if (maximize) {
            fprintf(stderr, "DEBUG: Creating InternalLinearMaximizeObjective\n");
            data->objective = std::make_shared<InternalLinearMaximizeObjective>(objective_scope_indices, data);
        } else {
            fprintf(stderr, "DEBUG: Creating InternalLinearMinimizeObjective\n");
            data->objective = std::make_shared<InternalLinearMinimizeObjective>(objective_scope_indices, data);
        }

        return GHOST_SUCCESS;

    } catch (const std::bad_alloc&) {
        data->set_error("Memory allocation failed while setting linear objective.");
        return GHOST_ERROR_MEMORY;
    } catch (const std::exception& e) {
        data->set_error("GHOST C++ exception during objective setting: " + std::string(e.what()));
        return GHOST_ERROR_UNKNOWN;
    } catch (...) {
        data->set_error("Unknown C++ exception during objective setting.");
        return GHOST_ERROR_UNKNOWN;
    }
}


// === Options Management ===

GhostOptionsHandle ghost_create_options() {
    try {
        GhostOptionsData* data = new GhostOptionsData();
        // Options struct has a default constructor, so no need to set defaults here
        return reinterpret_cast<GhostOptionsHandle>(data);
    } catch (const std::bad_alloc&) {
        // Cannot set error message as we don't have a handle to store it
        return nullptr;
    } catch (...) {
        return nullptr;
    }
}

void ghost_destroy_options(GhostOptionsHandle options_handle) {
    if (options_handle) {
        delete reinterpret_cast<GhostOptionsData*>(options_handle);
    }
}

GhostStatus ghost_set_option_parallel(GhostOptionsHandle options_handle, bool parallel) {
    GhostOptionsData* data = get_options_data(options_handle);
    if (!data) return GHOST_ERROR_NULL_HANDLE;

    try {
        data->options.parallel_runs = parallel;
        return GHOST_SUCCESS;
    } catch (...) {
        // No way to set error message on options handle
        return GHOST_ERROR_UNKNOWN;
    }
}

GhostStatus ghost_set_option_num_threads(GhostOptionsHandle options_handle, int num_threads) {
    GhostOptionsData* data = get_options_data(options_handle);
    if (!data) return GHOST_ERROR_NULL_HANDLE;

    try {
        data->options.number_threads = num_threads;
        return GHOST_SUCCESS;
    } catch (...) {
        // No way to set error message on options handle
        return GHOST_ERROR_UNKNOWN;
    }
}


// === Solving ===

GhostStatus ghost_solve(GhostSessionHandle handle, GhostOptionsHandle options_handle, double timeout_microseconds) {
    GhostSessionData* data = get_session_data(handle);
    if (!data) return GHOST_ERROR_NULL_HANDLE;
    data->set_error(""); // Clear previous error

    if (timeout_microseconds <= 0) {
        data->set_error("Timeout must be positive.");
        return GHOST_ERROR_INVALID_ARG;
    }

    // Check variable_params size now
    if (data->variable_params.empty()) {
        data->set_error("No variables defined in the model.");
        return GHOST_ERROR_API_USAGE;
    }

    try {
        fprintf(stderr, "DEBUG: Starting ghost_solve with %zu variables (from params), %zu constraints\n",
                data->variable_params.size(), data->constraints.size());

        // Check if we have an objective
        if (data->objective) {
            fprintf(stderr, "DEBUG: Objective is set, name: %s\n", data->objective->get_name().c_str());
        } else {
            fprintf(stderr, "DEBUG: No objective set\n");
        }

        // Prepare the objective (use NullObjective if none provided)
        std::shared_ptr<ghost::Objective> objective_to_use;
        if (data->objective) {
            objective_to_use = data->objective;
            fprintf(stderr, "DEBUG: Using provided objective\n");
        } else {
            objective_to_use = std::make_shared<ghost::NullObjective>();
            fprintf(stderr, "DEBUG: Using NullObjective since no objective was provided\n");
        }

        // Create a model directly from our data
        fprintf(stderr, "DEBUG: Creating model directly from data\n");
        ghost::Model model = create_model_from_data(
            data->variable_params,
            data->constraints,
            objective_to_use,
            data->permutation_problem
        );

        fprintf(stderr, "DEBUG: Created ghost::Model\n");

        // Get options (use default if not provided)
        ghost::Options options;
        if (options_handle) {
            GhostOptionsData* options_data = get_options_data(options_handle);
            if (options_data) {
                options = options_data->options;
                fprintf(stderr, "DEBUG: Using provided options\n");
            } else {
                fprintf(stderr, "DEBUG: options_data is null, using default options\n");
            }
        } else {
            fprintf(stderr, "DEBUG: options_handle is null, using default options\n");
        }

        fprintf(stderr, "DEBUG: Creating SearchUnit\n");
        // Create a SearchUnit directly (bypassing the templated Solver class)
        ghost::SearchUnit search_unit(
            std::move(model),
            options,
            std::make_unique<ghost::algorithms::UniformVariableHeuristic>(),
            std::make_unique<ghost::algorithms::AdaptiveSearchVariableCandidatesHeuristic>(),
            std::make_unique<ghost::algorithms::AdaptiveSearchValueHeuristic>(),
            std::make_unique<ghost::algorithms::AdaptiveSearchErrorProjection>()
        );

        fprintf(stderr, "DEBUG: Created SearchUnit\n");

        // Get a future for the solution status
        std::future<bool> solution_future = search_unit.solution_found.get_future();

        fprintf(stderr, "DEBUG: Running local_search with timeout %f microseconds\n", timeout_microseconds);

        // Run the search
        search_unit.local_search(timeout_microseconds);

        fprintf(stderr, "DEBUG: local_search completed\n");

        // Get the solution status
        bool solution_found = solution_future.get();

        fprintf(stderr, "DEBUG: solution_found = %d\n", solution_found);

        // Store the results in the session data
        data->last_sat_error = search_unit.data.best_sat_error;
        fprintf(stderr, "DEBUG: last_sat_error = %f\n", data->last_sat_error);

        if (search_unit.data.is_optimization) {
            data->last_objective_value = search_unit.data.best_opt_cost;
            fprintf(stderr, "DEBUG: last_objective_value = %f\n", data->last_objective_value);
        }

        fprintf(stderr, "DEBUG: Transferring final model state back from SearchUnit\n");
        // Move the model back from the search unit to get the final variable state
        ghost::Model final_model = search_unit.transfer_model();

        // Store the solution values from the final model state
        data->last_solution_values.clear();
        data->last_solution_values.reserve(final_model.variables.size());
        fprintf(stderr, "DEBUG: Storing final solution values:\n");
        for (const auto& var : final_model.variables) {
            // Use the variable's ID (which should now be correctly set by SearchUnit) if needed,
            // but for storing results, the order is usually sufficient.
            data->last_solution_values.push_back(var.get_value());
            fprintf(stderr, "DEBUG:   Var ID %d = %d\n", var.get_id(), var.get_value());
        }
        // Note: data->variables (the vector of VariableParams) remains unchanged.
        // We only store the *results* in last_solution_values.

        // Set the solution status
        if (solution_found) {
            if (search_unit.data.is_optimization) {
                data->last_solution_status = GHOST_SOLUTION_STATUS_FEASIBLE;
                fprintf(stderr, "DEBUG: Solution status: FEASIBLE\n");
                return GHOST_FEASIBLE_FOUND;
            } else {
                data->last_solution_status = GHOST_SOLUTION_STATUS_SAT;
                fprintf(stderr, "DEBUG: Solution status: SAT\n");
                return GHOST_SAT_FOUND;
            }
        } else {
            data->last_solution_status = GHOST_SOLUTION_STATUS_INFEASIBLE;
            fprintf(stderr, "DEBUG: Solution status: INFEASIBLE\n");
            return GHOST_INFEASIBLE;
        }

    } catch (const std::bad_alloc&) {
        data->set_error("Memory allocation failed during solving.");
        return GHOST_ERROR_MEMORY;
    } catch (const std::exception& e) {
        data->set_error("GHOST C++ exception during solving: " + std::string(e.what()));
        return GHOST_ERROR_SOLVER;
    } catch (...) {
        data->set_error("Unknown C++ exception during solving.");
        return GHOST_ERROR_SOLVER;
    }
}


// === Result Querying ===

GhostSolutionStatus ghost_get_solution_status(GhostSessionHandle handle) {
    GhostSessionData* data = get_session_data(handle);
    if (!data) return GHOST_SOLUTION_STATUS_UNKNOWN;

    return data->last_solution_status;
}

GhostStatus ghost_get_variable_value(GhostSessionHandle handle, int var_id, int* value_ptr) {
    GhostSessionData* data = get_session_data(handle);
    if (!data) return GHOST_ERROR_NULL_HANDLE;
    data->set_error(""); // Clear previous error

    if (!value_ptr) {
        data->set_error("value_ptr cannot be NULL.");
        return GHOST_ERROR_INVALID_ARG;
    }

    // Validate var_id against the number of results stored
    if (var_id < 0 || static_cast<size_t>(var_id) >= data->last_solution_values.size()) {
        data->set_error("Invalid variable ID or no solution values available for ID: " + std::to_string(var_id));
        return GHOST_ERROR_INVALID_ID;
    }

    if (data->last_solution_status == GHOST_SOLUTION_STATUS_UNKNOWN) {
        data->set_error("No solution available. Call ghost_solve first.");
        return GHOST_ERROR_API_USAGE;
    }

    try {
        // Retrieve value from the stored results vector
        *value_ptr = data->last_solution_values[var_id];
        return GHOST_SUCCESS;
    } catch (const std::exception& e) {
        // Catch potential out_of_range, though size check should prevent it
        data->set_error("C++ exception while getting stored variable value: " + std::string(e.what()));
        return GHOST_ERROR_UNKNOWN;
    } catch (...) {
        data->set_error("Unknown C++ exception while getting variable value.");
        return GHOST_ERROR_UNKNOWN;
    }
}

GhostStatus ghost_get_objective_value(GhostSessionHandle handle, double* objective_value_ptr) {
    GhostSessionData* data = get_session_data(handle);
    if (!data) return GHOST_ERROR_NULL_HANDLE;
    data->set_error(""); // Clear previous error

    if (!objective_value_ptr) {
        data->set_error("objective_value_ptr cannot be NULL.");
        return GHOST_ERROR_INVALID_ARG;
    }

    if (data->last_solution_status == GHOST_SOLUTION_STATUS_UNKNOWN) {
        data->set_error("No solution available. Call ghost_solve first.");
        return GHOST_ERROR_API_USAGE;
    }

    if (!data->objective) {
        data->set_error("No objective function defined.");
        return GHOST_ERROR_API_USAGE;
    }

    try {
        *objective_value_ptr = data->last_objective_value;
        return GHOST_SUCCESS;
    } catch (const std::exception& e) {
        data->set_error("GHOST C++ exception while getting objective value: " + std::string(e.what()));
        return GHOST_ERROR_UNKNOWN;
    } catch (...) {
        data->set_error("Unknown C++ exception while getting objective value.");
        return GHOST_ERROR_UNKNOWN;
    }
}

GhostStatus ghost_get_variable_values(GhostSessionHandle handle, int* values_buffer, size_t buffer_size) {
    GhostSessionData* data = get_session_data(handle);
    if (!data) return GHOST_ERROR_NULL_HANDLE;
    data->set_error(""); // Clear previous error

    if (!values_buffer) {
        data->set_error("values_buffer cannot be NULL.");
        return GHOST_ERROR_INVALID_ARG;
    }

    if (data->last_solution_status == GHOST_SOLUTION_STATUS_UNKNOWN) {
        data->set_error("No solution available. Call ghost_solve first.");
        return GHOST_ERROR_API_USAGE;
    }

    // Check buffer size against the number of stored results
    if (buffer_size < data->last_solution_values.size()) {
        data->set_error("Buffer too small. Need at least " + std::to_string(data->last_solution_values.size()) + " elements.");
        return GHOST_ERROR_INVALID_ARG;
    }

    try {
        // Copy values from the stored results vector
        std::copy(data->last_solution_values.begin(), data->last_solution_values.end(), values_buffer);
        return GHOST_SUCCESS;
    } catch (const std::exception& e) {
        data->set_error("C++ exception while copying variable values: " + std::string(e.what()));
        return GHOST_ERROR_UNKNOWN;
    } catch (...) {
        data->set_error("Unknown C++ exception while getting variable values.");
        return GHOST_ERROR_UNKNOWN;
    }
}

} // extern "C"
