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

#include "building.hpp"

using namespace std;

namespace ghost
{
  class Academy : public Building
  {
  public:
    Academy() : Building(3, 2, 0, 3, 7, 8, Terran, 2, "A") { }
    Academy(int pos) : Building(3, 2, 0, 3, 7, 8, Terran, 2, "A", pos) { }
  };

  class Armory : public Building
  {
  public:
    Armory() : Building(3, 2, 0, 0, 9, 0, Terran, 3, "R") { }
    Armory(int pos) : Building(3, 2, 0, 0, 9, 0, Terran, 3, "R", pos) { }
  };

  class Barracks : public Building
  {
  public:
    Barracks() : Building(4, 3, 8, 7, 15, 16, Terran, 1, "B") { }
    Barracks(int pos) : Building(4, 3, 8, 7, 15, 16, Terran, 1, "B", pos) { }
  };

  class Bunker : public Building
  {
  public:
    Bunker() : Building(3, 2, 8, 15, 15, 16, Terran, 2, "U") { }
    Bunker(int pos) : Building(3, 2, 8, 15, 15, 16, Terran, 2, "U", pos) { }
  };

  class CommandCenter : public Building
  {
  public:
    CommandCenter() : Building(4, 3, 7, 5, 6, 6, Terran, 0, "C") { }
    CommandCenter(int pos) : Building(4, 3, 7, 5, 6, 6, Terran, 0, "C", pos) { }
  };

  class EngineeringBay : public Building
  {
  public:
    EngineeringBay() : Building(4, 3, 16, 15, 19, 16, Terran, 1, "E") { }
    EngineeringBay(int pos) : Building(4, 3, 16, 15, 19, 16, Terran, 1, "E", pos) { }
  };

  class Factory : public Building
  {
  public:
    Factory() : Building(4, 3, 8, 7, 7, 8, Terran, 2, "F") { }
    Factory(int pos) : Building(4, 3, 8, 7, 7, 8, Terran, 2, "F", pos) { }
  };

  class MissileTurret : public Building
  {
  public:
    MissileTurret() : Building(2, 2, 0, 15, 15, 16, Terran, 2, "T") { }
    MissileTurret(int pos) : Building(2, 2, 0, 15, 15, 16, Terran, 2, "T", pos) { }
  };

  class ScienceFacility : public Building
  {
  public:
    ScienceFacility() : Building(4, 3, 10, 15, 9, 16, Terran, 4, "I") { }
    ScienceFacility(int pos) : Building(4, 3, 10, 15, 9, 16, Terran, 4, "I", pos) { }
  };

  class Starport : public Building
  {
  public:
    Starport() : Building(4, 3, 8, 15, 9, 16, Terran, 3, "P") { }
    Starport(int pos) : Building(4, 3, 8, 15, 9, 16, Terran, 3, "P", pos) { }
  };

  class SupplyDepot : public Building
  {
  public:
    SupplyDepot() : Building(3, 2, 10, 9, 5, 10, Terran, 0, "S") { }
    SupplyDepot(int pos) : Building(3, 2, 10, 9, 5, 10, Terran, 0, "S", pos) { }
  };
}
