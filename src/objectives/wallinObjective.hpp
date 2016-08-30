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
#include <memory>

#include "objective.hpp"
#include "../variables/building.hpp"
#include "../domains/wallinDomain.hpp"

using namespace std;

namespace ghost
{
  /*******************/
  /* WallinObjective */
  /*******************/
  class WallinObjective : public Objective<Building, WallinDomain>
  {
  public:
    WallinObjective( const string & );

  protected:
    virtual void v_setHelper( const Building &b, const vector< Building > *vecVariables, const WallinDomain *domain );

    virtual double v_postprocessSatisfaction( vector< Building > *vecVariables,
					      WallinDomain *domain,
					      double &bestCost,
					      vector< Building > &bestSolution,
					      double sat_timeout) const;
    
    virtual double v_postprocessOptimization( vector< Building > *vecBuildings,
					      WallinDomain *domain,
					      double &bestCost,
					      double opt_timeout);

    
    static int sizeWall;
  };
  
  /**********/
  /* GapObj */
  /**********/
  class GapObj : public WallinObjective
  {
  public:
    GapObj();

  private:
    double v_cost( vector< Building > *vecVariables, WallinDomain *domain ) const;
    int v_heuristicVariable( const vector< int > &vecId, const vector< Building > *vecVariables, WallinDomain *domain );
    void v_setHelper( const Building &b, const vector< Building > *vecVariables, const WallinDomain *domain );
    int gapSize( const Building &b, const vector< Building > *vecVariables, const WallinDomain *domain ) const;
  };

  /***************/
  /* BuildingObj */
  /***************/
  class BuildingObj : public WallinObjective
  {
  public:
    BuildingObj();

  private:
    double v_cost( vector< Building > *vecVariables, WallinDomain *domain ) const;
    int v_heuristicVariable( const vector< int > &vecId, const vector< Building > *vecVariables, WallinDomain *domain );
    double v_postprocessOptimization( vector< Building > *vecVariables, WallinDomain *domain, double &bestCost, double opt_timeout );
  };

  /***************/
  /* TreeTechObj */
  /***************/
  class TechTreeObj : public WallinObjective
  {
  public:
    TechTreeObj();

  private:
    double v_cost( vector< Building > *vecVariables, WallinDomain *domain ) const;
    int v_heuristicVariable( const vector< int > &vecId, const vector< Building > *vecVariables, WallinDomain *domain );
  };
}
