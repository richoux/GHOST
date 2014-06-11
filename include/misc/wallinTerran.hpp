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
  vector< Building > makeTerranBuildings()
  {
    // Academy
    Building a1 = factoryTerranBuilding("Academy");
    Building a2 = factoryTerranBuilding("Academy");

    // Armory
    Building r1 = factoryTerranBuilding("Armory");
    Building r2 = factoryTerranBuilding("Armory");

    // Barracks
    Building b1 = factoryTerranBuilding("Barracks");
    Building b2 = factoryTerranBuilding("Barracks");

    // Bunker
    Building u1 = factoryTerranBuilding("Bunker");
    Building u2 = factoryTerranBuilding("Bunker");

    // Command Center
    Building c1 = factoryTerranBuilding("Command Center");
    Building c2 = factoryTerranBuilding("Command Center");

    // Engineering Bay
    Building e1 = factoryTerranBuilding("Engineering Bay");
    Building e2 = factoryTerranBuilding("Engineering Bay");

    // Factory
    Building f1 = factoryTerranBuilding("Factory");
    Building f2 = factoryTerranBuilding("Factory");

    // Missile Turret
    Building t1 = factoryTerranBuilding("Missile Turret");
    Building t2 = factoryTerranBuilding("Missile Turret");
  
    // Science Facility
    Building i1 = factoryTerranBuilding("Science Facility");
    Building i2 = factoryTerranBuilding("Science Facility");

    // Starport
    Building p1 = factoryTerranBuilding("Starport");
    Building p2 = factoryTerranBuilding("Starport");

    // Supply Depot
    Building s1 = factoryTerranBuilding("Supply Depot");
    Building s2 = factoryTerranBuilding("Supply Depot");

    // vector< Building > vec {a1, a2, r1, r2, b1, b2, u1, u2, c1, c2, e1, e2, f1, f2, t1, t2, i1, i2, p1, p2, s1, s2};
    vector< Building > vec {a1, a2, b1, b2, u1, u2, f1, f2, s1, s2};
    return vec;
  }

  vector< shared_ptr<WallinConstraint> > makeTerranConstraints( const vector< Building > &vec, const WallinGrid &grid )
  {
    shared_ptr<WallinConstraint> overlap	= make_shared<Overlap>( vec, grid );
    shared_ptr<WallinConstraint> buildable	= make_shared<Buildable>( vec, grid );
    shared_ptr<WallinConstraint> noGaps		= make_shared<NoGaps>( vec, grid );
    shared_ptr<WallinConstraint> specialTiles	= make_shared<StartingTargetTiles>( vec, grid );

    vector< shared_ptr<WallinConstraint> > vecConstraints {overlap, buildable, noGaps, specialTiles};
    return vecConstraints;
  }
}
