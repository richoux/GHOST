#pragma once

#include <vector>
#include <functional>
#include <algorithm>

#include <ghost/constraint.hpp>
#include <ghost/variable.hpp>

class AtLeast : public ghost::Constraint
{
	std::vector<double> _object_value;
	double _target_value;

	double required_error( const std::vector<ghost::Variable*>& variables ) const override;

public:
	AtLeast( const std::vector<ghost::Variable>& variables,
	         const std::vector<double>&& object_value,
	         double target_value )
		: Constraint( variables ),
		  _object_value( std::move( object_value ) ),
		  _target_value( target_value )
	{	}
};
