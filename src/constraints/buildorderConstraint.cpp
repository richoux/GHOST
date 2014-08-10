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

#include "../../include/constraints/buildorderConstraint.hpp"

using namespace std;

namespace ghost
{
  BuildOrderConstraint::BuildOrderConstraint( const vector< Action > *variables, const BuildOrderDomain *domain )
    : Constraint<Action, BuildOrderDomain>(variables, domain) { }


  double BuildOrderConstraint::v_cost( vector<double> &varCost ) const
  {
    double conflicts = 0.;
    bool depConflict;
    
    for( auto it = variables->begin() ; it != variables->end() ; ++it )
    {
      if( it->getType() == special )
	continue;
	
      depConflict = false;
      auto dep = it->getDependencies();
      if( !dep.empty() && !( dep.size() == 1 && dep.at(0).compare("Protoss_Nexus") == 0 ) )
	for( const auto &d : dep )
	  if( none_of( variables->begin(), it, [&](Action &a){return d.compare(a.getFullName()) == 0;} ) )
	  {
	    depConflict = true;
	    
	    for( auto it_dep = it+1 ; it_dep != variables->end() ; ++it_dep )
	      if( d.compare( it_dep->getFullName() ) == 0 )
	      {
	    	varCost.at( it_dep->getId() ) += 2;
	    	conflicts += 2;
	      }
	  }

      if( it->getCostGas() > 0 && none_of( variables->begin(), it, [&](Action &a){return a.getFullName().compare("Protoss_Assimilator") == 0;} ) )
      {
	depConflict = true;
	
	for( auto it_dep = it+1 ; it_dep != variables->end() ; ++it_dep )
	  if( it_dep->getFullName().compare("Protoss_Assimilator") == 0 )
	  {
	    varCost.at( it_dep->getId() ) += 2;
	    conflicts += 2;
	  }
      }

      if( depConflict )
      {
	varCost.at( it->getId() ) += 3;
	conflicts += 3;
      }
    }
    
    return conflicts;
  }

  vector<double> BuildOrderConstraint::v_simulateCost( Action &currentAction,
						       const vector<int> &newPosition,
						       vector< vector<double> > &vecVarSimCosts,
						       shared_ptr< Objective< Action, BuildOrderDomain > > objective )
  {
    vector<double> simCosts( domain->getSize(), -1. );
    int backup = currentAction.getValue();
    
    if( objective )
      objective->resetHelper();

    for( const auto &pos : newPosition )
    {
      if( pos == 0 )
      {
	for( int i = backup ; i > 0 ; --i )
	{
	  std::swap( variables->at(i-1), variables->at(i) );
	  variables->at(i).shiftValue();
	}
	variables->at(0).setValue( 0 );	
      }
      else
      {
	std::swap( variables->at(pos-1), variables->at(pos) );
	variables->at(pos-1).swapValue( variables->at(pos) );
      }

      simCosts[pos] = v_cost( vecVarSimCosts[pos] );
      
      if( objective )
	objective->setHelper( currentAction, variables, domain );
    }

    for( int i = variables->size() - 1 ; i > backup ; --i )
    {
      std::swap( variables->at(i-1), variables->at(i) );
      variables->at(i).shiftValue();
    }
    variables->at(backup).setValue( backup );	
    
    return simCosts;
  }
}
