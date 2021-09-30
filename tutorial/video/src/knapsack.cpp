#include <iostream>
#include <ghost/solver.hpp>
#include <vector>
#include <memory>
#include <string>
#include <chrono>

#include "knapsack_model_builder.hpp"

using namespace std::literals::chrono_literals;

int main( int argc, char** argv )
{
	KSBuilder builder;

	ghost::Solver solver( builder );

	double cost;
	std::vector<int> solution;

	ghost::Options options;
	options.parallel_runs = true;

	solver.solve( cost, solution, 100ms, options );
}
