/*
 * GHOST (General meta-Heuristic Optimization Solving Tool) is a C++ library 
 * designed to help developers to model and implement optimization problem 
 * solving. It contains a meta-heuristic solver aiming to solve any kind of 
 * combinatorial and optimization real-time problems represented by a CSP/COP. 
 *
 * GHOST has been first developped to help making AI for the RTS game
 * StarCraft: Brood war, but can be used for any kind of applications where 
 * solving combinatorial and optimization problems within some tenth of 
 * milliseconds is needed. It is a generalization of the Wall-in project.
 * Please visit https://github.com/richoux/GHOST for further information.
 * 
 * Copyright (C) 2014-2017 Florian Richoux
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

#include "objective.hpp"

using namespace ghost;

Objective::Objective( const string& name )
  : name(name)
{ }
  
int Objective::expert_heuristic_value( const vector< Variable > &variables,
				       Variable &var,
				       const vector< int >& possible_values ) const
{
  double minCost = numeric_limits<double>::max();
  double simulatedCost;
  
  int backup = var.get_value();
  vector<int> bestValues;
  
  for( auto& v : possible_values )
  {
    var.set_value( v );
    simulatedCost = cost( variables );
    
    if( minCost > simulatedCost )
    {
      minCost = simulatedCost;
      bestValues.clear();
      bestValues.push_back( v );
    }
    else
      if( minCost == simulatedCost )
	bestValues.push_back( v );
  }
  
  var.set_value( backup );
  
  return bestValues[ random.get_random_number( bestValues.size() ) ];
}
  
Variable* Objective::expert_heuristic_value( const vector< Variable* > &bad_variables ) const
{
  return bad_variables[ random.get_random_number( bad_variables.size() ) ];
}
 
void Objective::expert_postprocess_satisfaction( vector< Variable > &variables,
						 double& bestCost,
						 vector< int >& solution ) const
{ }

void Objective::expert_postprocess_optimization( vector< Variable > &variables,
						 double& bestCost,
						 vector< int >& solution ) const
{ }

