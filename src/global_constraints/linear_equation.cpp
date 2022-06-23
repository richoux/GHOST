#include <cmath>
#include <algorithm>
#include <iostream>

#include "global_constraints/linear_equation.hpp"

using ghost::global_constraints::LinearEquation;

LinearEquation::LinearEquation( const std::vector<int>& index, int rhs )
	: Constraint( index ),
	  _rhs( rhs ),
	  _current_diff( 0 )
{ }

double LinearEquation::required_error( const std::vector<Variable*>& variables ) const
{
	int sum = 0;
	for( const auto& var : variables )
		sum += var->get_value();

	_current_diff = sum - _rhs;
	
	return std::abs( _current_diff );
}

double LinearEquation::optional_delta_error( const std::vector<Variable*>& variables,
                                       const std::vector<int>& variable_indexes,
                                       const std::vector<int>& candidate_values ) const
{
	int diff = _current_diff;

	for( int i = 0 ; i < static_cast<int>( variable_indexes.size() ); ++i )
		diff += ( candidate_values[ i ] - variables[ variable_indexes[i] ]->get_value() );
	
	return std::abs( diff ) - std::abs( _current_diff );
} 

void LinearEquation::conditional_update_data_structures( const std::vector<Variable*>& variables, int variable_index, int new_value ) 
{
	_current_diff += ( new_value - variables[ variable_index ]->get_value() );
}
