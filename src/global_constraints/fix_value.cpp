/*
 * GHOST (General meta-Heuristic Optimization Solving Tool) is a C++ framework
 * designed to help developers to model and implement optimization problem
 * solving. It contains a meta-heuristic solver aiming to solve any kind of
 * combinatorial and optimization real-time problems represented by a CSP/COP/EFSP/EFOP. 
 *
 * First developped to solve game-related optimization problems, GHOST can be used for
 * any kind of applications where solving combinatorial and optimization problems. In
 * particular, it had been designed to be able to solve not-too-complex problem instances
 * within some milliseconds, making it very suitable for highly reactive or embedded systems.
 * Please visit https://github.com/richoux/GHOST for further information.
 *
 * Copyright (C) 2014-2022 Florian Richoux
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

#include "global_constraints/fix_value.hpp"

using ghost::global_constraints::FixValue;

FixValue::FixValue( const std::vector<int>& variables_index, int value )
	: Constraint( variables_index ),
	  _value( value )
{ }

double FixValue::required_error( const std::vector<Variable*>& variables ) const
{
	double error = 0.;
	for( auto& var : variables )
		error += std::abs( var->get_value() - _value );
	return error;
}

double FixValue::optional_delta_error( const std::vector<Variable*>& variables,
                                       const std::vector<int>& variable_indexes,
                                       const std::vector<int>& candidate_values ) const
{
	double diff = 0.;
	for( int index = 0 ; index < static_cast<int>( variable_indexes.size() ) ; ++index )
		diff += std::abs( candidate_values[ index ] - _value )
			- std::abs( variables[ variable_indexes[ index ] ]->get_value() - _value );
	
	return diff;
} 
