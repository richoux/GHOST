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


#include "../include/tools.hpp"

namespace wallin
{
  void updateConstraints( const std::vector< std::shared_ptr<Constraint> > &vecConstraints, const Grid &grid )
  {
    std::for_each( vecConstraints.begin(), vecConstraints.end(), [&]( const std::shared_ptr<Constraint> &c ){ c->update( grid ); });
  }

  void printConstraints( const std::vector< std::shared_ptr<Constraint> > &vecConstraints )
  {
    std::for_each( vecConstraints.begin(), vecConstraints.end(), []( const std::shared_ptr<Constraint> &c ){ std::cout << *c << std::endl; });
  }

  void addAllInGrid( const std::vector<std::shared_ptr<Building> > &vec, Grid &grid )
  {
    std::for_each( vec.begin(), vec.end(), [&]( const std::shared_ptr<Building> &b ){ grid.add(*b); });
  }

  void clearAllInGrid( const std::vector<std::shared_ptr<Building> > &vec, Grid &grid )
  {
    std::for_each( vec.begin(), vec.end(), [&]( const std::shared_ptr<Building> &b ){ grid.clear(*b); });
  }

  int countBuildings( const std::vector< std::shared_ptr<Building> > &vec )
  {
    return std::count_if( vec.begin(), vec.end(), []( const std::shared_ptr<Building> &b ){ return b->isOnGrid(); });
  }
}
