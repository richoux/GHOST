/*
 * GHOST (General meta-Heuristic Optimization Solving Tool) is a C++ library 
 * designed to help developers to model and implement optimization problem 
 * solving. It contains a meta-heuristic solver aiming to solve any kind of 
 * combinatorial and optimization real-time problems represented by a CSP/COP. 
 *
 * GHOST has been first developped to help making AI for the RTS game
 * StarCraft: Brood war, but can be used for any kind of applications where 
 * solving combinatorial and optimization problems within some tenth of 
 * milliseconds is needed. It is a generalization of the Wall-in project.
 * Please visit https://github.com/richoux/GHOST for further information.
 * 
 * Copyright (C) 2014-2017 Florian Richoux
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


#include <typeinfo>
#include <algorithm>
#include <numeric>
#include <exception>

#include "domain.hpp"

using namespace ghost;

Domain::Domain( int outsideScope )
  : _outsideScope	( outsideScope )
{ }

Domain::Domain( const std::vector< int >& domain, int outsideScope )
  : _currentDomain	( domain ),
    _outsideScope	( outsideScope )
{
  if( std::find( std::begin( domain ), std::end( domain ), _outsideScope ) != std::end( domain ) )
    throw 0;
}

Domain::Domain( int size, int startValue )
  : _currentDomain	( std::vector<int>( size ) ),
    _outsideScope	( startValue - 1 )
{
  std::iota( std::begin( _currentDomain ), std::end( _currentDomain ), startValue );
}

int Domain::get_value( int index ) const
{
  if( index >=0 && index < (int)_currentDomain.size() )
    return _currentDomain[ index ];
  else
    return _outsideScope;
}

int Domain::index_of( int value ) const
{
  auto it = std::find( std::begin( _currentDomain ), std::end( _currentDomain ), value );
  if( it == std::end( _currentDomain ) )
    return -1;
  else
    return it - std::begin( _currentDomain );
}
