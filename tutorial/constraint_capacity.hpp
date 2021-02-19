#pragma once

#include <vector>
#include <functional>

#include <ghost/constraint.hpp>
#include <ghost/variable.hpp>

#include "object_data.hpp"

class Capacity : public ghost::Constraint
{
	// Suppose we have a class ObjectData storing
	// the size and the value of each object type.
	std::vector<ObjectData> object_data;
	int capacity;

	double required_error( const std::vector<ghost::Variable>& variables ) const override;
public:
	Capacity( const std::vector<ghost::Variable>& variables,
	          const std::vector<ObjectData>& object_data,
	          int capacity );
};

