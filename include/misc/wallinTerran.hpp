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

#include "../variables/wallinTerranBuildings.hpp"
#include "../constraints/wallinConstraint.hpp"
#include "../domains/wallinGrid.hpp"

using namespace std;

namespace ghost
{
  // vector<Building > makeTerranBuildings( std::string obj )
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

  //     vector< Building > vec {a1, a2, r1, r2, b1, b2, u1, u2, c1, c2, e1, e2, f1, f2, t1, t2, i1, i2, p1, p2, s1, s2};
  //     return vec;
  //   }
  //   else
  //   {
  //     vector< Building > vec {a1, a2, b1, b2, u1, u2, f1, f2, s1, s2};
  //     return vec;
  //   }
  // }

  vector< Building > makeTerranBuildings()
  {
    // Academy
    Building a1;
    Building a2;

    // Armory
    Building r1;
    Building r2;

    // Barracks
    Building b1;
    Building b2;

    // Bunker
    Building u1;
    Building u2;

    // Command Center
    Building c1;
    Building c2;

    // Engineering Bay
    Building e1;
    Building e2;

    // Factory
    Building f1;
    Building f2;

    // Missile Turret
    Building t1;
    Building t2;
  
    // Science Facility
    Building i1;
    Building i2;

    // Starport
    Building p1;
    Building p2;  

    // Supply Depot
    Building s1;
    Building s2;

    // a1 = make_shared<Academy>( );
    // a2 = make_shared<Academy>( );
    // // r1 = make_shared<Armory>( );
    // // r2 = make_shared<Armory>( );
    // b1 = make_shared<Barracks>( );
    // b2 = make_shared<Barracks>( );
    // u1 = make_shared<Bunker>( );
    // u2 = make_shared<Bunker>( );
    // // c1 = make_shared<CommandCenter>( );
    // // c2 = make_shared<CommandCenter>( );
    // // e1 = make_shared<EngineeringBay>( );
    // // e2 = make_shared<EngineeringBay>( );
    // f1 = make_shared<Factory>( );
    // f2 = make_shared<Factory>( );
    // // t1 = make_shared<MissileTurret>( );
    // // t2 = make_shared<MissileTurret>( );
    // // i1 = make_shared<ScienceFacility>( );
    // // i2 = make_shared<ScienceFacility>( );
    // // p1 = make_shared<Starport>( );
    // // p2 = make_shared<Starport>( );
    // s1 = make_shared<SupplyDepot>( );
    // s2 = make_shared<SupplyDepot>( );
    
    // vector< Building > vec {a1, a2, r1, r2, b1, b2, u1, u2, c1, c2, e1, e2, f1, f2, t1, t2, i1, i2, p1, p2, s1, s2};
    vector< Building > vec {a1, a2, b1, b2, u1, u2, f1, f2, s1, s2};
    return vec;
  }

  vector< Constraint > makeTerranConstraints( const vector< Building > &vec, const WallinGrid &grid )
  {
    WallinConstraint overlap( vec, grid );
    WallinConstraint buildable( vec, grid );
    WallinConstraint noGaps( vec, grid );
    WallinConstraint specialTiles( vec, grid );

    // overlap	 = make_shared<Overlap>( vec, grid );
    // buildable	 = make_shared<Buildable>( vec, grid );
    // noGaps	 = make_shared<NoGaps>( vec, grid );
    // specialTiles = make_shared<StartingTargetTiles>( vec, grid );
    
    vector< Constraint > vecConstraints {overlap, buildable, noGaps, specialTiles};
    return vecConstraints;
  }
}
