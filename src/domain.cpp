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

using namespace std;
using namespace ghost;

Domain::Domain( int outsideScope )
  : _outsideScope	( outsideScope )
{ }

Domain::Domain( const vector< int >& domain, int outsideScope )
  : _currentDomain	( domain ),
    _outsideScope	( outsideScope )
{
  if( find( begin( domain ), end( domain ), _outsideScope ) != end( domain ) )
    throw 0;
}

Domain::Domain( int size, int startValue )
  : _currentDomain	( vector<int>( size ) ),
    _outsideScope	( startValue - 1 )
{
  iota( begin( _currentDomain ), end( _currentDomain ), startValue );
}

Domain::Domain( const Domain &other )
  : _currentDomain( other._currentDomain ),
    _outsideScope( other._outsideScope ),
    _random( other._random )
{ }

Domain& Domain::operator=( Domain other )
{
  this->swap( other );
  return *this;
}

void Domain::swap( Domain &other )
{
  std::swap( this->_currentDomain, other._currentDomain );
  std::swap( this->_outsideScope, other._outsideScope );
  std::swap( this->_random, other._random );
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
  auto it = find( begin( _currentDomain ), end( _currentDomain ), value );
  if( it == end( _currentDomain ) )
    return -1;
  else
    return it - begin( _currentDomain );
}
