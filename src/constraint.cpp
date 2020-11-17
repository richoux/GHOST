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
	: _neighborhood( 1, 1.0, false, 0.0 ),
	  _is_expert_delta_error_defined( true ),
	  id ( NBER_CTR++ ),
	  variables	( variables )
{ }

double Constraint::simulate( const std::vector<std::pair<int, int>>& changes )
{
	if( _is_expert_delta_error_defined ) [[likely]]
	{
		return expert_delta_error( changes );
	}
	else
	{
		std::vector<int> copy_variables( variables.size() );
		int index = 0;

		for( auto&v : variables )
			copy_variables[ index++ ] = v.get_value();
		
		for( auto& pair : changes )
			variables.at( pair.first ).set_value( pair.second );

		auto delta = error() - _current_error;

		index = 0;
		for( auto&v : variables )
			v.set_value( copy_variables[ index++ ] );

		return delta;
	}
}

bool Constraint::has_variable( const Variable& var ) const
{
	return std::find_if( variables.cbegin(),
	                     variables.cend(),
	                     [&]( auto& v ){ return v.get_id() == var.get_id(); } ) != variables.cend();
}  

double Constraint::expert_delta_error( const std::vector<std::pair<int, int>>& changes ) const
{
	_is_expert_delta_error_defined = false;
	throw deltaErrorNotDefinedException();
}
