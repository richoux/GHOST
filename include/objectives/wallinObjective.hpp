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

#include "../variable/building.hpp"
#include "../domain/wallinGrid.hpp"

using namespace std;

namespace ghost
{
  /*******************/
  /* WallinObjective */
  /*******************/
  class WallinObjective : public Objective<Building, WallinGrid>
  {
  public:
    WallinObjective( string );
    void v_setHelper( const Building &b, const vector< Building > &vecVariables, const WallinGrid &grid );
    void v_postprocessSatisfaction( const vector< TypeVariable > &vecVariables,
				    const TypeDomain &domain,
				    double &bestCost,
				    vector<int> &bestSolution ) const;
  protected:
    static int sizeWall;
  };
  
  /***********/
  /* NoneObj */
  /***********/
  class NoneObj : public WallinObjective
  {
  public:
    NoneObj( string );
    double v_cost( const vector< Building > &vecVariables, const WallinGrid& ) const;
    int v_heuristicVariable( const vector< int > &vecId, const vector< Building > &vecVariables, const WallinGrid& );
    void v_setHelper( const Building&, const vector< Building >&, const WallinGrid& );
    void v_postprocessOptimization( const vector< Building > &vecVariables, const WallinGrid &domain, double &bestCost ) const;
  };

  /**********/
  /* GapObj */
  /**********/
  class GapObj : public WallinObjective
  {
  public:
    GapObj( string );
    double v_cost( const vector< Building > &vecVariables, const WallinGrid& ) const;
    int v_heuristicVariable( const vector< int > &vecId, const vector< Building > &vecVariables, const WallinGrid& );
    void v_setHelper( const Building&, const vector< Building >&, const WallinGrid& );
  private:
    int gapSize( const Building&, const vector< Building >&, const WallinGrid& ) const;
  };

  /***************/
  /* BuildingObj */
  /***************/
  class BuildingObj : public WallinObjective
  {
  public:
    BuildingObj( string );
    double v_cost( const vector< Building > &vecVariables, const WallinGrid& ) const;
    int v_heuristicVariable( const vector< int > &vecId, const vector< Building > &vecVariables, const WallinGrid& );
    void v_postprocessOptimization( const vector< Building > &vecVariables, const WallinGrid &domain, double &bestCost ) const;
  };

  /***************/
  /* TreeTechObj */
  /***************/
  class TechTreeObj : public WallinObjective
  {
  public:
    TechTreeObj( string );
    double v_cost( const vector< Building > &vecVariables, const WallinGrid& ) const;
    int v_heuristicVariable( const vector< int > &vecId, const vector< Building > &vecVariables, const WallinGrid& );
  };

  class FactoryObj
  {
  public:
    shared_ptr<Objective> makeObjective( const string& ) const;
  };
}
