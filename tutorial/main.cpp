#include <vector>
#include <functional>
#include <memory>
#include <iostream>

#include <ghost/variable.hpp>
#include <ghost/constraint.hpp>
#include <ghost/solver.hpp>

#if defined OBJECTIVE
#include <ghost/objective.hpp>
#endif

#include "object_data.hpp"
#include "constraint_capacity.hpp"

#if defined OBJECTIVE
#include "max_value.hpp"
#else
#include "at_least.hpp"
#endif

int main()
{
/*
 * Defining variables and eventual additional data about variables (here, object_data)
 */
	std::vector< ghost::Variable > variables;
	std::vector< ObjectData > object_data;

	variables.emplace_back( std::string("bottle"), std::string("b"), 0, 51 );
	variables.emplace_back( std::string("sandwich"), std::string("s"), 0, 11 );

	object_data.emplace_back( 1, 500 );
	object_data.emplace_back( 1.25, 650 );

// We cannot have containers of plain '&' references in C++, but since variables are shared 
// among constraints and the objective function, having references on Variable is convenient.
// Thus, Constraint and Objective are waiting for a vector of reference_wrapper<Variable>
	std::vector< std::reference_wrapper< ghost::Variable >> variables_ref( variables.begin(), 
		 variables.end() );

/*
 * Defining constraints
 */
	
// Let's make a knapsack with a capacity of 30
	std::shared_ptr< ghost::Constraint > capacity = std::make_shared< Capacity >( variables_ref, 
		        object_data,
		        30 );

	
#if defined OBJECTIVE
	std::vector< std::shared_ptr< ghost::Constraint >> constraints { capacity };
	
	/*
	 * Defining the objective function
	 */
	
	std::shared_ptr< ghost::Objective > objective = std::make_shared< MaxValue >( object_data );
#else
// We won't accept any object combinations with a total value below 16000
	std::shared_ptr< ghost::Constraint > at_least_value = std::make_shared< AtLeast >( variables_ref,
		             object_data,
		             16000 );
	
	std::vector< std::shared_ptr< ghost::Constraint >> constraints { capacity, at_least_value };
#endif
	
/*
 * Defining the solver and calling it
 */

#if defined OBJECTIVE
	ghost::Solver solver( variables, constraints, objective );
#else
	ghost::Solver solver( variables, constraints );
#endif
	
// cost will store the optimal cost found bu the solver
// solution will store the value of variables giving this score
	double cost = 0.;
	std::vector<int> solution( variables_ref.size(), 0 );

// Run the solver with a 100 milliseconds budget
// After 100 milliseconds, the solver will write in cost and solution the best solution it has found.
	solver.solve( cost, solution, 10000, 100000 );

	std::cout << "Cost: " << cost << "\nSolution:";
	for( auto v : solution )
		std::cout << " " << v;
	std::cout << "\n";
}
