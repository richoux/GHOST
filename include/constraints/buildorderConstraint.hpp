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


#pragma once

#include <vector>
#include <iostream>
#include <memory>
#include <algorithm>
#include <string>

#include "constraint.hpp"
#include "../variables/action.hpp"
#include "../domains/buildorderDomain.hpp"
#include "../objectives/objective.hpp"
#include "../misc/actionFactory.hpp"

using namespace std;

namespace ghost
{
  class BuildOrderConstraint : public Constraint<Action, BuildOrderDomain>
  {
  public:
    BuildOrderConstraint( const vector< Action >*, const BuildOrderDomain* );


  private:
    double		v_cost(		vector<double> &varCost ) const;

    vector<double>	v_simulateCost( Action &currentAction,
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
	domain->clear( currentAction );
	currentAction.setValue( pos );
	domain->add( currentAction );
      
	simCosts[pos+1] = v_cost( vecVarSimCosts[pos+1] );

	if( objective )
	  objective->setHelper( currentAction, variables, domain );
      }
      
      domain->clear( currentAction );
      currentAction.setValue( backup );
      domain->add( currentAction );

      return simCosts;
    }
  };  
}
