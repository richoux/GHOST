#include "knapsack_objective.hpp"

KSObjective::KSObjective( const std::vector<ghost::Variable>& variables, std::shared_ptr<ghost::AuxiliaryData> data )
	: Maximize( variables, "Max profit" ),
	  _values( std::dynamic_pointer_cast<KSCoefficients>(data)->values )
{ }

double KSObjective::required_cost( const std::vector<ghost::Variable*>& variables ) const
{
	double total_value = 0.0;

	for( size_t i = 0 ; i < variables.size() ; ++i )
		total_value += variables[i]->get_value() * _values[i]; 

	return total_value;
}
