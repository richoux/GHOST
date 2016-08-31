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


#include <iostream>
#include <numeric>
#include <algorithm>
#include <chrono>
#include <typeinfo>

#include "targetSelectionDomain.hpp"
#include "../misc/unitMap.hpp"

using namespace std;

namespace ghost
{
  TargetSelectionDomain::TargetSelectionDomain( int numberVariables, vector<UnitEnemy> *data )
    : Domain( numberVariables + 1, numberVariables, -1 ), enemies(data)
  { }
  
  void TargetSelectionDomain::v_restart( vector<Unit> *variables )
  {
    for( auto &v : *variables )
    {
      v.setValue( -1 );
      // v.setData( unitOf[ v.getFullName() ] );
    }
  }

  vector<UnitEnemy> TargetSelectionDomain::getEnemiesInRange( const Unit &u )
  {
    vector<UnitEnemy> inRange;

    for( const auto &e : *enemies )
      if( u.isInRange( e ) )
	inRange.push_back( e );
    
    return inRange;
  }

  vector<UnitEnemy> TargetSelectionDomain::getLivingEnemiesInRange( const Unit &u )
  {
    vector<UnitEnemy> inRange;

    for( const auto &e : *enemies )
      if( u.isInRangeAndAlive( e ) )
	inRange.push_back( e );
    
    return inRange;
  }

  ostream& operator<<( ostream &os, const TargetSelectionDomain &t )
  {
    os << endl;
    for( int i = 0 ; i < t.enemies->size() ; ++i )
      os << t.enemies->at(i).data
	 << "Value: " << i << endl
	 << "Coord: (" << t.enemies->at(i).coord.x << ", " << t.enemies->at(i).coord.y << ")" << endl
	 << "-------" << endl << endl;


    return os;
  }
}
