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

#include "variable.hpp"

using namespace ghost;

int Variable::NBER_VAR = 0;

// Variable::Variable()
// 	: _id(-1),
// 	  _name(""),
// 	  _current_value(-1)
// { }

Variable::Variable( const std::string& name, const std::vector<int>& domain, int index )
	: _id( NBER_VAR++ ),
	  _name( name ),
	  _domain( domain ),
	  _index( domain.cbegin() + index ),
	  _current_value( domain.at( index ) )
{ }

Variable::Variable( const std::string& name, int startValue, std::size_t size, int index )
	: _id( NBER_VAR++ ),
	  _name( name ),
	  _domain( std::vector<int>( size ) )
{
	std::iota( _domain.begin(), _domain.end(), startValue );
	_current_value = _domain.at( index );
	_index = _domain.cbegin() + index;
}
