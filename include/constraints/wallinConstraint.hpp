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


#pragma once

#include <vector>
#include <iostream>
#include <memory>
#include <map>

#include "constraint.hpp"
#include "../variables/building.hpp"
#include "../domains/wallinDomain.hpp"
#include "../objectives/objective.hpp"

using namespace std;

namespace ghost
{
  class WallinConstraint : public Constraint<Building, WallinDomain>
  {
  public:
    WallinConstraint( const vector< Building >*, const WallinDomain* );

    virtual vector<double> simulateCost( Building &oldBuilding,
				 const vector<int> &newPosition,
				 vector< vector<double> > &vecVarSimCosts,
				 shared_ptr< Objective< Building, WallinDomain > > objective )
    {
      std::vector<double> simCosts( domain->getSize(), -1. );
      int backup = oldBuilding.getValue();
      // int index = oldBuilding.getId();
      int previousPos = 0;
    
      if( objective )
	objective->resetHelper();

      for( auto &pos : newPosition )
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

	//variables->at( index ).setValue( pos );
	
	simCosts[pos+1] = cost( vecVarSimCosts[pos+1] );
	if( objective )
	  objective->setHelper( oldBuilding, variables, domain );

	previousPos = pos;
      }

      domain->clear( oldBuilding );
      oldBuilding.setValue( backup );
      domain->add( oldBuilding );

      //variables->at( index ).setValue( backup );

      return simCosts;
    }

    virtual vector<double> simulateCost( Building &oldBuilding,
					 const vector<int> &newPosition,
					 vector< vector<double> > &vecVarSimCosts )
    {
      return simulateCost( oldBuilding, newPosition, vecVarSimCosts, nullptr );
    }

  protected:
    bool isWall() const;
  };  


  /***********/
  /* Overlap */
  /***********/  
  class Overlap : public WallinConstraint
  {
  public:
    Overlap( const vector< Building >*, const WallinDomain* );
    
    double cost( vector<double>& ) const;
    vector<double> simulateCost( Building&, const vector<int>&, vector< vector<double> >& );
  };

  
  /*************/
  /* Buildable */
  /*************/  
  class Buildable : public WallinConstraint
  {
  public:
    Buildable( const vector< Building >*, const WallinDomain* );
    
    double cost( vector<double>& ) const;
    vector<double> simulateCost( Building&, const vector<int>&, vector< vector<double> >& );
  };

  
  /**********/
  /* NoGaps */
  /**********/  
  class NoGaps : public WallinConstraint
  {
  public:
    NoGaps( const vector< Building >*, const WallinDomain* );
    
    double cost( vector<double>& ) const;
    double simulateCost( Building&, const int, vector<double>& );
  };

  
  /***********************/
  /* StartingTargetTiles */
  /***********************/  
  class StartingTargetTiles : public WallinConstraint
  {
  public:
    StartingTargetTiles( const vector< Building >*, const WallinDomain* );

    double cost( vector<double>& ) const;
  private:
    map<int, Building*> mapBuildings;
  };
}
