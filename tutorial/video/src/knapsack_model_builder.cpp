#include "knapsack_model_builder.hpp"
#include "knapsack_capacity.hpp"
#include "knapsack_alldiff.hpp"
#include "knapsack_objective.hpp"

KSBuilder::KSBuilder()
	: ModelBuilder()
{ }

void KSBuilder::declare_variables()
{
	create_n_variables( 5, 0, 16 );
}

void KSBuilder::declare_constraints()
{
	constraints.emplace_back( std::make_shared<KSCapacity>( variables, 15, std::vector<int>{12,2,1,1,4} ) );
	constraints.emplace_back( std::make_shared<KSAllDiff>( variables ) );
}

void KSBuilder::declare_objective()
{
	objective = std::make_shared<KSObjective>( variables, std::vector<int>{4,2,2,1,10} );
}
