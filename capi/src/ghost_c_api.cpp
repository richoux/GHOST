#include "capi/include/ghost_c_api.h"

// GHOST Core Headers
#include "include/variable.hpp"
#include "include/constraint.hpp"
#include "include/objective.hpp"
#include "include/model.hpp"
#include "include/options.hpp"
#include "include/search_unit.hpp"
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

// Forward declaration for the internal objective class
class InternalLinearObjective;

struct GhostSessionData {
    bool permutation_problem = false;
    std::vector<ghost::Variable> variables;
    std::vector<std::shared_ptr<ghost::Constraint>> constraints;
    std::shared_ptr<ghost::Objective> objective; // Can hold InternalLinearObjective or others
    // std::shared_ptr<ghost::AuxiliaryData> auxiliary_data; // Add if needed

    // Mappings (optional but potentially useful)
    // std::map<int, int> c_api_var_id_to_internal_index;

    // Last error message
    std::string last_error_message;

    // Last solution results
    GhostSolutionStatus last_solution_status = GHOST_SOLUTION_STATUS_UNKNOWN;
    double last_objective_value = std::numeric_limits<double>::quiet_NaN();
    double last_sat_error = std::numeric_limits<double>::quiet_NaN();
    std::vector<int> last_solution_values;

    // Internal objective data (if using InternalLinearObjective)
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

// --- Internal Objective Class Definition ---

// Example for a linear objective managed internally by the C API
class InternalLinearObjective : public ghost::Objective {
private:
    const GhostSessionData* session_data; // Non-owning pointer to access coeffs/ids

public:
    InternalLinearObjective(const std::vector<int>& var_indices, bool maximize, const GhostSessionData* data)
        : ghost::Objective(var_indices, maximize, "InternalLinearObjective"), session_data(data) {}

