/*
 * Wall-in is a C++ library designed for StarCraft: Brood,
 * making a wall optimizised for a given objective: minimize the
 * number of buildings, the technology needed, the number of gaps
 * between building big enough to let enter small units, etc.
 * To do so, it use some Constraint Programming techniques 
 * like meta-heuristics.
 * Please visit https://github.com/richoux/Wall-in 
 * for further information.
 * 
 * Copyright (C) 2014 Florian Richoux
 *
 * This file is part of Wall-in.
 * Wall-in is free software: you can redistribute it and/or 
 * modify it under the terms of the GNU General Public License as published 
 * by the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * Wall-in is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with Wall-in. If not, see http://www.gnu.org/licenses/.
 */


#include "../include/building.hpp"

namespace wallin
{
  int Building::nberBuildings = 0;

  Building::Building(int x, 
		     int y, 
		     int top, 
		     int right, 
		     int bottom, 
		     int left, 
		     std::string shortname, 
		     Race race,
		     int treedepth,
		     int position)
    : length(x),
      height(y),
      gapTop(top), 
      gapRight(right), 
      gapBottom(bottom), 
      gapLeft(left),
      shortname(shortname),
      id(Building::nberBuildings++),
      race(race),
      treedepth(treedepth),
      position(position)
  { }

  Building::~Building() {}

  std::ostream& operator<<( std::ostream& os, const Building& b )
  {
    return os
      << "Type: " <<  typeid(b).name() << std::endl
      << "Race: " <<  b.getRace() << std::endl
      << "Short name: " << b.shortname << std::endl
      << "Id num: " << b.id << std::endl
      << "Tree depth: " << b.treedepth << std::endl
      << "Position: " <<  b.position << std::endl
      << "Length: " <<  b.length << std::endl
      << "Height: " <<  b.height << std::endl
      << "Gap Top: " <<  b.gapTop << std::endl
      << "Gap Right: " <<  b.gapRight << std::endl
      << "Gap Bottom: " <<  b.gapBottom << std::endl
      << "Gap Left: " <<  b.gapLeft << std::endl
      << "-------" << std::endl;
  }

}
