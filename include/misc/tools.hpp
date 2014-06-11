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

#include <algorithm>
#include <vector>
#include <memory>

#include "../variables/building.hpp"
#include "../constraints/constraint.hpp"
#include "../domains/domain.hpp"
#include "../domains/wallinGrid.hpp"

namespace ghost
{
  template <typename TypeDomain>
  void updateConstraints( const std::vector< std::shared_ptr<Constraint> > &vecConstraints, const TypeDomain &domain )
  {
    std::for_each( vecConstraints.begin(), vecConstraints.end(), [&]( const std::shared_ptr<Constraint> &c ){ c->update( domain ); });
  }
  
  void printConstraints( const std::vector< std::shared_ptr<Constraint> >& );
  void addAllInGrid( const std::vector< std::shared_ptr<Building> >&, shared_ptr<Domain>& );
  void clearAllInGrid( const std::vector< std::shared_ptr<Building> >&, shared_ptr<Domain>& );
}
