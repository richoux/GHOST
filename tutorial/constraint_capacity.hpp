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

	double required_cost() const override;
public:
	Capacity( const std::vector< std::reference_wrapper<ghost::Variable> >& variables,
	          const std::vector<ObjectData>& object_data,
	          int capacity );
};

