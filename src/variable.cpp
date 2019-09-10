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
 * Copyright (C) 2014-2019 Florian Richoux
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
#include <typeinfo>

#include "variable.hpp"

using namespace std;
using namespace ghost;

int Variable::NBER_VAR = 0;

Variable::Variable()
  : _id		( -1 ),
    _name	( "" ),
    _shortName	( "" ),
    _domain	( Domain( 0, 0 ) ),
    _index	( -1 ),
    _cache_value( -1 )
{ }

Variable::Variable( const string& name, const string& shortName, const Domain& domain, int index )
  : _id		( NBER_VAR++ ),
    _name	( name ),
    _shortName	( shortName ),
    _domain	( domain ),
    _index	( index ),
    _cache_value( domain.get_value( index ) )
{ }

Variable::Variable( const string& name, const string& shortName, const vector<int>& domain, int index )
  : Variable( name, shortName, Domain{ domain }, index )
{ }

Variable::Variable( const string& name, const string& shortName, int startValue, size_t size, int index )
  : Variable( name, shortName, Domain{ startValue, size }, index )
{ }

Variable::Variable( const Variable &other )
  : _id		( other._id ),
    _name	( other._name ),
    _shortName	( other._shortName ),
    _domain	( other._domain ),
    _index	( other._index ),
    _cache_value( other._cache_value )
{ }

Variable& Variable::operator=( Variable other )
{
  this->swap( other );
  return *this;
}

void Variable::swap( Variable &other )
{
  std::swap( this->_id,		other._id );
  std::swap( this->_name,	other._name );
  std::swap( this->_shortName,	other._shortName );
  std::swap( this->_domain,	other._domain );
  std::swap( this->_index,	other._index );
  std::swap( this->_cache_value,other._cache_value );
}  
