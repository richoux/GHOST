#include "constraint_capacity.hpp"

double Capacity::required_error( const std::vector<ghost::Variable*>& variables ) const
{
	double total_size = 0.0;

	for( size_t i = 0 ; i < variables.size() ; ++i )
		total_size += ( variables[i]->get_value() * _object_size[i] );

	if( total_size <= _capacity )
		return 0.0;
	else
		return 1.0;
}
