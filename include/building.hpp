/*
 * GHOST (General meta-Heuristic Optimization Solving Tool) is a C++ library 
 * designed for StarCraft: Brood war. 
 * GHOST is a meta-heuristic solver aiming to solve any kind of combinatorial 
 * and optimization RTS-related problems represented by a CSP. It is an extension
 * of a previous project Wall-in.
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
 * along with Wall-in. If not, see http://www.gnu.org/licenses/.
 */


#pragma once

#include <algorithm>
#include <iostream>
#include <typeinfo>

using namespace std;

namespace wallin
{
  enum Race{ Terran, Protoss, Zerg, Unknown };
  
  class Building
  {
  public:
    Building(int, int, int, int, int, int, string, Race, int, int = -1);
    Building(const Building&) = default;
    Building(Building&&) = default;
    Building& operator=(const Building&) = default;
    Building& operator=(Building&&) = default;

    virtual ~Building() = 0;

    inline void setPos(int pos)		{ position = pos; }
    inline void shiftPos()		{ ++position; }
    inline int  getPosition()	const	{ return position; }
    inline bool isOnGrid()	const	{ return position != -1; }

    inline int    getId()	const	{ return id; }
    inline string getShort()	const	{ return shortname; }
    inline string getRace()	const	
      { 
	switch( race ) 
	{
	case Terran: return "Terran";
	case Protoss: return "Protoss";
	case Zerg: return "Zerg";
	default: return "Unknown";
	}
      }
    inline int getTreedepth()	const	{ return treedepth; }

    inline int getLength()	const	{ return length; }
    inline int getHeight()	const	{ return height; }
    inline int getSurface()	const	{ return height * length; }

    inline int getGapTop()	const	{ return gapTop; }
    inline int getGapRight()	const	{ return gapRight; }
    inline int getGapBottom()	const	{ return gapBottom; }
    inline int getGapLeft()	const	{ return gapLeft; }

    inline void swapPosition(Building &other)	{ std::swap(this->position, other.position); }

    inline bool operator<( const Building& other ) const { return id < other.id; }

    friend ostream& operator<<( ostream&, const Building& );

  protected:
    int length;
    int height;

    int gapTop;
    int gapRight;
    int gapBottom;
    int gapLeft;

    string shortname;
    int id;
    Race race;
    int treedepth;

    int position;
  private:
    static int nberBuildings;
  };

  class Academy : public Building
  {
  public:
    Academy() : Building(3, 2, 0, 3, 7, 8, "A", Terran, 2) { }
    Academy(int pos) : Building(3, 2, 0, 3, 7, 8, "A", Terran, 2, pos) { }
  };

  class Armory : public Building
  {
  public:
    Armory() : Building(3, 2, 0, 0, 9, 0, "R", Terran, 3) { }
    Armory(int pos) : Building(3, 2, 0, 0, 9, 0, "R", Terran, 3, pos) { }
  };

  class Barracks : public Building
  {
  public:
    Barracks() : Building(4, 3, 8, 7, 15, 16, "B", Terran, 1) { }
    Barracks(int pos) : Building(4, 3, 8, 7, 15, 16, "B", Terran, 1, pos) { }
  };

  class Bunker : public Building
  {
  public:
    Bunker() : Building(3, 2, 8, 15, 15, 16, "U", Terran, 2) { }
    Bunker(int pos) : Building(3, 2, 8, 15, 15, 16, "U", Terran, 2, pos) { }
  };

  class CommandCenter : public Building
  {
  public:
    CommandCenter() : Building(4, 3, 7, 5, 6, 6, "C", Terran, 0) { }
    CommandCenter(int pos) : Building(4, 3, 7, 5, 6, 6, "C", Terran, 0, pos) { }
  };

  class EngineeringBay : public Building
  {
  public:
    EngineeringBay() : Building(4, 3, 16, 15, 19, 16, "E", Terran, 1) { }
    EngineeringBay(int pos) : Building(4, 3, 16, 15, 19, 16, "E", Terran, 1, pos) { }
  };

  class Factory : public Building
  {
  public:
    Factory() : Building(4, 3, 8, 7, 7, 8, "F", Terran, 2) { }
    Factory(int pos) : Building(4, 3, 8, 7, 7, 8, "F", Terran, 2, pos) { }
  };

  class MissileTurret : public Building
  {
  public:
    MissileTurret() : Building(2, 2, 0, 15, 15, 16, "T", Terran, 2) { }
    MissileTurret(int pos) : Building(2, 2, 0, 15, 15, 16, "T", Terran, 2, pos) { }
  };

  class ScienceFacility : public Building
  {
  public:
    ScienceFacility() : Building(4, 3, 10, 15, 9, 16, "I", Terran, 4) { }
    ScienceFacility(int pos) : Building(4, 3, 10, 15, 9, 16, "I", Terran, 4, pos) { }
  };

  class Starport : public Building
  {
  public:
    Starport() : Building(4, 3, 8, 15, 9, 16, "P", Terran, 3) { }
    Starport(int pos) : Building(4, 3, 8, 15, 9, 16, "P", Terran, 3, pos) { }
  };

  class SupplyDepot : public Building
  {
  public:
    SupplyDepot() : Building(3, 2, 10, 9, 5, 10, "S", Terran, 0) { }
    SupplyDepot(int pos) : Building(3, 2, 10, 9, 5, 10, "S", Terran, 0, pos) { }
  };

} // namespace wallin
