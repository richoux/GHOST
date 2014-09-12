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


#include <vector>
#include <map>
#include <memory>

#include "../../include/objectives/targetSelectionObjective.hpp"

using namespace std;

namespace ghost
{
  /****************************/
  /* TargetSelectionObjective */
  /****************************/

  TargetSelectionObjective::TargetSelectionObjective( const string &name )
    : Objective<Unit, TargetSelectionDomain>( name )
  {
    
  }

  TargetSelectionObjective::TargetSelectionObjective( const string &name, const vector< pair<string, int> > &input, vector<Unit> &variables )
    : Objective<Unit, TargetSelectionDomain>( name )
  {

  }
    
  int TargetSelectionObjective::v_heuristicVariable( const vector< int > &vecId, const vector< Unit > *vecVariables, TargetSelectionDomain *domain )
  {
    // select first units that are about to shoot, and then splash units first among them.
    auto min =  min_element( vecId.begin(), 
			     vecId.end(), 
			     [&](int b1, int b2)
			     { return vecVariables->at(b1).canShootIn() < vecVariables->at(b2).canShootIn(); } );

    vector< int > varMinShoot( vecId.size() );
    
    auto it_shoot = copy_if( vecId.begin(),
			     vecId.end(),
			     varMinShoot.begin(),
			     [&](int b){return vecVariables->at(b).canShootIn() == *min;} );

    varMinShoot.resize( distance( varMinShoot.begin(), it_shoot ) );
    varMinShoot.shrink_to_fit();    
    vector< int > varSplash( vecMinShoot.size() );

    auto it_splash = copy_if( varMinShoot.begin(),
			      varMinShoot.end(),
			      varSplash.begin(),
			      [&](int b){return vecVariables->at(b).isSplash();} );
    
    if( it_splash == varSplash.begin() )
      return vecMinShoot[ randomVar.getRandNum( varMinShoot.size() ) ];    
    else
      return vecSplash[ randomVar.getRandNum( distance( varSplash.begin(), it_splash ) ) ];    
  }

  int TargetSelectionObjective::v_heuristicValue( const vector< double > &vecGlobalCosts, 
						  double &bestEstimatedCost,
						  int &bestValue ) const
  {

  }

  void TargetSelectionObjective::v_setHelper( const Unit &b, const vector< Unit > *vecVariables, const TargetSelectionDomain *domain )
  {

  }

  double TargetSelectionObjective::v_postprocessOptimization( vector< Unit > *vecUnits,
							      TargetSelectionDomain *domain,
							      double &bestCost,
							      double opt_timeout )
  {

  }

  double TargetSelectionObjective::v_postprocessSatisfaction( vector< Unit > *vecVariables,
							      TargetSelectionDomain *domain,
							      double &bestCost,
							      vector< Unit > &bestSolution,
							      double sat_timeout ) const
  {

  }

  
  /*************/
  /* MaxDamage */
  /*************/
  MaxDamage::MaxDamage()
    : TargetSelectionObjective( "MaxDamage" )
  {

  }
  
  MaxDamage::MaxDamage( const vector< pair<string, int> > &input, vector<Unit> &variables )
    : TargetSelectionObjective( "MaxDamage" )
  {

  }

  double MaxDamage::v_cost( vector< Unit > *vecVariables, TargetSelectionDomain *domain ) const
  {

  }

}
