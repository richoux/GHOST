/*
 * GHOST (General meta-Heuristic Optimization Solving Tool) is a C++ library 
 * designed for StarCraft: Brood war. 
 * GHOST is a meta-heuristic solver aiming to solve any kind of combinatorial 
 * and optimization RTS-related problems represented by a CSP. 
 * It is an extension of the project Wall-in.
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


#include "../../include/constraints/wallinConstraint.hpp"

namespace ghost
{
  WallinConstraint( const vector< Building > &variables, const WallinGrid &domain )
    : Constraint<Building, WallinGrid>(variables, domain) { }
    
  bool WallinConstraint::isWall() const
  {
    auto startingBuildings = domain->buildingsAt( domain->getStartingTile() );
    if( startingBuildings.size() != 1)
      return false;

    auto targetBuildings = domain->buildingsAt( domain->getTargetTile() );
    if( targetBuildings.size() != 1)
      return false;

    // if same building on both the starting and target tile
    if( *startingBuildings.begin() == *targetBuildings.begin() )
      return true;

    int nberTarget = *( targetBuildings.begin() );

    int nberCurrent = *( startingBuildings.begin() );
    shared_ptr<Variable> current = variables[ nberCurrent ];
    set< shared_ptr<Variable> > toVisit = domain->getBuildingsAround( *current, variables );
    set< shared_ptr<Variable> > visited;
    set< shared_ptr<Variable> > neighbors;
    
    visited.insert( current );

    if( toVisit.find( variables[nberTarget] ) != toVisit.end() )
      return true;
    
    while( !toVisit.empty() )
    {
      auto first = *( toVisit.begin() );
      current = first;
      toVisit.erase( first );
      neighbors = domain->getBuildingsAround( *current, variables );
      
      for( auto n : neighbors )
      {
	if( n->getId() == nberTarget )
	  return true;
	if( visited.find( n ) == visited.end() )
	  toVisit.insert( n );
      }

      visited.insert( current );
    }

    return false;
  }
}
