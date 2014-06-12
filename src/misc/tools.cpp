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


#include "../include/misc/tools.hpp"

namespace ghost
{
  int countSelectedVariables( const std::vector< Variable > &vec )
  {
    return std::count_if( vec.begin(), vec.end(), []( const Variable &b ){ return b->isSelected(); });
  }

  void printConstraints( const std::vector< std::shared_ptr<Constraint> > &vecConstraints )
  {
    std::for_each( vecConstraints.begin(), vecConstraints.end(), []( const std::shared_ptr<Constraint> &c ){ std::cout << *c << std::endl; });
  }

  void addAllInGrid( const std::vector< Variable > &vec, Domain &domain )
  {
    std::for_each( vec.begin(), vec.end(), [&]( const Variable &b ){ domain.add(b); });
  }

  void clearAllInGrid( const std::vector< Variable > &vec, Domain &domain )
  {
    std::for_each( vec.begin(), vec.end(), [&]( const Variable &b ){ domain.clear(b); });
  }
}
