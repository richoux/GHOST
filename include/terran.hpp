/*
 * GHOST (General meta-Heuristic Optimization Solving Tool) is a C++ library 
 * designed for StarCraft: Brood war. 
 * GHOST is a meta-heuristic solver aiming to solve any kind of combinatorial 
 * and optimization RTS-related problems represented by a CSP. It is an extension
 * of a previous project Wall-in.
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
 * along with Wall-in. If not, see http://www.gnu.org/licenses/.
 */


#pragma once

#include <vector>
#include <memory>

#include "building.hpp"
#include "constraint.hpp"
#include "grid.hpp"

using namespace std;

namespace wallin
{
  // Academy
  shared_ptr<Building> a1;
  shared_ptr<Building> a2;

  // Armory
  shared_ptr<Building> r1;
  shared_ptr<Building> r2;

  // Barracks
  shared_ptr<Building> b1;
  shared_ptr<Building> b2;

  // Bunker
  shared_ptr<Building> u1;
  shared_ptr<Building> u2;

  // Command Center
  shared_ptr<Building> c1;
  shared_ptr<Building> c2;

  // Engineering Bay
  shared_ptr<Building> e1;
  shared_ptr<Building> e2;

  // Factory
  shared_ptr<Building> f1;
  shared_ptr<Building> f2;

  // Missile Turret
  shared_ptr<Building> t1;
  shared_ptr<Building> t2;
  
  // Science Facility
  shared_ptr<Building> i1;
  shared_ptr<Building> i2;

  // Starport
  shared_ptr<Building> p1;
  shared_ptr<Building> p2;  

  // Supply Depot
  shared_ptr<Building> s1;
  shared_ptr<Building> s2;

  shared_ptr<Constraint> overlap;
  shared_ptr<Constraint> buildable;
  shared_ptr<Constraint> noGaps;
  shared_ptr<Constraint> specialTiles;

  // vector<shared_ptr<Building> > makeTerranBuildings( std::string obj )
  // {
  //   a1 = make_shared<Academy>( );
  //   a2 = make_shared<Academy>( );
  //   b1 = make_shared<Barracks>( );
  //   b2 = make_shared<Barracks>( );
  //   u1 = make_shared<Bunker>( );
  //   u2 = make_shared<Bunker>( );
  //   f1 = make_shared<Factory>( );
  //   f2 = make_shared<Factory>( );
  //   s1 = make_shared<SupplyDepot>( );
  //   s2 = make_shared<SupplyDepot>( );

  //   if( obj.compare("treetech") == 0 || obj.compare("t") == 0 || obj.compare("T") == 0)
  //   {
  //     r1 = make_shared<Armory>( );
  //     r2 = make_shared<Armory>( );
  //     c1 = make_shared<CommandCenter>( );
  //     c2 = make_shared<CommandCenter>( );
  //     e1 = make_shared<EngineeringBay>( );
  //     e2 = make_shared<EngineeringBay>( );
  //     t1 = make_shared<MissileTurret>( );
  //     t2 = make_shared<MissileTurret>( );
  //     i1 = make_shared<ScienceFacility>( );
  //     i2 = make_shared<ScienceFacility>( );
  //     p1 = make_shared<Starport>( );
  //     p2 = make_shared<Starport>( );

  //     vector< shared_ptr<Building> > vec {a1, a2, r1, r2, b1, b2, u1, u2, c1, c2, e1, e2, f1, f2, t1, t2, i1, i2, p1, p2, s1, s2};
  //     return vec;
  //   }
  //   else
  //   {
  //     vector< shared_ptr<Building> > vec {a1, a2, b1, b2, u1, u2, f1, f2, s1, s2};
  //     return vec;
  //   }
  // }

  vector<shared_ptr<Building> > makeTerranBuildings()
  {
    a1 = make_shared<Academy>( );
    a2 = make_shared<Academy>( );
    // r1 = make_shared<Armory>( );
    // r2 = make_shared<Armory>( );
    b1 = make_shared<Barracks>( );
    b2 = make_shared<Barracks>( );
    u1 = make_shared<Bunker>( );
    u2 = make_shared<Bunker>( );
    // c1 = make_shared<CommandCenter>( );
    // c2 = make_shared<CommandCenter>( );
    // e1 = make_shared<EngineeringBay>( );
    // e2 = make_shared<EngineeringBay>( );
    f1 = make_shared<Factory>( );
    f2 = make_shared<Factory>( );
    // t1 = make_shared<MissileTurret>( );
    // t2 = make_shared<MissileTurret>( );
    // i1 = make_shared<ScienceFacility>( );
    // i2 = make_shared<ScienceFacility>( );
    // p1 = make_shared<Starport>( );
    // p2 = make_shared<Starport>( );
    s1 = make_shared<SupplyDepot>( );
    s2 = make_shared<SupplyDepot>( );
    
    // vector< shared_ptr<Building> > vec {a1, a2, r1, r2, b1, b2, u1, u2, c1, c2, e1, e2, f1, f2, t1, t2, i1, i2, p1, p2, s1, s2};
    vector< shared_ptr<Building> > vec {a1, a2, b1, b2, u1, u2, f1, f2, s1, s2};
    return vec;
  }

  vector< shared_ptr<Constraint> > makeTerranConstraints( const vector<shared_ptr<Building> >& vec, const Grid& grid )
  {
    overlap	 = make_shared<Overlap>( vec, grid );
    buildable	 = make_shared<Buildable>( vec, grid );
    noGaps	 = make_shared<NoGaps>( vec, grid );
    specialTiles = make_shared<StartingTargetTiles>( vec, grid );
    
    vector< shared_ptr<Constraint> > vecConstraints {overlap, buildable, noGaps, specialTiles};
    return vecConstraints;
  }
}
