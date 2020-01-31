#pragma once

#include <vector>
#include <functional>

#include <ghost/variable.hpp>
#include <ghost/objective.hpp>

#include "object_data.hpp"

class MaxValue : public ghost::Objective
{
	std::vector<ObjectData> object_data;

	double required_cost( const std::vector< ghost::Variable >& ) const override;
public:
	MaxValue( const std::vector<ObjectData>& object_data );
};
