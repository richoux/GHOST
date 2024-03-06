/*
 * GHOST (General meta-Heuristic Optimization Solving Tool) is a C++ framework
 * designed to help developers to model and implement optimization problem
 * solving. It contains a meta-heuristic solver aiming to solve any kind of
 * combinatorial and optimization real-time problems represented by a CSP/COP/EF-CSP/EF-COP. 
 *
 * First developed to solve game-related optimization problems, GHOST can be used for
 * any kind of applications where solving combinatorial and optimization problems. In
 * particular, it had been designed to be able to solve not-too-complex problem instances
 * within some milliseconds, making it very suitable for highly reactive or embedded systems.
 * Please visit https://github.com/richoux/GHOST for further information.
 *
 * Copyright (C) 2014-2024 Florian Richoux
 *
 * This file is part of GHOST.
 * GHOST is free software: you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as published
 * by the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * GHOST is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with GHOST. If not, see http://www.gnu.org/licenses/.
 */

#include <cmath>
#include <algorithm>
#include <numeric>
#include <iostream>

#include "global_constraints/all_equal.hpp"

using ghost::global_constraints::AllEqual;

AllEqual::AllEqual( const std::vector<int>& variables_index )
	: Constraint( variables_index )
{ }

AllEqual::AllEqual( const std::vector<Variable>& variables )
	: Constraint( variables )
{ }

double AllEqual::required_error( const std::vector<Variable*>& variables ) const
{
	_count.clear();
	
	for( auto v : variables )
		if( _count.find( v->get_value() ) == _count.end() )
			_count[ v->get_value() ] = 1;
		else
			_count[ v->get_value() ] = _count[ v->get_value() ] + 1;

	auto max = std::max_element( _count.begin(), _count.end(), [](const auto& c1, const auto& c2){ return c1.second < c2.second; } );

	return static_cast<double>( variables.size() ) - max->second;
}

double AllEqual::optional_delta_error( const std::vector<Variable*>& variables, const std::vector<int>& variable_indexes, const std::vector<int>& candidate_values ) const
{
	// Warning: we may have several values with the same maximal count. How to handle that in a smart way?

	auto copy_count( _count );
	auto max = std::max_element( _count.begin(), _count.end(), [](const auto& c1, const auto& c2){ return c1.second < c2.second; } );

	//int number_substraction = std::count( variable_indexes.begin(), variable_indexes.end(), [&](const auto& i){ return variables[ i ]->get_value() == max->first; } );
	//int number_addition = std::count( candidate_values.begin(), candidate_values.end(), [&](const int v){ return v == max->first; } );
	
	for( int i = 0 ; i < static_cast<int>( variable_indexes.size() ) ; ++i )
	{
		copy_count[ variables[ variable_indexes[ i ] ]->get_value() ] = copy_count[ variables[ variable_indexes[ i ] ]->get_value() ] - 1;
		if( copy_count.count( candidate_values[ i ] ) == 0 )
			copy_count[ candidate_values[ i ] ] = 1;
		else
			copy_count[ candidate_values[ i ] ] = copy_count[ candidate_values[ i ] ] + 1;
	}

	auto max_bis = std::max_element( copy_count.begin(), copy_count.end(), [](const auto& c1, const auto& c2){ return c1.second < c2.second; } );
	
	return static_cast<double>( max->second - max_bis->second );
}

void AllEqual::conditional_update_data_structures( const std::vector<Variable*>& variables, int variable_index, int new_value )
{
	_count[ variables[ variable_index ]->get_value() ] = _count[ variables[ variable_index ]->get_value() ] - 1;

	if( _count.count( new_value ) == 0 )
		_count[ new_value ] = 1;
	else
		_count[ new_value ] = _count[ new_value ] + 1;	
}
