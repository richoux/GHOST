#if defined CFN
#include <algorithm>
#endif

#include "constraint_capacity.hpp"

Capacity::Capacity( const std::vector<ghost::Variable>& variables,
                    const std::vector<ObjectData>& object_data,
                    int capacity )
	: Constraint( variables ),
	  object_data ( object_data ),
	  capacity ( capacity )
{ }

double Capacity::required_error( const std::vector<ghost::Variable>& variables ) const
{
	double total_objects_size = 0.0;

	for( int i = 0; i < variables.size(); ++i )
		total_objects_size += ( variables[i].get_value() * object_data[i].size() );

#if defined CFN
	return std::max( 0., total_objects_size - capacity );
#else
	if( total_objects_size > capacity )
		return 1;
	else
		return 0;
#endif
}
