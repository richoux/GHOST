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


#include "../../include/misc/actionMap.hpp"

namespace ghost
{
  map<string, ActionData> actionOf    {  
      make_pair( "Protoss_Mineral", ActionData( 0, 0, 0, 0, special, {"Protoss_Nexus"}, "Protoss_Probe", Protoss, "Mineral" ) ),
      make_pair( "Protoss_Gas", ActionData( 0, 0, 0, 0, special, {"Protoss_Nexus", "Protoss_Assimilator"}, "Protoss_Probe", Protoss, "Gas" ) ),
      make_pair( "Protoss_Probe", ActionData( 20, 50, 0, 1, unit, {"Protoss_Nexus"}, "Protoss_Nexus", Protoss, "Protoss_Probe" ) ),
      make_pair( "Protoss_Zealot", ActionData( 40, 100, 0, 2, unit, {"Protoss_Gateway"}, "Protoss_Gateway", Protoss, "Protoss_Zealot" ) ),
      make_pair( "Protoss_Dragoon", ActionData( 50, 125, 50, 2, unit, {"Protoss_Gateway", "Protoss_Cybernetics_Core", "Protoss_Assimilator"}, "Protoss_Gateway", Protoss, "Protoss_Dragoon" ) ),
      make_pair( "Protoss_Gateway", ActionData( 60, 150, 0, 0, building, {"Protoss_Nexus"}, "Protoss_Probe", Protoss, "Protoss_Gateway" ) ),
      make_pair( "Protoss_Cybernetics_Core", ActionData( 60, 200, 0, 0, building, {"Protoss_Gateway"}, "Protoss_Probe", Protoss, "Protoss_Cybernetics_Core" ) ),
      make_pair( "Protoss_Forge", ActionData( 40, 150, 0, 0, building, {"Protoss_Nexus"}, "Protoss_Probe", Protoss, "Protoss_Forge" ) ),
      make_pair( "Protoss_Ground_Weapons_1", ActionData( 266, 100, 100, 0, upgrade, {"Protoss_Forge", "Protoss_Assimilator"}, "Protoss_Forge", Protoss, "Protoss_Ground_Weapons_1" ) ),
      make_pair( "Protoss_Pylon", ActionData( 30, 100, 0, 0, building, {}, "Protoss_Probe", Protoss, "Protoss_Pylon" ) ),
      make_pair( "Protoss_Assimilator", ActionData( 40, 100, 0, 0, building, {}, "Protoss_Probe", Protoss, "Protoss_Assimilator" ) )
   };
}
