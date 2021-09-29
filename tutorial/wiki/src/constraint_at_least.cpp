#include "constraint_at_least.hpp"

double AtLeast::required_error( const std::vector<ghost::Variable*>& variables ) const
{
	double total_value = 0.0;

	for( size_t i = 0 ; i < variables.size() ; ++i )
		total_value += ( variables[i]->get_value() * _object_value[i] );

	if( total_value >= _target_value )
		return 0.0;
	else
		return 1.0;
}
