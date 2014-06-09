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


#include "../../include/constraint/wallinStartingTargetTiles.hpp"

using namespace std;

namespace ghost
{
  StartingTargetTiles::StartingTargetTiles(const vector< Building > &variables, const WallinGrid &domain) 
    : WallinConstraint(variables, domain)
  {
    for( auto b : variables )
      mapBuildings[b.getId()] = b;
  }

  double StartingTargetTiles::cost( vector<double>& varCost ) const
  {
    // no building on one of these two tiles: cost of the tile = 6
    // a building with no or with 2 or more neighbors: cost of the tile = 3
    // two or more buildings on one of these tile: increasing penalties.
    double conflicts = 0.;

    set<int> startingBuildings = domain.buildingsAt( domain.getStartingTile() );
    set<int> targetBuildings = domain.buildingsAt( domain.getTargetTile() );

    Building b;
    int neighbors;

    // if same building on both the starting and target tile
    if( startingBuildings.size() == 1 && targetBuildings.size() == 1 && *startingBuildings.begin() == *targetBuildings.begin() )
      return 0.;

    if( startingBuildings.empty() )
    {
      // penalize buildings not placed on the domain
      for( auto v : variables )
	if( !v.isOnDomain() )
	{
	  varCost[ v.getId() ] += 2;
	  conflicts += 2;
	}
    }
    else
    {
      //int penalty = 0;
      for( int bId : startingBuildings )
      {
	b = mapBuildings.at(bId);
	neighbors = domain.countAround( b, variables );

	if( neighbors != 1 )
	{
	  conflicts += 2;
	  varCost[ bId ] += 2;
	}

	//conflicts += penalty++;
      }
    }

    if( targetBuildings.empty() )
    {      
      // penalize buildings not placed on the domain
      for( auto v : variables )
	if( !v.isOnDomain() )
	{
	  varCost[ v.getId() ] += 2;
	  conflicts += 2;
	}
    }
    else
    {
      //int penalty = 0;
      for( int bId : targetBuildings )
      {
	b = mapBuildings.at(bId);
	neighbors = domain.countAround( b, variables );

	if( neighbors != 1 )
	{
	  conflicts += 2;
	  varCost[ bId ] += 2;
	}

	//conflicts += penalty++;	
      }
      
    }

    return conflicts;    
  }
}
