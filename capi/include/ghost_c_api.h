#ifndef GHOST_C_API_H
#define GHOST_C_API_H

#include <stdbool.h> // For bool type
#include <stddef.h>  // For size_t type

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Opaque handle representing a GHOST modeling and solving session.
 */
struct GhostSessionHandle_t;
typedef struct GhostSessionHandle_t* GhostSessionHandle;

/**
 * @brief Opaque handle representing GHOST solver options.
 */
struct GhostOptionsHandle_t;
typedef struct GhostOptionsHandle_t* GhostOptionsHandle;

/**
 * @brief Status codes returned by GHOST C API functions.
 */
typedef enum {
    GHOST_SUCCESS = 0,          /**< Operation completed successfully. */
    GHOST_SAT_FOUND = 1,        /**< Feasible solution found (satisfaction problem). */
    GHOST_OPTIMAL_FOUND = 2,    /**< Optimal solution found (optimization problem, if provable). */
    GHOST_FEASIBLE_FOUND = 3,   /**< Feasible solution found (optimization problem, may not be optimal). */
    GHOST_INFEASIBLE = -1,      /**< Problem proven infeasible or no solution found within timeout. */
    GHOST_ERROR_UNKNOWN = -2,   /**< An unspecified error occurred. */
    GHOST_ERROR_NULL_HANDLE = -3,/**< A required handle (Session or Options) was NULL. */
    GHOST_ERROR_INVALID_ARG = -4,/**< An invalid argument was provided (e.g., negative size, null pointer). */
    GHOST_ERROR_INVALID_ID = -5, /**< An invalid variable or constraint ID was provided. */
    GHOST_ERROR_MEMORY = -6,    /**< Memory allocation failed. */
    GHOST_ERROR_SOLVER = -7,    /**< An internal solver error occurred during search. */
    GHOST_ERROR_API_USAGE = -8  /**< Incorrect API usage (e.g., getting results before solving). */
} GhostStatus;

/**
 * @brief Solution status codes (subset of GhostStatus for querying results).
 */
typedef enum {
    GHOST_SOLUTION_STATUS_UNKNOWN = 0,      /**< Solver has not been run or status is unknown. */
    GHOST_SOLUTION_STATUS_SAT = 1,          /**< Feasible solution found (satisfaction). */
    GHOST_SOLUTION_STATUS_OPTIMAL = 2,      /**< Optimal solution found (optimization). */
    GHOST_SOLUTION_STATUS_FEASIBLE = 3,     /**< Feasible, possibly non-optimal solution found (optimization). */
    GHOST_SOLUTION_STATUS_INFEASIBLE = -1   /**< Problem proven infeasible or no solution found. */
} GhostSolutionStatus;


// === Session Management ===

/**
 * @brief Creates a new GHOST session.
 *
 * @param permutation_problem Set to true if the problem should be treated as a permutation problem.
 * @return A handle to the new session, or NULL on failure.
 */
GhostSessionHandle ghost_create_session(bool permutation_problem);

/**
 * @brief Destroys a GHOST session and frees associated resources.
 *
 * @param handle The session handle to destroy. Must not be NULL.
 */
void ghost_destroy_session(GhostSessionHandle handle);

/**
 * @brief Retrieves the last error message associated with a session.
 *
 * The returned string is valid until the next API call on the same handle.
 * Returns NULL if no error has occurred or the handle is invalid.
 *
 * @param handle The session handle. Must not be NULL.
 * @return A pointer to the last error message string, or NULL.
 */
const char* ghost_get_last_error(GhostSessionHandle handle);


// === Variable Addition ===

/**
 * @brief Adds a new integer variable with a contiguous domain [min_val, max_val].
 *
 * @param handle The session handle. Must not be NULL.
 * @param min_val The minimum value in the variable's domain (inclusive).
 * @param max_val The maximum value in the variable's domain (inclusive).
 * @param name An optional name for the variable (can be NULL or empty).
 * @return The unique ID of the newly created variable (>= 0), or a negative GhostStatus code on error.
 */
int ghost_add_variable(GhostSessionHandle handle, int min_val, int max_val, const char* name);

/**
 * @brief Adds a new integer variable with a custom, potentially non-contiguous domain.
 *
 * @param handle The session handle. Must not be NULL.
 * @param domain_values An array containing the allowed integer values for the variable. Must not be NULL if domain_size > 0.
 * @param domain_size The number of values in the domain_values array.
 * @param name An optional name for the variable (can be NULL or empty).
 * @return The unique ID of the newly created variable (>= 0), or a negative GhostStatus code on error.
 */
int ghost_add_variable_domain(GhostSessionHandle handle, const int* domain_values, size_t domain_size, const char* name);


// === Constraint Addition ===

/**
 * @brief Adds a linear equality constraint: sum(coeffs[i] * var[var_ids[i]]) == rhs.
 *
 * @param handle The session handle. Must not be NULL.
 * @param var_ids An array of variable IDs involved in the constraint. Must not be NULL if num_vars > 0.
 * @param coeffs An array of coefficients corresponding to each variable in var_ids. If NULL, coefficients default to 1.0. Must have num_vars elements if not NULL.
 * @param num_vars The number of variables (and coefficients) in the constraint.
 * @param rhs The right-hand side value of the equality.
 * @return A unique ID for the constraint (>= 0), or a negative GhostStatus code on error.
 */
int ghost_add_linear_eq_constraint(GhostSessionHandle handle, const int* var_ids, const double* coeffs, size_t num_vars, double rhs);

/**
 * @brief Adds an AllDifferent constraint: all variables in var_ids must take distinct values.
 *
 * @param handle The session handle. Must not be NULL.
 * @param var_ids An array of variable IDs involved in the constraint. Must not be NULL if num_vars > 0.
 * @param num_vars The number of variables in the constraint.
 * @return A unique ID for the constraint (>= 0), or a negative GhostStatus code on error.
 */
