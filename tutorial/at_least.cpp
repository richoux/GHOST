#if defined CFN
#include <algorithm>
#endif

#include "at_least.hpp"

AtLeast::AtLeast( const std::vector<ghost::Variable>& variables,
                  const std::vector<ObjectData>& object_data,
                  double k )
	: Constraint( variables ),
	  object_data ( object_data ),
	  k ( k )
{ }

double AtLeast::required_error( const std::vector<ghost::Variable>& variables ) const
{
	double total_value = 0.0;

	for( int i = 0; i < variables.size(); ++i )
		total_value += ( variables[i].get_value() * object_data[i].value() );

#if defined CFN
	return std::max( 0., k - total_value );
#else
	if( total_value >= k )
		return 0;
	else
		return 1;
#endif
}
