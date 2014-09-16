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
#include <algorithm>
#include <chrono>
#include <ctime>
#include <cstdlib>

#include "../../include/objectives/targetSelectionObjective.hpp"

using namespace std;

namespace ghost
{
  /****************************/
  /* TargetSelectionObjective */
  /****************************/

  TargetSelectionObjective::TargetSelectionObjective( const string &name ) : Objective<Unit, TargetSelectionDomain>( name ) { }

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

    if( it_shoot == varMinShoot.begin() )
      return vecId[ randomVar.getRandNum( vecId.size() ) ];

    varMinShoot.resize( distance( varMinShoot.begin(), it_shoot ) );
    varMinShoot.shrink_to_fit();    
    vector< int > varSplash( varMinShoot.size() );

    auto it_splash = copy_if( varMinShoot.begin(),
			      varMinShoot.end(),
			      varSplash.begin(),
			      [&](int b){return vecVariables->at(b).isSplash();} );
    
    // cout << "varMinShoot.size() = " << varMinShoot.size() << endl;

    if( it_splash == varSplash.begin() )
      return varMinShoot[ randomVar.getRandNum( varMinShoot.size() ) ];    
    else
      return varSplash[ randomVar.getRandNum( distance( varSplash.begin(), it_splash ) ) ];    
  }


  double TargetSelectionObjective::v_postprocessSatisfaction( vector< Unit > *vecVariables,
							      TargetSelectionDomain *domain,
							      double &bestCost,		
							      vector< Unit > &bestSolution,
							      double sat_timeout ) const 

  {
    chrono::time_point<chrono::high_resolution_clock> startPostprocess = chrono::high_resolution_clock::now(); 
    chrono::duration<double,micro> postprocesstimer(0);

    double cost = v_cost( vecVariables, domain );
    
    if( cost < bestCost )
    {
      bestCost = cost;
      copy( begin(*vecVariables), end(*vecVariables), begin(bestSolution) );
    }

    postprocesstimer = chrono::high_resolution_clock::now() - startPostprocess;
    return postprocesstimer.count();
  }

  
  /*************/
  /* MaxDamage */
  /*************/
  MaxDamage::MaxDamage() : TargetSelectionObjective( "MaxDamage" ) { }
  
  double MaxDamage::v_cost( vector< Unit > *vecVariables, TargetSelectionDomain *domain ) const
  {
    double damages = 0.;
    vector<double> hits;
    vector<UnitEnemy> *enemies = domain->getAllEnemies();

    for( const auto &v : *vecVariables )
      if( v.getValue() != -1 )
      {
	hits = v.computeDamage( enemies );
	for_each( begin( hits ), end( hits ), [&](double d){damages += d;} );
      }

    return 1. / damages;
  }

  void MaxDamage::v_setHelper( const Unit &u, const vector< Unit > *vecVariables, const TargetSelectionDomain *domain )
  {
    if( !u.isDead() && u.canShoot() && u.getValue() != -1 )
    {
      auto hits = u.computeDamage( domain->getAllEnemies() );
      heuristicValueHelper.at( u.getValue() + 1 ) = 1. / hits.at( u.getValue() );
    }
  }


  /***********/
  /* MaxKill */
  /***********/
  MaxKill::MaxKill() : TargetSelectionObjective( "MaxKill" ) { }
  
  double MaxKill::v_cost( vector< Unit > *vecVariables, TargetSelectionDomain *domain ) const
  {
    vector<double> hits;
    vector<UnitEnemy> *enemies = domain->getAllEnemies();

    vector<UnitEnemy> copyEnemies(*enemies);

    for( const auto &v : *vecVariables )
    {
      if( v.getValue() != -1 )
      {
	hits = v.computeDamage( enemies );
	for( int i = 0 ; i < copyEnemies.size() ; ++i )
	  copyEnemies[i].data.hp -= hits[i];
      }
    }

    return 1. / count_if( begin(copyEnemies), end(copyEnemies), [](UnitEnemy &u){ return u.isDead(); } );
  }

  void MaxKill::v_setHelper( const Unit &u, const vector< Unit > *vecVariables, const TargetSelectionDomain *domain )
  {
    if( !u.isDead() && u.canShoot() && u.getValue() != -1 )
    {
      auto hits = u.computeDamage( domain->getAllEnemies() );
      auto enemyHP = domain->getEnemyData( u.getValue() ).data.hp;

      if( enemyHP <= 0 )
	heuristicValueHelper.at( u.getValue() + 1 ) = 1000.;	
      else if( enemyHP <= hits.at( u.getValue() ) )
	heuristicValueHelper.at( u.getValue() + 1 ) = 0.;
      else
	heuristicValueHelper.at( u.getValue() + 1 ) = 1. / hits.at( u.getValue() );
	// heuristicValueHelper.at( u.getValue() + 1 ) = std::abs( enemyHP - hits.at( u.getValue() ) );
    }
  }

}