int ghost_add_alldifferent_constraint(GhostSessionHandle handle, const int* var_ids, size_t num_vars);

// Add functions for other constraints (LinearLeq, LinearGeq, etc.) here as needed...


// === Objective Setting ===

/**
 * @brief Sets a linear objective function: minimize or maximize sum(coeffs[i] * var[var_ids[i]]).
 *        Calling this function replaces any previously set objective.
 *
 * @param handle The session handle. Must not be NULL.
 * @param maximize If true, maximize the objective; otherwise, minimize it.
 * @param var_ids An array of variable IDs involved in the objective. Must not be NULL if num_vars > 0.
 * @param coeffs An array of coefficients corresponding to each variable in var_ids. Must not be NULL if num_vars > 0.
 * @param num_vars The number of variables (and coefficients) in the objective.
 * @return GHOST_SUCCESS or a negative GhostStatus code on error.
 */
GhostStatus ghost_set_linear_objective(GhostSessionHandle handle, bool maximize, const int* var_ids, const double* coeffs, size_t num_vars);


// === Options Management ===

/**
 * @brief Creates a new options handle with default settings.
 *
 * @return A handle to the new options object, or NULL on failure.
 */
GhostOptionsHandle ghost_create_options();

/**
 * @brief Destroys an options handle and frees associated resources.
 *
 * @param options_handle The options handle to destroy. Can be NULL (no-op).
 */
void ghost_destroy_options(GhostOptionsHandle options_handle);

/**
 * @brief Sets the parallel execution option.
 *
 * @param options_handle The options handle. Must not be NULL.
 * @param parallel Set to true to enable parallel runs.
 * @return GHOST_SUCCESS or a negative GhostStatus code on error.
 */
GhostStatus ghost_set_option_parallel(GhostOptionsHandle options_handle, bool parallel);

/**
 * @brief Sets the number of threads for parallel execution.
 *        Only effective if parallel execution is enabled.
 *        A value <= 0 typically means use hardware concurrency.
 *
 * @param options_handle The options handle. Must not be NULL.
 * @param num_threads The desired number of threads.
 * @return GHOST_SUCCESS or a negative GhostStatus code on error.
 */
GhostStatus ghost_set_option_num_threads(GhostOptionsHandle options_handle, int num_threads);

// Add setters for other relevant options (tabu times, thresholds, etc.) here...
// Example:
// GhostStatus ghost_set_option_tabu_time_local_min(GhostOptionsHandle options_handle, int value);
// GhostStatus ghost_set_option_restart_threshold(GhostOptionsHandle options_handle, int value);


// === Solving ===

/**
 * @brief Solves the problem defined in the session.
 *
 * Uses the fast search (heuristic) method.
 *
 * @param handle The session handle containing the model definition. Must not be NULL.
 * @param options_handle An optional handle to solver options. Can be NULL to use default options.
 * @param timeout_microseconds The maximum time allowed for the solver in microseconds.
 * @return A GhostStatus code indicating the outcome (e.g., GHOST_SAT_FOUND, GHOST_OPTIMAL_FOUND, GHOST_INFEASIBLE, GHOST_ERROR_...).
 */
GhostStatus ghost_solve(GhostSessionHandle handle, GhostOptionsHandle options_handle, double timeout_microseconds);


// === Result Querying ===

/**
 * @brief Gets the status of the last solution found.
 *
 * Call this after ghost_solve returns a success/feasible/infeasible status.
 *
 * @param handle The session handle. Must not be NULL.
 * @return A GhostSolutionStatus code.
 */
GhostSolutionStatus ghost_get_solution_status(GhostSessionHandle handle);

/**
 * @brief Gets the value assigned to a specific variable in the best found solution/candidate.
 *
 * Call this after ghost_solve returns a success/feasible status.
 *
 * @param handle The session handle. Must not be NULL.
 * @param var_id The ID of the variable to query.
 * @param value_ptr Pointer to an integer where the variable's value will be stored. Must not be NULL.
 * @return GHOST_SUCCESS if the value was retrieved, or a negative GhostStatus code on error (e.g., invalid ID, no solution).
 */
GhostStatus ghost_get_variable_value(GhostSessionHandle handle, int var_id, int* value_ptr);

/**
 * @brief Gets the objective value of the best found solution/candidate.
 *
 * Call this after ghost_solve returns a success/feasible status for an optimization problem.
 *
 * @param handle The session handle. Must not be NULL.
 * @param objective_value_ptr Pointer to a double where the objective value will be stored. Must not be NULL.
 * @return GHOST_SUCCESS if the value was retrieved, or a negative GhostStatus code on error (e.g., no solution, not an optimization problem).
 */
GhostStatus ghost_get_objective_value(GhostSessionHandle handle, double* objective_value_ptr);

/**
 * @brief Gets the values assigned to all variables in the best found solution/candidate.
 *
 * Call this after ghost_solve returns a success/feasible status.
 * The provided buffer must be large enough to hold the values for all variables.
 * You can get the number of variables beforehand if needed (consider adding a ghost_get_num_variables function).
 *
 * @param handle The session handle. Must not be NULL.
 * @param values_buffer A pointer to an integer array where variable values will be stored. Must not be NULL.
 * @param buffer_size The size of the values_buffer array. Must be at least the number of variables in the model.
 * @return GHOST_SUCCESS if values were retrieved, or a negative GhostStatus code on error (e.g., no solution, buffer too small).
 */
GhostStatus ghost_get_variable_values(GhostSessionHandle handle, int* values_buffer, size_t buffer_size);


#ifdef __cplusplus
} // extern "C"
#endif

#endif // GHOST_C_API_H
