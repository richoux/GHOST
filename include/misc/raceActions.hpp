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

#include <map>

#include "../variables/action.hpp"

namespace ghost
{
  struct RaceActions
  {
    // RaceActions()
    //   : protoss
    //     {
    //       make_pair( "Protoss_Zealot", Action( 40, 100, 0, 2, unit, {"Protoss_Gateway"}, "Protoss_Gateway", Protoss, "puz", "Protoss_Zealot" ) ),
    //       make_pair( "Protoss_Dragoon", Action( 50, 125, 50, 2, unit, {"Protoss_Gateway", "Protoss_Cybernetics_Core", "Protoss_Assimilator"}, "Protoss_Gateway", Protoss, "pud", "Protoss_Dragoon" ) ),
    // 	  make_pair( "Protoss_Gateway", Action( 60, 150, 0, 0, building, {"Protoss_Nexus"}, "Protoss_Probe", Protoss, "pbg", "Protoss_Gateway" ) ),
    // 	  make_pair( "Protoss_Cybernetics_Core", Action( 60, 200, 0, 0, building, {"Protoss_Gateway"}, "Protoss_Probe", Protoss, "pbc", "Protoss_Cybernetics_Core" ) ),
    // 	  make_pair( "Protoss_Forge", Action( 40, 150, 0, 0, building, {"Protoss_Nexus"}, "Protoss_Probe", Protoss, "pbf", "Protoss_Forge" ) ),
    // 	  make_pair( "Protoss_Ground_Weapons_1", Action( 266, 100, 100, 0, upgrade, {"Protoss_Forge", "Protoss_Assimilator"}, "Protoss_Forge", Protoss, "ppgw1", "Protoss_Ground_Weapons_1" ) ),
    // 	  make_pair( "Protoss_Pylon", Action( 30, 100, 0, 0, building, {}, "Protoss_Probe", Protoss, "pby", "Protoss_Pylon" ) ),
    // 	  make_pair( "Protoss_Assimilator", Action( 40, 100, 0, 0, building, {}, "Protoss_Probe", Protoss, "pba", "Protoss_Assimilator" ) )
    //     }      
    // { }
    RaceActions();

    map<string, Action> protoss;
    map<string, Action> terran;
    map<string, Action> zerg;
  };
}
