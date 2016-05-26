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


#include <typeinfo>

#include "../../include/variables/building.hpp"

namespace ghost
{
	Building::Building()
		: Variable("N", "None"),
		length(0),
		height(0),
		gapTop(0),
		gapRight(0),
		gapBottom(0),
		gapLeft(0),
		race(Race::Unknown),
		treedepth(0)
	{ }
  
  Building::Building(int x, int y, int top, int right, int bottom, int left, Race race, int treedepth,
	  string name, string fullName, int position)
	  : Variable(name, fullName, position),
	  length(x),
	  height(y),
	  gapTop(top),
	  gapRight(right),
	  gapBottom(bottom),
	  gapLeft(left),
	  race(race),
	  treedepth(treedepth)
  { }

  Building::Building( const Building &other )
    : Variable(other),
      length(other.length),
      height(other.height),
      gapTop(other.gapTop), 
      gapRight(other.gapRight), 
      gapBottom(other.gapBottom), 
      gapLeft(other.gapLeft),
      race(other.race),
      treedepth(other.treedepth)
  { }

  Building& Building::operator=( Building other )
  {
    this->swap( other );
    return *this;
  }

  void Building::swap( Building &other )
  {
    std::swap(this->name, other.name);
    std::swap(this->fullName, other.fullName);
    std::swap(this->id, other.id);
    std::swap(this->value, other.value);
    std::swap(this->length, other.length);
    std::swap(this->height, other.height);
    std::swap(this->gapTop, other.gapTop);
    std::swap(this->gapRight, other.gapRight);
    std::swap(this->gapBottom, other.gapBottom);
    std::swap(this->gapLeft, other.gapLeft);
    std::swap(this->race, other.race);
    std::swap(this->treedepth, other.treedepth);
  }

  ostream& operator<<( ostream& os, const Building& b )
  {
    return os
      << "Type: " <<  typeid(b).name() << endl
      << "Race: " <<  b.getRaceString() << endl
      << "Name: " << b.name << endl
      << "Full name: " << b.fullName << endl
      << "Id num: " << b.id << endl
      << "Tree depth: " << b.treedepth << endl
      << "Position: " <<  b.value << endl
      << "Length: " <<  b.length << endl
      << "Height: " <<  b.height << endl
      << "Gap Top: " <<  b.gapTop << endl
      << "Gap Right: " <<  b.gapRight << endl
      << "Gap Bottom: " <<  b.gapBottom << endl
      << "Gap Left: " <<  b.gapLeft << endl
      << "-------" << endl;
  }
}
