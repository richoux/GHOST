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


#include "../../include/constraint/wallinNoGaps.hpp"

using namespace std;

namespace ghost
{
  NoGaps::NoGaps(const vector< Building > &variables, const WallinGrid &domain) 
    : WallinConstraint(variables, domain) { }

  double NoGaps::cost( vector<double> &varCost ) const
  {
    // cost = |buildings with one neighbor| - 1 + |buildings with no neighbors|
    double conflicts = 0.;

    if( !isWall() )
    {
      int nberNeighbors;
      vector<int> oneNeighborBuildings;

      for( auto building : variables )
      {
	if( building.isOnDomain() )
	{
	  // if we don't have a wall, penalise all buildings on the domain->
	  ++conflicts;
	  ++varCost[ building.getId() ];
	  
	  nberNeighbors = domain.countAround( *building, variables );

	  if( nberNeighbors == 0 || nberNeighbors > 2 ) // to change with Protoss and pylons
	  {
	    ++conflicts;
	    ++varCost[ building.getId() ];
	  }
	  else
	  {
	    if( nberNeighbors == 1 )
	      oneNeighborBuildings.push_back( building.getId() );
	  }
	}
      }

      if( oneNeighborBuildings.size() > 2 ) // for latter: pylons can be alone, or have 1 neighbor only
      {
	for( auto b : oneNeighborBuildings )
	  if( ! domain->isStartingOrTargetTile( b ) )
	  {
	    ++conflicts;
	    ++varCost[ b ];
	  }
      }
    }
    
    return conflicts;    
  }
}
