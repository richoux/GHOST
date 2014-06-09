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


#include <set>

#include "../../include/constraint/wallinOverlap.hpp"

using namespace std;

namespace ghost
{
  Overlap::Overlap(const vector< Building > &variables, const WallinGrid &domain) 
    : WallinConstraint(variables, domain) { }

  double Overlap::cost( vector<double> &varCost ) const
  {
    // version 1: 1 failure = 1 cost
    // return double( domain->failures().size() );

    // version 2: 1 conflict = 1 cost (may have several conflicts into one failure)
    double conflicts = 0.;

    for( auto failures : domain.failures() )
    {
      int nbConflict = failures.second.size() - 1;
      if( nbConflict > 0 && failures.second.find( "###" ) == string::npos )
      {
	conflicts += nbConflict;
	set<int> setBuildings = domain.buildingsAt( failures.first );
	for( auto id : setBuildings )
	  varCost[ id ] += nbConflict;
      }
    }

    return conflicts;    
  }

  vector<double> Overlap::simulateCost( Building &oldBuilding, const vector<int> &newPosition, vector< vector<double> > &vecVarSimCosts )
  {
    vector<double> simCosts( domain.getSize(), -1. );
    int backup = oldBuilding.getValue();
    int previousPos;
    int diff;

    for( auto pos : newPosition )
    {
      if( pos >= 1 && pos == previousPos + 1 )
      {
	vecVarSimCosts[pos + 1] = vecVarSimCosts[pos];
	
	diff = domain.shift( oldBuilding ).first;
	if( diff != 0 )
	{
	  set<int> setBuildings = domain.buildingsAt( pos + 1 );
	  for( auto id : setBuildings )
	    vecVarSimCosts[pos + 1][ id ] += diff;
	}

	simCosts[pos + 1] = simCosts[pos] + diff;
      }
      else
      { 
	domain.clear( oldBuilding );
	oldBuilding.setValue( pos );
	domain.add( oldBuilding );
	
	simCosts[pos + 1] = cost( vecVarSimCosts[pos + 1] );
      }

      previousPos = pos;
    }

    domain.clear( oldBuilding );
    oldBuilding.setValue( backup );
    domain.add( oldBuilding );
    
    return simCosts;
  }
}
