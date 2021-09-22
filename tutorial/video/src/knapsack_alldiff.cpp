#include "knapsack_alldiff.hpp"

KSAllDiff::KSAllDiff( const std::vector<ghost::Variable>& variables )
	: Constraint( variables )
{ }

double KSAllDiff::required_error( const std::vector<ghost::Variable*>& variables ) const
{
	// EFOP version

	double count = 0.0;

	for( size_t i = 0 ; i < variables.size() - 1 ; ++i )
		for( size_t j = i + 1 ; j < variables.size() ; ++j )
			if( variables[i]->get_value() == variables[j]->get_value() )
				++count;

	return count;
}
