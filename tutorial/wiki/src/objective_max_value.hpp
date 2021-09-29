#pragma once

#include <vector>
#include <functional>

#include <ghost/variable.hpp>
#include <ghost/objective.hpp>

class MaxValue : public ghost::Maximize
{
	std::vector<double> _object_value;

	double required_cost( const std::vector<ghost::Variable*>& ) const override;

public:
	MaxValue( const std::vector<ghost::Variable>&, const std::vector<double>&& object_value );
};
