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


#include "actionMap.hpp"

namespace ghost
{
  map<string, const ActionData> actionOf
  {
    // special
    make_pair( "Protoss_Mineral",
  	       ActionData( 0, 0, 0, 0, special, {"Protoss_Nexus"}, "Protoss_Probe", Protoss, "Mineral" ) ),
      make_pair( "Protoss_Gas",
  		 ActionData( 0, 0, 0, 0, special, {"Protoss_Nexus", "Protoss_Assimilator"}, "Protoss_Probe", Protoss, "Gas" ) ),
      // units
      make_pair( "Protoss_Probe",
  		 ActionData( 20, 50, 0, 1, unit, {"Protoss_Nexus"}, "Protoss_Nexus", Protoss, "Protoss_Probe" ) ),
      make_pair( "Protoss_Zealot",
  		 ActionData( 40, 100, 0, 2, unit, {"Protoss_Gateway"}, "Protoss_Gateway", Protoss, "Protoss_Zealot" ) ),
      make_pair( "Protoss_Dragoon",
  		 ActionData( 50, 125, 50, 2, unit, {"Protoss_Cybernetics_Core"}, "Protoss_Gateway", Protoss, "Protoss_Dragoon" ) ),
      make_pair( "Protoss_High_Templar",
  		 ActionData( 50, 50, 150, 2, unit, {"Protoss_Templar_Archives"}, "Protoss_Gateway", Protoss, "Protoss_High_Templar" ) ),
      make_pair( "Protoss_Dark_Templar",
  		 ActionData( 50, 125, 100, 2, unit, {"Protoss_Templar_Archives"}, "Protoss_Gateway", Protoss, "Protoss_Dark_Templar" ) ),
      make_pair( "Protoss_Reaver",
  		 ActionData( 70, 200, 100, 4, unit, {"Protoss_Robotics_Support_Bay"}, "Protoss_Robotics_Facility", Protoss, "Protoss_Reaver" ) ),
      make_pair( "Protoss_Archon",
  		 ActionData( 20, 0, 0, 0, unit, {"Protoss_High_Templar"}, "Protoss_High_Templar", Protoss, "Protoss_Archon" ) ),
      make_pair( "Protoss_Dark_Archon",
  		 ActionData( 20, 0, 0, 0, unit, {"Protoss_Dark_Templar"}, "Protoss_Dark_Templar", Protoss, "Protoss_Dark_Archon" ) ),
      make_pair( "Protoss_Observer",
  		 ActionData( 40, 25, 75, 1, unit, {"Protoss_Observatory"}, "Protoss_Robotics_Facility", Protoss, "Protoss_Observer" ) ),
      make_pair( "Protoss_Shuttle",
  		 ActionData( 60, 200, 0, 2, unit, {"Protoss_Robotics_Facility"}, "Protoss_Robotics_Facility", Protoss, "Protoss_Shuttle" ) ),
      make_pair( "Protoss_Scout",
  		 ActionData( 80, 275, 125, 3, unit, {"Protoss_Stargate"}, "Protoss_Stargate", Protoss, "Protoss_Scout" ) ),
      make_pair( "Protoss_Carrier",
  		 ActionData( 140, 350, 250, 6, unit, {"Protoss_Fleet_Beacon"}, "Protoss_Stargate", Protoss, "Protoss_Carrier" ) ),
      make_pair( "Protoss_Arbiter",
  		 ActionData( 160, 100, 350, 4, unit, {"Protoss_Arbiter_Tribunal"}, "Protoss_Stargate", Protoss, "Protoss_Arbiter" ) ),
      make_pair( "Protoss_Corsair",
  		 ActionData( 40, 150, 350, 2, unit, {"Protoss_Stargate"}, "Protoss_Stargate", Protoss, "Protoss_Corsair" ) ),
      // buildings
      make_pair( "Protoss_Nexus",
  		 ActionData( 120, 400, 0, 0, building, {}, "Protoss_Probe", Protoss, "Protoss_Nexus" ) ),
      make_pair( "Protoss_Pylon",
  		 ActionData( 30, 100, 0, 0, building, {}, "Protoss_Probe", Protoss, "Protoss_Pylon" ) ),
      make_pair( "Protoss_Assimilator",
  		 ActionData( 40, 100, 0, 0, building, {}, "Protoss_Probe", Protoss, "Protoss_Assimilator" ) ),
      make_pair( "Protoss_Gateway",
  		 ActionData( 60, 150, 0, 0, building, {"Protoss_Nexus"}, "Protoss_Probe", Protoss, "Protoss_Gateway" ) ),
      make_pair( "Protoss_Forge",
  		 ActionData( 40, 150, 0, 0, building, {"Protoss_Nexus"}, "Protoss_Probe", Protoss, "Protoss_Forge" ) ),
      make_pair( "Protoss_Shield_Battery",
  		 ActionData( 30, 100, 0, 0, building, {"Protoss_Gateway"}, "Protoss_Probe", Protoss, "Protoss_Shield_Battery" ) ),
      make_pair( "Protoss_Cybernetics_Core",
  		 ActionData( 60, 200, 0, 0, building, {"Protoss_Gateway"}, "Protoss_Probe", Protoss, "Protoss_Cybernetics_Core" ) ),
      make_pair( "Protoss_Photon_Cannon",
  		 ActionData( 50, 150, 0, 0, building, {"Protoss_Forge"}, "Protoss_Probe", Protoss, "Protoss_Photon_Cannon" ) ),
      make_pair( "Protoss_Robotics_Facility",
  		 ActionData( 80, 200, 200, 0, building, {"Protoss_Cybernetics_Core"}, "Protoss_Probe", Protoss, "Protoss_Robotics_Facility" ) ),
      make_pair( "Protoss_Stargate",
  		 ActionData( 70, 150, 150, 0, building, {"Protoss_Cybernetics_Core"}, "Protoss_Probe", Protoss, "Protoss_Stargate" ) ),
      make_pair( "Protoss_Citadel_of_Adun",
  		 ActionData( 60, 150, 100, 0, building, {"Protoss_Cybernetics_Core"}, "Protoss_Probe", Protoss, "Protoss_Citadel_of_Adun" ) ),
      make_pair( "Protoss_Robotics_Support_Bay",
  		 ActionData( 30, 150, 100, 0, building, {"Protoss_Robotics_Facility"}, "Protoss_Probe", Protoss, "Protoss_Robotics_Support_Bay" ) ),
      make_pair( "Protoss_Fleet_Beacon",
  		 ActionData( 60, 300, 200, 0, building, {"Protoss_Stargate"}, "Protoss_Probe", Protoss, "Protoss_Fleet_Beacon" ) ),
      make_pair( "Protoss_Templar_Archives",
  		 ActionData( 60, 150, 200, 0, building, {"Protoss_Citadel_of_Adun"}, "Protoss_Probe", Protoss, "Protoss_Templar_Archives" ) ),
      make_pair( "Protoss_Observatory",
  		 ActionData( 30, 50, 100, 0, building, {"Protoss_Robotics_Facility"}, "Protoss_Probe", Protoss, "Protoss_Observatory" ) ),
      make_pair( "Protoss_Arbiter_Tribunal",
  		 ActionData( 60, 200, 150, 0, building, {"Protoss_Stargate", "Protoss_Templar_Archives"}, "Protoss_Probe", Protoss, "Protoss_Arbiter_Tribunal" ) ),
      // upgrades
      make_pair( "Protoss_Ground_Weapons_1",
  		 ActionData( 266, 100, 100, 0, upgrade, {"Protoss_Forge"}, "Protoss_Forge", Protoss, "Protoss_Ground_Weapons_1" ) ),
      make_pair( "Protoss_Ground_Weapons_2",
  		 ActionData( 298, 150, 150, 0, upgrade, {"Protoss_Ground_Weapons_1"}, "Protoss_Forge", Protoss, "Protoss_Ground_Weapons_2" ) ),
      make_pair( "Protoss_Ground_Weapons_3",
  		 ActionData( 330, 200, 200, 0, upgrade, {"Protoss_Ground_Weapons_2"}, "Protoss_Forge", Protoss, "Protoss_Ground_Weapons_3" ) ),
      make_pair( "Protoss_Ground_Armor_1",
  		 ActionData( 266, 100, 100, 0, upgrade, {"Protoss_Forge"}, "Protoss_Forge", Protoss, "Protoss_Ground_Armor_1" ) ),
      make_pair( "Protoss_Ground_Armor_2",
  		 ActionData( 298, 175, 175, 0, upgrade, {"Protoss_Ground_Armor_1"}, "Protoss_Forge", Protoss, "Protoss_Ground_Armor_2" ) ),
      make_pair( "Protoss_Ground_Armor_3",
  		 ActionData( 330, 250, 250, 0, upgrade, {"Protoss_Ground_Armor_2"}, "Protoss_Forge", Protoss, "Protoss_Ground_Armor_3" ) ),
      make_pair( "Protoss_Plasma_Shields_1",
  		 ActionData( 266, 200, 200, 0, upgrade, {"Protoss_Forge"}, "Protoss_Forge", Protoss, "Protoss_Plasma_Shields_1" ) ),
      make_pair( "Protoss_Plasma_Shields_2",
  		 ActionData( 298, 300, 300, 0, upgrade, {"Protoss_Plasma_Shields_1"}, "Protoss_Forge", Protoss, "Protoss_Plasma_Shields_2" ) ),
      make_pair( "Protoss_Plasma_Shields_3",
  		 ActionData( 330, 400, 400, 0, upgrade, {"Protoss_Plasma_Shields_2"}, "Protoss_Forge", Protoss, "Protoss_Plasma_Shields_3" ) ),
      make_pair( "Protoss_Air_Weapons_1",
  		 ActionData( 266, 100, 100, 0, upgrade, {"Protoss_Cybernetics_Core"}, "Protoss_Cybernetics_Core", Protoss, "Protoss_Air_Weapons_1" ) ),
      make_pair( "Protoss_Air_Weapons_2",
  		 ActionData( 298, 175, 175, 0, upgrade, {"Protoss_Air_Weapons_1"}, "Protoss_Cybernetics_Core", Protoss, "Protoss_Air_Weapons_2" ) ),
      make_pair( "Protoss_Air_Weapons_3",
  		 ActionData( 330, 250, 250, 0, upgrade, {"Protoss_Air_Weapons_2"}, "Protoss_Cybernetics_Core", Protoss, "Protoss_Air_Weapons_3" ) ),
      make_pair( "Protoss_Air_Armor_1",
  		 ActionData( 266, 150, 150, 0, upgrade, {"Protoss_Cybernetics_Core"}, "Protoss_Cybernetics_Core", Protoss, "Protoss_Air_Armor_1" ) ),
      make_pair( "Protoss_Air_Armor_2",
  		 ActionData( 298, 225, 225, 0, upgrade, {"Protoss_Air_Armor_1"}, "Protoss_Cybernetics_Core", Protoss, "Protoss_Air_Armor_2" ) ),
      make_pair( "Protoss_Air_Armor_3",
  		 ActionData( 330, 300, 300, 0, upgrade, {"Protoss_Air_Armor_2"}, "Protoss_Cybernetics_Core", Protoss, "Protoss_Air_Armor_3" ) ),
      // researches
      make_pair( "Singularity_Charge",
  		 ActionData( 166, 150, 150, 0, research, {"Protoss_Cybernetics_Core"}, "Protoss_Cybernetics_Core", Protoss, "Singularity_Charge" ) ),
      make_pair( "Leg_Enhancements",
  		 ActionData( 133, 150, 150, 0, research, {"Protoss_Citadel_of_Adun"}, "Protoss_Citadel_of_Adun", Protoss, "Leg_Enhancements" ) ),
      make_pair( "Scarab_Damage",
  		 ActionData( 166, 200, 200, 0, research, {"Protoss_Robotics_Support_Bay"}, "Protoss_Robotics_Support_Bay", Protoss, "Scarab_Damage" ) ),
      make_pair( "Reaver_Capacity",
  		 ActionData( 166, 200, 200, 0, research, {"Protoss_Robotics_Support_Bay"}, "Protoss_Robotics_Support_Bay", Protoss, "Reaver_Capacity" ) ),
      make_pair( "Gravitic_Drive",
  		 ActionData( 166, 200, 200, 0, research, {"Protoss_Robotics_Support_Bay"}, "Protoss_Robotics_Support_Bay", Protoss, "Gravitic_Drive" ) ),
      make_pair( "Apial_Sensors",
  		 ActionData( 166, 100, 100, 0, research, {"Protoss_Fleet_Beacon"}, "Protoss_Fleet_Beacon", Protoss, "Apial_Sensors" ) ),
      make_pair( "Gravitic_Thrusters",
  		 ActionData( 166, 200, 200, 0, research, {"Protoss_Fleet_Beacon"}, "Protoss_Fleet_Beacon", Protoss, "Gravitic_Thrusters" ) ),
      make_pair( "Carrier_Capacity",
  		 ActionData( 100, 100, 100, 0, research, {"Protoss_Fleet_Beacon"}, "Protoss_Fleet_Beacon", Protoss, "Carrier_Capacity" ) ),
      make_pair( "Argus_Jewel",
  		 ActionData( 166, 100, 100, 0, research, {"Protoss_Fleet_Beacon"}, "Protoss_Fleet_Beacon", Protoss, "Argus_Jewel" ) ),
      make_pair( "Disruption_Web",
  		 ActionData( 80, 200, 200, 0, research, {"Protoss_Fleet_Beacon"}, "Protoss_Fleet_Beacon", Protoss, "Disruption_Web" ) ),
      make_pair( "Psionic_Storm",
  		 ActionData( 120, 200, 200, 0, research, {"Protoss_Templar_Archives"}, "Protoss_Templar_Archives", Protoss, "Psionic_Storm" ) ),
      make_pair( "Hallucination",
		 ActionData( 80, 150, 150, 0, research, {"Protoss_Templar_Archives"}, "Protoss_Templar_Archives", Protoss, "Hallucination" ) ),
      make_pair( "Khaydarin_Amulet",
		 ActionData( 166, 150, 150, 0, research, {"Protoss_Templar_Archives"}, "Protoss_Templar_Archives", Protoss, "Khaydarin_Amulet" ) ),
      make_pair( "Maelstrom",
		 ActionData( 100, 100, 100, 0, research, {"Protoss_Templar_Archives"}, "Protoss_Templar_Archives", Protoss, "Maelstrom" ) ),
      make_pair( "Mind_Control",
		 ActionData( 120, 200, 200, 0, research, {"Protoss_Templar_Archives"}, "Protoss_Templar_Archives", Protoss, "Mind_Control" ) ),
      make_pair( "Argus_Talisman",
  		 ActionData( 166, 150, 150, 0, research, {"Protoss_Templar_Archives"}, "Protoss_Templar_Archives", Protoss, "Argus_Talisman" ) ),
      make_pair( "Sensor_Array",
  		 ActionData( 133, 150, 150, 0, research, {"Protoss_Observatory"}, "Protoss_Observatory", Protoss, "Sensor_Array" ) ),
      make_pair( "Gravitic_Boosters",
  		 ActionData( 166, 200, 200, 0, research, {"Protoss_Observatory"}, "Protoss_Observatory", Protoss, "Gravitic_Boosters" ) ),
      make_pair( "Recall",
  		 ActionData( 120, 150, 150, 0, research, {"Protoss_Arbiter_Tribunal"}, "Protoss_Arbiter_Tribunal", Protoss, "Recall" ) ),
      make_pair( "Stasis_Field",
  		 ActionData( 100, 150, 150, 0, research, {"Protoss_Arbiter_Tribunal"}, "Protoss_Arbiter_Tribunal", Protoss, "Stasis_Field" ) ),
      make_pair( "Khaydarin_Core",
  		 ActionData( 166, 150, 150, 0, research, {"Protoss_Arbiter_Tribunal"}, "Protoss_Arbiter_Tribunal", Protoss, "Khaydarin_Core" ) )
   };

}
