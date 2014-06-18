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
  // Caution: irritating to eyes.
  Building factoryTerranBuilding( const string &name, int pos = -1 )
  {
    if( name.compare("a") == 0 || name.compare("A") == 0 || name.compare("Academy") == 0 )
      return Building(3, 2, 0, 3, 7, 8, Terran, 2, "A", "Terran_Academy", pos);
    else
    {
      if( name.compare("r") == 0 || name.compare("R") == 0 || name.compare("Armory") == 0 )
	return Building(3, 2, 0, 0, 9, 0, Terran, 3, "R", "Terran_Armory", pos);
      else
      {
	if( name.compare("b") == 0 || name.compare("B") == 0 || name.compare("Barracks") == 0 )
	  return Building(4, 3, 8, 7, 15, 16, Terran, 1, "B", "Terran_Barracks", pos);
	else
	{
	  if( name.compare("u") == 0 || name.compare("U") == 0 || name.compare("Bunker") == 0 )
	    return Building(3, 2, 8, 15, 15, 16, Terran, 2, "U", "Terran_Bunker", pos);
	  else
	  {
	    if( name.compare("c") == 0 || name.compare("C") == 0 || name.compare("Command Center") == 0 )
	      return Building(4, 3, 7, 5, 6, 6, Terran, 0, "C", "Terran_Command_Center", pos);
	    else
	    {
	      if( name.compare("e") == 0 || name.compare("E") == 0 || name.compare("Engineering Bay") == 0 )
		return Building(4, 3, 16, 15, 19, 16, Terran, 1, "E", "Terran_Engineering_Bay", pos);
	      else
	      {
		if( name.compare("f") == 0 || name.compare("F") == 0 || name.compare("Factory") == 0 )
		  return Building(4, 3, 8, 7, 7, 8, Terran, 2, "F", "Terran_Factory", pos);
		else
		{
		  if( name.compare("t") == 0 || name.compare("T") == 0 || name.compare("Missile Turret") == 0 )
		    return Building(2, 2, 0, 15, 15, 16, Terran, 2, "T", "Terran_Missile_Turret", pos);
		  else
		  {
		    if( name.compare("i") == 0 || name.compare("I") == 0 || name.compare("Science Facility") == 0 )
		      return Building(4, 3, 10, 15, 9, 16, Terran, 4, "I", "Terran_Science_Facility", pos);
		    else
		    {
		      if( name.compare("p") == 0 || name.compare("P") == 0 || name.compare("Starport") == 0 )
			return Building(4, 3, 8, 15, 9, 16, Terran, 3, "P", "Terran_Starport", pos);
		      else
		      {
			if( name.compare("s") == 0 || name.compare("S") == 0 || name.compare("Supply Depot") == 0 )
			  return Building(3, 2, 10, 9, 5, 10, Terran, 0, "S", "Terran_Supply_Depot", pos);
			else
			  return Building();
		      }
		    }
		  }
		}
	      }
	    }
	  }
	}
      }
    }
  }
}
