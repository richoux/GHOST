#include "knapsack_coefficients.hpp"

KSCoefficients::KSCoefficients()
	: AuxiliaryData(),
	  weights( std::vector<int>{12,2,1,1,4} ),
	  values( std::vector<int>{4,2,2,1,10} )
{ }

void KSCoefficients::update( const std::vector<ghost::Variable*>& variables, int index, int new_value ) { }
