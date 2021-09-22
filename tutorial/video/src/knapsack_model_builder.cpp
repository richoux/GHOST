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
	constraints.push_back( std::make_shared<KSCapacity>( variables, 15, auxiliary_data ) );
	constraints.push_back( std::make_shared<KSAllDiff>( variables ) );
}

void KSBuilder::declare_objective()
{
	objective = std::make_shared<KSObjective>( variables, auxiliary_data );
}

void KSBuilder::declare_auxiliary_data()
{
	auxiliary_data = std::make_shared<KSCoefficients>();
}
