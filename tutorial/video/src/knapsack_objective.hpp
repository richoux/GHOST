#pragma once

#include <vector>
#include <memory>
#include <ghost/objective.hpp>

#include "knapsack_coefficients.hpp"

class KSObjective : public ghost::Maximize
{
	std::vector<int> _values;

public:
	KSObjective( const std::vector<ghost::Variable>& variables, std::shared_ptr<ghost::AuxiliaryData> data );

	double required_cost( const std::vector<ghost::Variable*>& variables ) const override;
};
