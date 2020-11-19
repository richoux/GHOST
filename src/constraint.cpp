/*
 * GHOST (General meta-Heuristic Optimization Solving Tool) is a C++ library 
 * designed to help developers to model and implement optimization problem 
 * solving. It contains a meta-heuristic solver aiming to solve any kind of 
 * combinatorial and optimization real-time problems represented by a CSP/COP/CFN. 
 *
 * GHOST has been first developped to help making AI for the RTS game
 * StarCraft: Brood war, but can be used for any kind of applications where 
 * solving combinatorial and optimization problems within some tenth of 
 * milliseconds is needed. It is a generalization of the Wall-in project.
 * Please visit https://github.com/richoux/GHOST for further information.
 * 
 * Copyright (C) 2014-2020 Florian Richoux
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

#include <algorithm>

#include "constraint.hpp"

using namespace ghost;

int Constraint::NBER_CTR = 0;

Constraint::Constraint( const std::vector<Variable>& variables )
	: _is_expert_delta_error_defined( true ),
	  _id( NBER_CTR++ ),
	  _variables( variables )
{ }

double Constraint::simulate( const std::vector<std::pair<int, int>>& changes )
{
	if( _is_expert_delta_error_defined ) [[likely]]
	{
		return expert_delta_error( changes );
	}
	else
	{
		std::vector<Variable> copy_variables( _variables.size() );
		std::copy( _variables.begin(), _variables.end(), copy_variables.begin() );
		
		for( auto& pair : changes )
			copy_variables.at( pair.first ).set_value( pair.second );

		return error( copy_variables ) - _current_error;
	}
}

bool Constraint::has_variable( const Variable& var ) const
{
	return std::find_if( _variables.cbegin(),
	                     _variables.cend(),
	                     [&]( auto& v ){ return v.get_id() == var.get_id(); } ) != _variables.cend();
}  

double Constraint::expert_delta_error( const std::vector<std::pair<int, int>>& changes ) const
{
	_is_expert_delta_error_defined = false;
	throw deltaErrorNotDefinedException();
}
