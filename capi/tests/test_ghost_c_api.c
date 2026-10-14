#include "ghost_c_api.h"
#include <stdio.h>
#include <stdlib.h>

// Simple test program demonstrating the GHOST C API
// This example creates a simple CSP with 3 variables and an AllDifferent constraint

int main() {
    // Create a session
    GhostSessionHandle session = ghost_create_session(false);
    if (!session) {
        fprintf(stderr, "Failed to create GHOST session\n");
        return 1;
    }

    // Add variables with domain [1, 3]
    int var1 = ghost_add_variable(session, 1, 3, "x");
    int var2 = ghost_add_variable(session, 1, 3, "y");
    int var3 = ghost_add_variable(session, 1, 3, "z");

    if (var1 < 0 || var2 < 0 || var3 < 0) {
        fprintf(stderr, "Failed to add variables: %s\n", ghost_get_last_error(session));
        ghost_destroy_session(session);
        return 1;
    }

    printf("Added variables with IDs: %d, %d, %d\n", var1, var2, var3);

    // Add an AllDifferent constraint
    int var_ids[] = {var1, var2, var3};
    int constraint_id = ghost_add_alldifferent_constraint(session, var_ids, 3);
    if (constraint_id < 0) {
        fprintf(stderr, "Failed to add constraint: %s\n", ghost_get_last_error(session));
        ghost_destroy_session(session);
        return 1;
    }

    printf("Added AllDifferent constraint with ID: %d\n", constraint_id);

    // Create options and set parallel to true with 2 threads
    GhostOptionsHandle options = ghost_create_options();
    if (!options) {
        fprintf(stderr, "Failed to create options\n");
        ghost_destroy_session(session);
        return 1;
    }

    ghost_set_option_parallel(options, true);
    ghost_set_option_num_threads(options, 2);

    // Solve the problem with a 1-second timeout
    GhostStatus status = ghost_solve(session, options, 1000000); // 1 second in microseconds

    if (status < 0) {
        fprintf(stderr, "Solving failed: %s\n", ghost_get_last_error(session));
        ghost_destroy_options(options);
        ghost_destroy_session(session);
        return 1;
    }

    // Check if a solution was found
    if (status == GHOST_SAT_FOUND) {
        printf("Solution found!\n");

        // Get variable values
        int values[3];
        if (ghost_get_variable_values(session, values, 3) == GHOST_SUCCESS) {
            printf("x = %d, y = %d, z = %d\n", values[0], values[1], values[2]);
        } else {
            fprintf(stderr, "Failed to get variable values: %s\n", ghost_get_last_error(session));
        }
    } else {
        printf("No solution found (status: %d)\n", status);
    }

    // Clean up
    ghost_destroy_options(options);
    ghost_destroy_session(session);

    return 0;
}
