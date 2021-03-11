#include "max_value.hpp"

MaxValue::MaxValue( const std::vector<ghost::Variable>& variables,
                    const std::vector<ObjectData>& object_data )
	: Objective( "Max value", variables ),
	  object_data ( object_data )
{ }

double MaxValue::required_cost( const std::vector<ghost::Variable>& variables ) const
{
	double total_value = 0.0;

	for( int i = 0; i < variables.size(); ++i )
		total_value += ( variables[i].get_value() * object_data[i].value() );

	// Notice the minus here.
	// GHOST's solver tries to minimize any objective function.
	// Thus, for maximization problems like this one, outputing '- returned_value' does the trick.
	return -total_value;
}
