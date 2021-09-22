#include "model_builder_sat.hpp"
#include "constraint_capacity.hpp"
#include "constraint_at_least.hpp"

TutorialBuilder::TutorialBuilder()
	: ModelBuilder()
{ }

void TutorialBuilder::declare_variables()
{
	variables.emplace_back( 0, 51, std::string("bottle") );
	variables.emplace_back( 0, 11, std::string("sandwich") );	
}

void TutorialBuilder::declare_constraints()
{
	constraints.push_back( std::make_shared<Capacity>( variables, std::vector<double>{1, 1.25}, 30 ) );
	constraints.push_back( std::make_shared<AtLeast>( variables, std::vector<double>{500, 650}, 15000 ) );
}
