#pragma once

#include <vector>
#include <memory>
#include <ghost/constraint.hpp>

#include "knapsack_coefficients.hpp"

class KSCapacity : public ghost::Constraint
{
	int _capacity;
	std::vector<int> _weights;

public:
	KSCapacity( const std::vector<ghost::Variable>& variables, int capacity, std::shared_ptr<ghost::AuxiliaryData> data );

	double required_error( const std::vector<ghost::Variable*>& variables ) const override;
};
