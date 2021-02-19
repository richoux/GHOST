#pragma once

#include <vector>
#include <functional>

#include <ghost/constraint.hpp>
#include <ghost/variable.hpp>

#include "object_data.hpp"

class AtLeast : public ghost::Constraint
{
	std::vector<ObjectData> object_data;
	double k;

	double required_error( const std::vector<ghost::Variable>& variables ) const override;
public:
	AtLeast( const std::vector<ghost::Variable>& variables,
	         const std::vector<ObjectData>& object_data,
	         double k );
};
