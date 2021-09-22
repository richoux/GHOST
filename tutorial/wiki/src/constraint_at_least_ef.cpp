#include "constraint_at_least.hpp"

double AtLeast::required_error( const std::vector<ghost::Variable*>& variables ) const
{
	double total_value = 0.0;

	for( size_t i = 0 ; i < variables.size() ; ++i )
		total_value += ( variables[i]->get_value() * _object_value[i] );

	return std::max( 0., _target_value - total_value );
}
