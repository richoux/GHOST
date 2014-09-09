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


#include "../../include/misc/unitMap.hpp"

namespace ghost
{
  map<string, const UnitData> unitOf
  {
    make_pair( "Terran_Marine",
  	       UnitData( "Terran_Marine", 40, 0, Small, 0, 15, 6, Normal, {0, 128}, {0,0,0} ) ),
    make_pair( "Terran_Firebat",
  	       UnitData( "Terran_Firebat", 50, 1, Small, 0, 22, 16, Concussive, {0, 32}, {15,20,25}, true ) ),
    make_pair( "Terran_Ghost",
  	       UnitData( "Terran_Ghost", 45, 0, Small, 0, 22, 10, Concussive, {0, 244}, {0,0,0} ) ),
    make_pair( "Terran_Vulture",
  	       UnitData( "Terran_Vulture", 80, 0, Medium, 0, 30, 20, Concussive, {0, 160}, {0,0,0} ) ),
    make_pair( "Terran_Goliath",
  	       UnitData( "Terran_Goliath", 125, 1, Large, 0, 22, 12, Normal, {0, 192}, {0,0,0} ) ),
    make_pair( "Terran_Siege_Tank_Tank_Mode",
  	       UnitData( "Terran_Siege_Tank_Tank_Mode", 150, 1, Large, 0, 37, 30, Explosive, {0, 224}, {0,0,0} ) ),
    make_pair( "Terran_Siege_Tank_Siege_Mode",
  	       UnitData( "Terran_Siege_Tank_Siege_Mode", 150, 1, Large, 0, 75, 70, Explosive, {64, 384}, {10,25,40}, true ) )
   };

}
