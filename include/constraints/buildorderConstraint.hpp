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
#include <map>

#include "constraint.hpp"
#include "../variables/action.hpp"
#include "../domains/buildorderDomain.hpp"
#include "../objectives/objective.hpp"

using namespace std;

namespace ghost
{
  class BuildOrderConstraint : public Constraint<Action, BuildOrderDomain>
  {
  public:
    BuildOrderConstraint( const vector< Building >*, const BuildOrderDomain* );

    double cost( vector<double> &varCost ) const { return v_cost( varCost ); }

    vector<double> simulateCost( Action &currentAction,
				 const vector<int> &newValue,
				 vector< vector<double> > &vecVarSimCosts )
    { return v_simulateCost( currentAction, newValue, vecVarSimCosts ); }

  protected:
    virtual double v_cost( vector<double>& ) const = 0;

    virtual vector<double> v_simulateCost( Action &currentAction,
					   const vector<int> &newValue,
					   vector< vector<double> > &vecVarSimCosts )
    {
      vector<double> simCosts( domain->getSize(), -1. );
      int backup = currentAction.getValue();
      int previousPos = 0;

      for( auto &pos : newValue )
      {
	if( pos >= 1 && pos == previousPos + 1 )
	{
	  domain->quickShift( currentAction );
	}
	else
	{ 
	  domain->clear( currentAction );
	  currentAction.setValue( pos );
	  domain->add( currentAction );
	}

	simCosts[pos+1] = cost( vecVarSimCosts[pos+1] );
	previousPos = pos;
      }

      domain->clear( currentAction );
      currentAction.setValue( backup );
      domain->add( currentAction );

      return simCosts;
    }
  };  


  /***********/
  /* Overlap */
  /***********/  
  class Overlap : public BuildOrderConstraint
  {
  public:
    Overlap( const vector< Action >*, const BuildOrderDomain* );
    
    double v_cost( vector<double>& ) const;
    vector<double> v_simulateCost( Action&, const vector<int>&, vector< vector<double> >& );
  };

  
  /*************/
  /* Buildable */
  /*************/  
  class Buildable : public BuildOrderConstraint
  {
  public:
    Buildable( const vector< Action >*, const BuildOrderDomain* );
    
    double v_cost( vector<double>& ) const;
    vector<double> v_simulateCost( Action&, const vector<int>&, vector< vector<double> >& );
  };

  
  /**********/
  /* NoGaps */
  /**********/  
  class NoGaps : public BuildOrderConstraint
  {
  public:
    NoGaps( const vector< Action >*, const BuildOrderDomain* );
    
    double v_cost( vector<double>& ) const;
    double postprocess_simulateCost( Action&, const int, vector<double>& );
  };

  
  /***********************/
  /* StartingTargetTiles */
  /***********************/  
  class StartingTargetTiles : public BuildOrderConstraint
  {
  public:
    StartingTargetTiles( const vector< Action >*, const BuildOrderDomain* );

    double v_cost( vector<double>& ) const;
  private:
    map<int, Action*> mapActions;
  };
}
