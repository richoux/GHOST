#pragma once

#include <vector>
#include <functional>
#include <algorithm>

#include <ghost/constraint.hpp>
#include <ghost/variable.hpp>

class Capacity : public ghost::Constraint
{
	std::vector<double> _object_size;
	int _capacity;

	double required_error( const std::vector<ghost::Variable*>& variables ) const override;
	
public:
	Capacity( const std::vector<ghost::Variable>& variables,
	          const std::vector<double>&& object_size,
	          int capacity )
		: Constraint( variables ),
		  _object_size( std::move( object_size ) ),
		  _capacity( capacity )
	{	}
};
