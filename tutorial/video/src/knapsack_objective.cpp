#include "knapsack_objective.hpp"

KSObjective::KSObjective( const std::vector<ghost::Variable>& variables, const std::vector<int>&& values )
	: Maximize( variables, "Max profit" ),
	  _values( std::move( values ) )
{ }

double KSObjective::required_cost( const std::vector<ghost::Variable*>& variables ) const
{
	double total_value = 0.0;

	for( size_t i = 0 ; i < variables.size() ; ++i )
		total_value += variables[i]->get_value() * _values[i];

	return total_value;
}