    double required_cost(const std::vector<ghost::Variable*>& current_variables) const override {
        if (!session_data) return std::numeric_limits<double>::quiet_NaN(); // Should not happen

        double cost = 0.0;
        // Assuming session_data->objective_var_ids contains the *original* C API IDs
        // and session_data->objective_coeffs contains the corresponding coefficients.
        // We need to map the C API IDs to the indices within current_variables.
        // The `_variables_position` map in the base Objective class helps here.
        // It maps the *global* variable ID (from the model) to the *local* index within the objective's scope.

        for (size_t i = 0; i < session_data->objective_var_ids.size(); ++i) {
            int global_var_id = session_data->objective_var_ids[i];
            double coeff = session_data->objective_coeffs[i];

            // Find the local index for this global variable ID within the objective's scope
            auto it = _variables_position.find(global_var_id);
            if (it != _variables_position.end()) {
                int local_index = it->second;
                if (local_index >= 0 && local_index < current_variables.size()) {
                     cost += coeff * current_variables[local_index]->get_value();
                } else {
                     // This indicates an internal inconsistency
                     return std::numeric_limits<double>::quiet_NaN();
                }
            } else {
                 // Variable ID from objective data not found in objective's scope - error
                 return std::numeric_limits<double>::quiet_NaN();
            }
        }
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
        // Calculate size for contiguous domain
        // Need to handle potential overflow if max_val - min_val is huge, though unlikely for int
        size_t domain_size = static_cast<size_t>(max_val) - static_cast<size_t>(min_val) + 1;
        std::string var_name = (name != nullptr) ? name : "";

        // Use the constructor: Variable(int starting_value, std::size_t size, int index = 0, const std::string& name = std::string())
        // We use index 0 by default.
        data->variables.emplace_back(min_val, domain_size, 0, var_name);

        // The ID returned by the C API will be the index in the vector
        int var_id = static_cast<int>(data->variables.size()) - 1;
        return var_id;

    } catch (const std::bad_alloc&) {
        data->set_error("Memory allocation failed while adding variable.");
        return GHOST_ERROR_MEMORY;
    } catch (const std::exception& e) {
        data->set_error("GHOST C++ exception during variable creation: " + std::string(e.what()));
        return GHOST_ERROR_UNKNOWN;
    } catch (...) {
        data->set_error("Unknown C++ exception during variable creation.");
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
        std::vector<int> domain_vec;
        if (domain_size > 0) {
            domain_vec.assign(domain_values, domain_values + domain_size);
        }
        // GHOST Variable constructor requires a non-empty domain.
        // If domain_size is 0, should we error out or create a variable that can never be satisfied?
        // Let's error out for now, as an empty domain seems problematic.
        if (domain_vec.empty()) {
             data->set_error("Variable domain cannot be empty.");
             return GHOST_ERROR_INVALID_ARG;
        }

        std::string var_name = (name != nullptr) ? name : "";

        // Use the constructor: Variable(const std::vector<int>& domain, int index = 0, const std::string& name = std::string())
        // We use index 0 by default.
        data->variables.emplace_back(domain_vec, 0, var_name);

        // The ID returned by the C API will be the index in the vector
        int var_id = static_cast<int>(data->variables.size()) - 1;
        return var_id;

    } catch (const std::bad_alloc&) {
        data->set_error("Memory allocation failed while adding variable with custom domain.");
        return GHOST_ERROR_MEMORY;
    } catch (const std::exception& e) {
        data->set_error("GHOST C++ exception during variable creation: " + std::string(e.what()));
        return GHOST_ERROR_UNKNOWN;
    } catch (...) {
        data->set_error("Unknown C++ exception during variable creation.");
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
    for (size_t i = 0; i < num_vars; ++i) {
        if (var_ids[i] < 0 || static_cast<size_t>(var_ids[i]) >= data->variables.size()) {
            data->set_error("Invalid variable ID provided: " + std::to_string(var_ids[i]));
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
        // Store objective details for InternalLinearObjective::required_cost
        data->objective_maximize = maximize;
        data->objective_var_ids.assign(var_ids, var_ids + num_vars);
        data->objective_coeffs.assign(coeffs, coeffs + num_vars);

        // Create the internal objective object.
        // The base ghost::Objective constructor needs the indices of the variables involved.
        std::vector<int> objective_scope_indices(var_ids, var_ids + num_vars);

        // Use make_shared to create the objective instance
        data->objective = std::make_shared<InternalLinearObjective>(objective_scope_indices, maximize, data);

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

    if (data->variables.empty()) {
        data->set_error("No variables defined in the model.");
        return GHOST_ERROR_API_USAGE;
    }

    try {
        // Create a ghost::Model from the components in the session data
        ghost::Model model(
            std::move(data->variables),  // Move variables into the model
            data->constraints,           // Share constraints
            data->objective,             // Share objective
            nullptr,                     // No auxiliary data for now
            data->permutation_problem    // Permutation flag
        );

        // Get options (use default if not provided)
        ghost::Options options;
        if (options_handle) {
            GhostOptionsData* options_data = get_options_data(options_handle);
            if (options_data) {
                options = options_data->options;
            }
        }

        // Create a SearchUnit directly (bypassing the templated Solver class)
        ghost::SearchUnit search_unit(
            std::move(model),
            options,
            std::make_unique<ghost::algorithms::UniformVariableHeuristic>(),
            std::make_unique<ghost::algorithms::AdaptiveSearchVariableCandidatesHeuristic>(),
            std::make_unique<ghost::algorithms::AdaptiveSearchValueHeuristic>(),
            std::make_unique<ghost::algorithms::AdaptiveSearchErrorProjection>()
        );

        // Get a future for the solution status
        std::future<bool> solution_future = search_unit.solution_found.get_future();

        // Run the search
        search_unit.local_search(timeout_microseconds);

        // Get the solution status
        bool solution_found = solution_future.get();

        // Store the results in the session data
        data->last_sat_error = search_unit.data.best_sat_error;
        if (search_unit.data.is_optimization) {
            data->last_objective_value = search_unit.data.best_opt_cost;
        }

        // Move the model back to the session data to access the variable values
        data->variables = std::move(search_unit.transfer_model().variables);

        // Store the solution values
        data->last_solution_values.clear();
        data->last_solution_values.reserve(data->variables.size());
        for (const auto& var : data->variables) {
            data->last_solution_values.push_back(var.get_value());
        }

        // Set the solution status
        if (solution_found) {
            if (search_unit.data.is_optimization) {
                data->last_solution_status = GHOST_SOLUTION_STATUS_FEASIBLE;
                return GHOST_FEASIBLE_FOUND;
            } else {
                data->last_solution_status = GHOST_SOLUTION_STATUS_SAT;
                return GHOST_SAT_FOUND;
            }
        } else {
            data->last_solution_status = GHOST_SOLUTION_STATUS_INFEASIBLE;
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

    if (var_id < 0 || static_cast<size_t>(var_id) >= data->variables.size()) {
        data->set_error("Invalid variable ID: " + std::to_string(var_id));
        return GHOST_ERROR_INVALID_ID;
    }

    if (data->last_solution_status == GHOST_SOLUTION_STATUS_UNKNOWN) {
        data->set_error("No solution available. Call ghost_solve first.");
        return GHOST_ERROR_API_USAGE;
    }

    try {
        *value_ptr = data->variables[var_id].get_value();
        return GHOST_SUCCESS;
    } catch (const std::exception& e) {
        data->set_error("GHOST C++ exception while getting variable value: " + std::string(e.what()));
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

    if (buffer_size < data->variables.size()) {
        data->set_error("Buffer too small. Need at least " + std::to_string(data->variables.size()) + " elements.");
        return GHOST_ERROR_INVALID_ARG;
    }

    try {
        for (size_t i = 0; i < data->variables.size(); ++i) {
            values_buffer[i] = data->variables[i].get_value();
        }
        return GHOST_SUCCESS;
    } catch (const std::exception& e) {
        data->set_error("GHOST C++ exception while getting variable values: " + std::string(e.what()));
        return GHOST_ERROR_UNKNOWN;
    } catch (...) {
        data->set_error("Unknown C++ exception while getting variable values.");
        return GHOST_ERROR_UNKNOWN;
    }
}

} // extern "C"
