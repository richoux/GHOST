#include <vector>
#include <functional>
#include <memory>
#include <iostream>
#include <chrono>

#include <ghost/solver.hpp>

#if defined OPTIMIZATION
#include "model_builder_opti.hpp"
#else
#include "model_builder_sat.hpp"
#endif

using namespace std::literals::chrono_literals;

int main()
{
  // Declaring the model builder
	TutorialBuilder builder;
	
	// Defining the solver and calling it
	ghost::Solver solver( builder );

	double cost;
	std::vector<int> solution;

	// Run the solver with a 500 microseconds budget
	solver.solve( cost, solution, 500us );
	
	// After 300 microseconds, the solver will write in cost and solution the best solution it has found.
	std::cout << "Cost: " << cost << "\nSolution:";
	for( auto v : solution )
		std::cout << " " << v;
	std::cout << "\n";
}
