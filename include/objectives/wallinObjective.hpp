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
					      vector< Building > &bestSolution ) const;
    
    virtual double v_postprocessOptimization( vector< Building > *vecBuildings,
					      WallinDomain *domain,
					      double &bestCost );

    
    static int sizeWall;
  };
  
  // /***********/
  // /* NoneObj */
  // /***********/
  // class NoneObj : public WallinObjective
  // {
  // public:
  //   NoneObj();

  // private:
  //   double v_cost( const vector< Building > *vecVariables, const WallinDomain *domain ) const;
  //   int v_heuristicVariable( const vector< int > &vecId, const vector< Building > *vecVariables, WallinDomain *domain );
  //   void v_setHelper( const Building &b, const vector< Building > *vecVariables, const WallinDomain *domain );
  //   double v_postprocessOptimization( vector< Building > *vecVariables, WallinDomain *domain, double &bestCost );
  // };

  /**********/
  /* GapObj */
  /**********/
  class GapObj : public WallinObjective
  {
  public:
    GapObj();

  private:
    double v_cost( const vector< Building > *vecVariables, const WallinDomain *domain ) const;
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
    double v_cost( const vector< Building > *vecVariables, const WallinDomain *domain ) const;
    int v_heuristicVariable( const vector< int > &vecId, const vector< Building > *vecVariables, WallinDomain *domain );
    double v_postprocessOptimization( vector< Building > *vecVariables, WallinDomain *domain, double &bestCost );
  };

  /***************/
  /* TreeTechObj */
  /***************/
  class TechTreeObj : public WallinObjective
  {
  public:
    TechTreeObj();

  private:
    double v_cost( const vector< Building > *vecVariables, const WallinDomain *domain ) const;
    int v_heuristicVariable( const vector< int > &vecId, const vector< Building > *vecVariables, WallinDomain *domain );
  };
}
