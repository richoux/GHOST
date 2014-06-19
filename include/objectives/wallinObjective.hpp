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
    virtual void v_setHelper( const Building &b, const vector< Building > *vecVariables, const WallinDomain *domain );

    virtual void v_postprocessSatisfaction( vector< Building > *vecVariables,
				    WallinDomain *domain,
				    double &bestCost,
				    vector<int> &bestSolution ) const;

    virtual void v_postprocessOptimization( const vector< Building > *vecBuildings,
				    WallinDomain *domain,
				    double &bestCost );

  protected:
    static int sizeWall;
  };
  
  /***********/
  /* NoneObj */
  /***********/
  class NoneObj : public WallinObjective
  {
  public:
    NoneObj();
    double v_cost( const vector< Building > *vecVariables, const WallinDomain *domain ) const;
    int v_heuristicVariable( const vector< int > &vecId, const vector< Building > *vecVariables, WallinDomain *domain );
    void v_setHelper( const Building &b, const vector< Building > *vecVariables, const WallinDomain *domain );
    void v_postprocessOptimization( const vector< Building > *vecVariables, const WallinDomain *domain, double &bestCost );
  };

  /**********/
  /* GapObj */
  /**********/
  class GapObj : public WallinObjective
  {
  public:
    GapObj();
    double v_cost( const vector< Building > *vecVariables, const WallinDomain *domain ) const;
    int v_heuristicVariable( const vector< int > &vecId, const vector< Building > *vecVariables, WallinDomain *domain );
    void v_setHelper( const Building &b, const vector< Building > *vecVariables, const WallinDomain *domain );
  private:
    int gapSize( const Building &b, const vector< Building > *vecVariables, const WallinDomain *domain ) const;
  };

  /***************/
  /* BuildingObj */
  /***************/
  class BuildingObj : public WallinObjective
  {
  public:
    BuildingObj();
    double v_cost( const vector< Building > *vecVariables, const WallinDomain *domain ) const;
    int v_heuristicVariable( const vector< int > &vecId, const vector< Building > *vecVariables, WallinDomain *domain );
    void v_postprocessOptimization( const vector< Building > *vecVariables, const WallinDomain *domain, double &bestCost );
  };

  /***************/
  /* TreeTechObj */
  /***************/
  class TechTreeObj : public WallinObjective
  {
  public:
    TechTreeObj();
    double v_cost( const vector< Building > *vecVariables, const WallinDomain *domain ) const;
    int v_heuristicVariable( const vector< int > &vecId, const vector< Building > *vecVariables, WallinDomain *domain );
  };
}
