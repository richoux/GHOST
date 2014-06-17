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

#include "../variables/variable.hpp"
#include "../constraints/constraint.hpp"
#include "../domains/domain.hpp"

namespace ghost
{
  // template <typename TypeVariable, typename TypeDomain>
  // void updateConstraints( const std::vector< std::shared_ptr<Constraint<TypeVariable, TypeDomain> > > &vecConstraints, const TypeDomain &domain )
  // {
  //   std::for_each( vecConstraints.begin(), vecConstraints.end(), [&]( const std::shared_ptr<Constraint<TypeVariable, TypeDomain> > &c ){ c->update( domain ); });
  // }
  
  template <typename TypeVariable, typename TypeDomain>
  void printConstraints( const std::vector< std::shared_ptr<Constraint<TypeVariable, TypeDomain> > >&vecConstraints )
  {
    std::for_each( vecConstraints.begin(), vecConstraints.end(), []( const std::shared_ptr<Constraint<TypeVariable, TypeDomain> > &c ){ std::cout << *c << std::endl; });
  }

  // template <typename TypeVariable, typename TypeDomain>
  // void addAllInGrid( const std::vector< TypeVariable > &vec, TypeDomain &domain )
  // {
  //   std::for_each( vec.begin(), vec.end(), [&]( const TypeVariable &b ){ domain.add(b); });
  // }

  // template <typename TypeVariable, typename TypeDomain>
  // void clearAllInGrid( const std::vector< TypeVariable > &vec, TypeDomain &domain )
  // {
  //   std::for_each( vec.begin(), vec.end(), [&]( const TypeVariable &b ){ domain.clear(b); });
  // }

  int countSelectedVariables( const std::vector< Variable > &vec )
  {
    return std::count_if( vec.begin(), vec.end(), []( const Variable &b ){ return b.isSelected(); });
  }
}
