/*
 * GHOST (General meta-Heuristic Optimization Solving Tool) is a C++ library 
 * designed to help developers to model and implement optimization problem 
 * solving. 
 * GHOST is a meta-heuristic solver aiming to solve any kind of combinatorial 
 * and optimization real-time problems represented by a CSP/COP. 
 * GHOST has been first developped to help making AI for the RTS game
 * StarCraft: Brood war, but can be used for any kind of applications where 
 * solving combinatorial and optimization problems within some tenth of 
 * milliseconds is needed. It is a generalization of the Wall-in project.
 * Please visit https://github.com/richoux/GHOST for further information.
 * 
 * Copyright (C) 2014-2016 Florian Richoux
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

#include "../../src/constraint.hpp"
#include "../../src/objective.hpp"
#include "building.hpp"
#include "wallinDomain.hpp"

using namespace std;

namespace ghost
{
  class WallinConstraint : public Constraint<Building, WallinDomain>
  {
  public:
    WallinConstraint( const vector< Building >*, const WallinDomain* );

  protected:
    virtual vector<double> v_simulateCost( Building &oldBuilding,
					   const vector<int> &newPosition,
					   vector< vector<double> > &vecVarSimCosts,
					   shared_ptr< Objective< Building, WallinDomain > > objective )
    {
      vector<double> simCosts( domain->getSize(), -1. );
      int backup = oldBuilding.getValue();
      int previousPos = 0;

      if( objective )
	objective->resetHelper();
      
      for( const auto &pos : newPosition )
      {
	if( pos >= 1 && pos == previousPos + 1 )
	{
	  domain->quickShift( oldBuilding );
	}
	else
	{ 
	  domain->clear( oldBuilding );
	  oldBuilding.setValue( pos );
	  domain->add( oldBuilding );
	}

	simCosts[pos+1] = cost( vecVarSimCosts[pos+1] );

	if( objective )
	  objective->setHelper( oldBuilding, variables, domain );

	previousPos = pos;
      }

      domain->clear( oldBuilding );
      oldBuilding.setValue( backup );
      domain->add( oldBuilding );

      return simCosts;
    }

    bool isWall() const;
  };  


  /***********/
  /* Overlap */
  /***********/  
  class Overlap : public WallinConstraint
  {
  public:
    Overlap( const vector< Building >*, const WallinDomain* );

  private:
    double v_cost( vector<double>& ) const;
    vector<double> v_simulateCost( Building&,
				   const vector<int>&,
				   vector< vector<double> >&,
				   shared_ptr< Objective< Building, WallinDomain > > );
  };

  
  /*************/
  /* Buildable */
  /*************/  
  class Buildable : public WallinConstraint
  {
  public:
    Buildable( const vector< Building >*, const WallinDomain* );

  private:
    double v_cost( vector<double>& ) const;
    vector<double> v_simulateCost( Building&,
				   const vector<int>&,
				   vector< vector<double> >&,
				   shared_ptr< Objective< Building, WallinDomain > > );
  };

  
  /**********/
  /* NoHoles */
  /**********/  
  class NoHoles : public WallinConstraint
  {
  public:
    NoHoles( const vector< Building >*, const WallinDomain* );

    double postprocess_simulateCost( Building&, const int, vector<double>& );
    
  private:
    double v_cost( vector<double>& ) const;
  };

  
  /***********************/
  /* StartingTargetTiles */
  /***********************/  
  class StartingTargetTiles : public WallinConstraint
  {
  public:
    StartingTargetTiles( const vector< Building >*, const WallinDomain* );

  private:
    double v_cost( vector<double>& ) const;

    map<int, Building*> mapBuildings;
  };
}
