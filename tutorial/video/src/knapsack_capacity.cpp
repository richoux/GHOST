#include "knapsack_capacity.hpp"

KSCapacity::KSCapacity( const std::vector<ghost::Variable>& variables, int capacity, std::shared_ptr<ghost::AuxiliaryData> data )
	: Constraint( variables ),
	  _capacity( capacity ),
	  _weights( std::dynamic_pointer_cast<KSCoefficients>(data)->weights )
{ }

double KSCapacity::required_error( const std::vector<ghost::Variable*>& variables ) const
{
	// Stuff in common

	double total_weight = 0.0;
	for( size_t i = 0 ; i < variables.size() ; ++i )
		total_weight += variables[i]->get_value() * _weights[i];

	/*
	// COP version here

	return total_weight <= _capacity ? 0.0 : 1.0;

	/*/
	// EFOP version here

	return std::max( 0., total_weight - _capacity );
	//*/
}
