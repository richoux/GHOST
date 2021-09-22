#include "objective_max_value.hpp"

MaxValue::MaxValue( const std::vector<ghost::Variable>& variables,
                    const std::vector<double>&& object_value )
	: Maximize( variables, "Max value" ),
	  _object_value( std::move( object_value ) )
{ }

double MaxValue::required_cost( const std::vector<ghost::Variable*>& variables ) const
{
	double total_value = 0.0;

	for( size_t i = 0 ; i < variables.size() ; ++i )
		total_value += ( variables[i]->get_value() * _object_value[i] );

	return total_value;
}
