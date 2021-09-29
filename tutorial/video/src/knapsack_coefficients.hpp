#pragma once

#include <vector>
#include <ghost/auxiliary_data.hpp>

class KSCoefficients : public ghost::AuxiliaryData
{
public:
	std::vector<int> weights;
	std::vector<int> values;
	
	KSCoefficients();

	void required_update( const std::vector<ghost::Variable*>& variables, int index, int new_value ) override;
};
