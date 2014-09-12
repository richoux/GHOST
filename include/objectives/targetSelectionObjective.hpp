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
#include <map>
#include <memory>

#include "objective.hpp"
#include "../variables/unit.hpp"
#include "../misc/unitMap.hpp"
#include "../domains/targetSelectionDomain.hpp"

using namespace std;

namespace ghost
{
  /****************************/
  /* TargetSelectionObjective */
  /****************************/
  class TargetSelectionObjective : public Objective<Unit, TargetSelectionDomain>
  {
  public:
    TargetSelectionObjective( const string &name );
    TargetSelectionObjective( const string &name, const vector< pair<string, int> > &input, vector<Unit> &variables );

  protected:
    int v_heuristicVariable( const vector< int > &vecId, const vector< Unit > *vecVariables, TargetSelectionDomain *domain );
    int v_heuristicValue( const vector< double > &vecGlobalCosts, 
			  double &bestEstimatedCost,
			  int &bestValue ) const;
    void v_setHelper( const Unit &b, const vector< Unit > *vecVariables, const TargetSelectionDomain *domain );

    double v_postprocessOptimization( vector< Unit > *vecUnits,
				      TargetSelectionDomain *domain,
				      double &bestCost,
				      double opt_timeout );

    double v_postprocessSatisfaction( vector< Unit > *vecVariables,
				      TargetSelectionDomain *domain,
				      double &bestCost,
				      vector< Unit > &bestSolution,
				      double sat_timeout ) const;
  };
  
  
  /*************/
  /* MaxDamage */
  /*************/
  class MaxDamage : public TargetSelectionObjective
  {
  public:
    MaxDamage();
    MaxDamage( const vector< pair<string, int> > &input, vector<Unit> &variables );
    double v_cost( vector< Unit > *vecVariables, TargetSelectionDomain *domain ) const;
  };
}
