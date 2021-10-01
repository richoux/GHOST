#pragma once

#include <vector>
#include <memory>
#include <ghost/constraint.hpp>

class KSCapacity : public ghost::Constraint
{
	int _capacity;
	std::vector<int> _weights;

public:
	KSCapacity( const std::vector<ghost::Variable>& variables, int capacity, const std::vector<int>&& weights );

	double required_error( const std::vector<ghost::Variable*>& variables ) const override;
};
