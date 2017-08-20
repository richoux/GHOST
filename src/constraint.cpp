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

#include "constraint.hpp"

using namespace std;
using namespace ghost;

int Constraint::NBER_CTR = 0;

Constraint::Constraint( const vector< shared_ptr< Variable > >& variables )
  : variables	( variables ),
    id		( NBER_CTR++ )
{ }

Constraint::Constraint( const Constraint &other )
  : variables	( other.variables ),
    id		( other.id )
{ }

// Constraint& Constraint::operator=( Constraint other )
// {
//   this->swap( other );
//   return *this;
// }

// void Constraint::swap( Constraint &other )
// {
//   swap( this->variables, other.variables );
//   swap( this->id, other.id );
// }

bool Constraint::has_variable( const shared_ptr< Variable > var ) const
{
  return get_variable_iterator( var ) != variables.cend();
}

vector< shared_ptr< Variable > >::const_iterator Constraint::get_variable_iterator( const shared_ptr< Variable > var ) const
{
  return find_if( variables.cbegin(),
		  variables.cend(),
		  [&]( auto& v ){ return v->get_id() == var->get_id(); } );
}
