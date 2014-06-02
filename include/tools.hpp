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


#pragma once

#include <algorithm>
// #include <set>
#include <vector>
#include <memory>

#include "building.hpp"
#include "constraint.hpp"
#include "grid.hpp"

namespace wallin
{
  void updateConstraints( const std::vector< std::shared_ptr<Constraint> >&, const Grid& );
  void printConstraints( const std::vector< std::shared_ptr<Constraint> >& );
  void addAllInGrid( const std::vector< std::shared_ptr<Building> >&, Grid& );
  void clearAllInGrid( const std::vector< std::shared_ptr<Building> >&, Grid& );
  int  countBuildings( const std::vector< std::shared_ptr<Building> >& );
}
