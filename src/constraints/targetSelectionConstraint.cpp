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


#include <vector>
#include <iostream>
#include <memory>
#include <algorithm>
#include <string>

#include "../../include/constraints/targetSelectionConstraint.hpp"

using namespace std;

namespace ghost
{
  TargetSelectionConstraint::TargetSelectionConstraint( const vector< Unit > *variables, const TargetSelectionDomain *domain )
    : Constraint<Unit, TargetSelectionDomain>(variables, domain) { }


  double TargetSelectionConstraint::v_cost( vector<double> &varCost ) const
  {
    double conflicts = 0.;

    for( auto it = variables->begin() ; it != variables->end() ; ++it )
    {
      // A unit u is badly affected to a target t iif:
      // 1. u can shoot
      // 2. u has at least one reachable target in its range
      // 3. t is the dummy target (-1) or t in unreachable from u
      if( it->canShoot()
	  && !domain->getEnemiesInRange( *it ).empty()
	  && ( it->getValue() == -1 || !it->isInRange( domain->getEnemyData( it->getValue() ) ) ) )
      {
	++conflicts;
	++varCost[ it->getId() ];
      }
    }
    
    return conflicts;
  }

  vector<double> TargetSelectionConstraint::v_simulateCost( Unit &currentUnit,
							    const vector<int> &newTarget,
							    vector< vector<double> > &vecVarSimCosts,
							    shared_ptr< Objective< Unit, TargetSelectionDomain > > objective )
  {
    vector<double> simCosts( domain->getSize(), -1. );
    int backup = currentUnit.getValue();
    
    if( objective )
      objective->resetHelper();

    for( const auto &target : newTarget )
    {
      domain->clear( currentUnit );
      currentUnit.setValue( target );
      domain->add( currentUnit );
      
      simCosts[target + 1] = v_cost( vecVarSimCosts[target + 1] );
    
      if( objective )
	objective->setHelper( currentUnit, variables, domain );
    }
  
    domain->clear( currentUnit );
    currentUnit.setValue( backup );
    domain->add( currentUnit );
    
    return simCosts;
  }
}
