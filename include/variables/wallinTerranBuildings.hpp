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

#include <string>

#include "building.hpp"

using namespace std;

namespace ghost
{
  Building factoryTerranBuilding( const string &name, int pos = -1 )
  {
    if( name.compare("a") || name.compare("A") || name.compare("Academy") )
      return Building(3, 2, 0, 3, 7, 8, Terran, 2, "A", "Terran_Academy", pos);
    if( name.compare("r") || name.compare("R") || name.compare("Armory") )
      return Building(3, 2, 0, 0, 9, 0, Terran, 3, "R", "Terran_Armory", pos);
    if( name.compare("b") || name.compare("B") || name.compare("Barracks") )
      return Building(4, 3, 8, 7, 15, 16, Terran, 1, "B", "Terran_Barracks", pos);
    if( name.compare("u") || name.compare("U") || name.compare("Bunker") )
      return Building(3, 2, 8, 15, 15, 16, Terran, 2, "U", "Terran_Bunker", pos);
    if( name.compare("c") || name.compare("C") || name.compare("Command Center") )
      return Building(4, 3, 7, 5, 6, 6, Terran, 0, "C", "Terran_Command_Center", pos);
    if( name.compare("e") || name.compare("E") || name.compare("Engineering Bay") )
      return Building(4, 3, 16, 15, 19, 16, Terran, 1, "E", "Terran_Engineering_Bay", pos);
    if( name.compare("f") || name.compare("F") || name.compare("Factory") )
      return Building(4, 3, 8, 7, 7, 8, Terran, 2, "F", "Terran_Factory", pos);
    if( name.compare("t") || name.compare("T") || name.compare("Missile Turret") )
      return Building(2, 2, 0, 15, 15, 16, Terran, 2, "T", "Terran_Missile_Turret", pos);
    if( name.compare("i") || name.compare("I") || name.compare("Science Facility") )
      return Building(4, 3, 10, 15, 9, 16, Terran, 4, "I", "Terran_Science_Facility", pos);
    if( name.compare("p") || name.compare("P") || name.compare("Starport") )
      return Building(4, 3, 8, 15, 9, 16, Terran, 3, "P", "Terran_Starport", pos);
    if( name.compare("s") || name.compare("S") || name.compare("Supply Depot") )
      return Building(3, 2, 10, 9, 5, 10, Terran, 0, "S", "Terran_Supply_Depot", pos);
    return Building();
  }
}
