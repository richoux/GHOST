/*
 * GHOST (General meta-Heuristic Optimization Solving Tool) is a C++ library 
 * designed for StarCraft: Brood war. 
 * GHOST is a meta-heuristic solver aiming to solve any kind of combinatorial 
 * and optimization RTS-related problems represented by a CSP/COP. 
 * It is a generalization of the project Wall-in.
 * Please visit https://github.com/richoux/GHOST for further information.
 * 
 * Copyright (C) 2014 Florian Richoux
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


#include <numeric>
#include <iostream>
#include <typeinfo>

#include "../../include/domains/buildorderDomain.hpp"

using namespace std;

namespace ghost
{
  BuildOrderDomain::BuildOrderDomain( int numberVariables, const vector<Action> *variables )
    : Domain( numberVariables, numberVariables, 0 ), order(*variables)
  { }
  
  void BuildOrderDomain::add( const Action &action )
  {
    auto it = order.insert( begin(order) + action.getValue(), action );
    for( auto iter = it + 1 ; iter != end(order) ; ++iter )
      iter->shiftValue();
  }
  
  void BuildOrderDomain::clear( const Action &action )
  {
    auto it = order.erase( begin(order) + action.getValue() );
    for( auto iter = it ; iter != end(order) ; ++iter )
      iter->setValue( iter->getValue() - 1 );
  }

  void BuildOrderDomain::moveTo( int from, int to )
  {
    if( from == to )
      return;
    
    Action action = order.at( from );
    action.setValue( to );
    order.erase( begin(order) + from );
    order.insert( begin(order) + to, action );

    if( from > to )
      for( auto iter = begin(order) + to ; iter != begin(order) + from ; ++iter )
	iter->shiftValue();

    else
      for( auto iter = begin(order) + from ; iter != begin(order) + to ; ++iter )
	iter->setValue( iter->getValue() - 1 );
  }

  void BuildOrderDomain::addAction( Action &action, bool initialized )
  {
    size++;
    domains.push_back( initialDomain );

    for( auto &d : domains )
      d.push_back( size );

    initialDomain.push_back( size );

    if( initialized )
    {
      add( action );
    }
    else
    {
      action.setValue( size );
      order.push_back( action );
    }
  }


  // friend ostream& operator<<( ostream &os, const BuildOrderDomain &b )
  // {
  // }
}
