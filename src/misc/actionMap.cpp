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
      make_pair( "Protoss_Dragoon", ActionData( 50, 125, 50, 2, unit, {"Protoss_Cybernetics_Core"}, "Protoss_Gateway", Protoss, "Protoss_Dragoon" ) ),
      make_pair( "Protoss_High_Templar", ActionData( 50, 50, 150, 2, unit, {"Protoss_Templar_Archives"}, "Protoss_Gateway", Protoss, "Protoss_High_Templar" ) ),
      make_pair( "Protoss_Dark_Templar", ActionData( 50, 125, 100, 2, unit, {"Protoss_Templar_Archives"}, "Protoss_Gateway", Protoss, "Protoss_Dark_Templar" ) ),
      make_pair( "Protoss_Reaver", ActionData( 70, 200, 100, 4, unit, {"Protoss_Robotics_Support_Bay"}, "Protoss_Robotics_Facility", Protoss, "Protoss_Reaver" ) ),
      make_pair( "Protoss_Archon", ActionData( 20, 0, 0, 0, unit, {"Protoss_High_Templar"}, "Protoss_High_Templar", Protoss, "Protoss_Archon" ) ),
      make_pair( "Protoss_Dark_Archon", ActionData( 20, 0, 0, 0, unit, {"Protoss_Dark_Templar"}, "Protoss_Dark_Templar", Protoss, "Protoss_Dark_Archon" ) ),
      make_pair( "Protoss_Observer", ActionData( 40, 25, 75, 1, unit, {"Protoss_Observatory"}, "Protoss_Robotics_Facility", Protoss, "Protoss_Observer" ) ),
      make_pair( "Protoss_Shuttle", ActionData( 60, 200, 0, 2, unit, {"Protoss_Robotics_Facility"}, "Protoss_Robotics_Facility", Protoss, "Protoss_Shuttle" ) ),
      make_pair( "Protoss_Scout", ActionData( 80, 275, 125, 3, unit, {"Protoss_Stargate"}, "Protoss_Stargate", Protoss, "Protoss_Scout" ) ),
      make_pair( "Protoss_Carrier", ActionData( 140, 350, 250, 6, unit, {"Protoss_Fleet_Beacon"}, "Protoss_Stargate", Protoss, "Protoss_Carrier" ) ),
      make_pair( "Protoss_Arbiter", ActionData( 160, 100, 350, 4, unit, {"Protoss_Arbiter_Tribunal"}, "Protoss_Stargate", Protoss, "Protoss_Arbiter" ) ),
      make_pair( "Protoss_Corsair", ActionData( 40, 150, 350, 2, unit, {"Protoss_Stargate"}, "Protoss_Stargate", Protoss, "Protoss_Corsair" ) ),

      
      make_pair( "Protoss_Nexus", ActionData( 120, 400, 0, 0, building, {}, "Protoss_Probe", Protoss, "Protoss_Nexus" ) ),
      make_pair( "Protoss_Pylon", ActionData( 30, 100, 0, 0, building, {}, "Protoss_Probe", Protoss, "Protoss_Pylon" ) ),
      make_pair( "Protoss_Assimilator", ActionData( 40, 100, 0, 0, building, {}, "Protoss_Probe", Protoss, "Protoss_Assimilator" ) ),
      make_pair( "Protoss_Gateway", ActionData( 60, 150, 0, 0, building, {"Protoss_Nexus"}, "Protoss_Probe", Protoss, "Protoss_Gateway" ) ),
      make_pair( "Protoss_Forge", ActionData( 40, 150, 0, 0, building, {"Protoss_Nexus"}, "Protoss_Probe", Protoss, "Protoss_Forge" ) ),
      make_pair( "Protoss_Cybernetics_Core", ActionData( 60, 200, 0, 0, building, {"Protoss_Gateway"}, "Protoss_Probe", Protoss, "Protoss_Cybernetics_Core" ) ),
      make_pair( "Protoss_Shield_Battery", ActionData( 30, 100, 0, 0, building, {"Protoss_Gateway"}, "Protoss_Probe", Protoss, "Protoss_Shield_Battery" ) ),

      
      make_pair( "Protoss_Ground_Weapons_1", ActionData( 266, 100, 100, 0, upgrade, {"Protoss_Forge"}, "Protoss_Forge", Protoss, "Protoss_Ground_Weapons_1" ) )
   };
}
