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
    : Domain( numberVariables, numberVariables, 0 )
  { }
  

  void BuildOrderDomain::addAction()
  {
    size++;
    domains.push_back( initialDomain );

    for( auto &d : domains )
      d.push_back( size );

    initialDomain.push_back( size );
  }


  // friend ostream& operator<<( ostream &os, const BuildOrderDomain &b )
  // {
  // }
}
