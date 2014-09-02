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
#include "../variables/unit.hpp"
#include "../domains/targetSelectionDomain.hpp"
#include "../objectives/objective.hpp"
#include "../misc/unitMap.hpp"

using namespace std;

namespace ghost
{
  class TargetSelectionConstraint : public Constraint<Unit, TargetSelectionDomain>
  {
  public:
    TargetSelectionConstraint( const vector< Unit >*, const TargetSelectionDomain* );


  private:
    double		v_cost	      ( vector<double> &varCost ) const;

    vector<double>	v_simulateCost( Unit &currentUnit,
					const vector<int> &newTarget,
					vector< vector<double> > &vecVarSimCosts,
					shared_ptr< Objective< Unit, TargetSelectionDomain > > objective );
  };  
}
