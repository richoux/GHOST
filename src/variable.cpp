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

#include <algorithm>
#include <typeinfo>

#include "variable.hpp"

using namespace std;
using namespace ghost;

int Variable::NBER_VAR = 0;

Variable::Variable( const string& name, const string& shortName, const Domain& domain, int index )
  : _id		( NBER_VAR++ ),
    name	( name ),
    shortName	( shortName ),
    domain	( domain ),
    index	( index )
{ }

Variable::Variable( const string& name, const string& shortName, int index, vector<int> domain )
  : Variable( name, shortName, { domain }, index )
{ }

Variable::Variable( const string& name, const string& shortName, int index, int size, int startValue )
  : Variable( name, shortName, { size, startValue }, index )
{ }

Variable::Variable( const Variable &other )
  : _id		( other._id ),
    name	( other.name ),
    shortName	( other.shortName ),
    domain	( other.domain ),
    index	( other.index )
{ }

Variable& Variable::operator=( Variable other )
{
  this->swap( other );
  return *this;
}

void Variable::swap( Variable &other )
{
  std::swap( this->_id, other._id );
  std::swap( this->name, other.name );
  std::swap( this->shortName, other.shortName );
  std::swap( this->domain, other.domain );
  std::swap( this->index, other.index );
}  

void Variable::do_random_initialization()
{
  set_value( domain.random_value() );
}

vector<int> Variable::possible_values() const
{
  vector<int> possibleValues;
  
  for( int i = 0 ; i < (int)domain.get_size() ; ++i )
    possibleValues.push_back( domain.get_value( i ) );
  
  return possibleValues;
}    
