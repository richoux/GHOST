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

	double required_cost() const override;
public:
	AtLeast( const std::vector< std::reference_wrapper<ghost::Variable> >& variables,
	         const std::vector<ObjectData>& object_data,
	         double k );
};
