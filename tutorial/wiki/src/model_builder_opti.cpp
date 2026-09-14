#include "model_builder_opti.hpp"
#include "constraint_capacity.hpp"
#include "objective_max_value.hpp"

TutorialBuilder::TutorialBuilder()
	: ModelBuilder()
{ }

void TutorialBuilder::declare_variables()
{
	create_variable( 0, 51, std::string("bottle") );
	create_variable( 0, 11, std::string("sandwich") );
}

void TutorialBuilder::declare_constraints()
{
	constraints.emplace_back( std::make_shared<Capacity>( variables, std::vector<double>{1, 1.25}, 30 ) );
}

void TutorialBuilder::declare_objective()
{
	objective = std::make_shared<MaxValue>( variables, std::vector<double>{500, 650} );
}
