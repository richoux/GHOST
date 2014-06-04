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


#include <typeinfo>

#include "../../include/variables/variable.hpp"

using namespace std;

namespace ghost
{
  int Variable::numberVariables = 0;
  
  Variable::Variable( string name, string fullName, int value )
    : name(name),
      fullName(fullName),
      id(Variable::numberVariables++),
      value(value)
  { }

  std::ostream& operator<<( std::ostream& os, const Variable& v )
  {
    return os
      << "Variable type: " <<  typeid(v).name() << std::endl
      << "Name: " << v.name << std::endl
      << "Full name: " << v.fullName << std::endl
      << "Id num: " << v.id << std::endl
      << "Value: " <<  v.value << std::endl
      << "-------" << std::endl;
  }

}
