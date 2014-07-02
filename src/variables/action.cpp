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

#include "../../include/variables/action.hpp"

namespace ghost
{
  Action::Action() { }
  
  Action::Action(int frame, 
		 int mineral, 
		 int gas, 
		 int supply, 
		 vector<string> dep, 
		 string creator, 
		 Race race,
		 string name,
		 string fullName,
		 int value)
    : Variable( name, fullName, value ),
      frameRequired(frame),
      costMineral(mineral),
      costGas(gas), 
      costSupply(supply), 
      dependencies(dep), 
      creator(creator),
      race(race)
  { }

  Action::Action( const Action &other )
    : Variable(other),
      frameRequired(other.frameRequired),
      costMineral(other.costMineral),
      costGas(other.costGas), 
      costSupply(other.costSupply), 
      dependencies(other.dependencies), 
      creator(other.creator),
      race(other.race)
  { }

  Action& Action::operator=( Action other )
  {
    this->swap( other );
    return *this;
  }

  void Action::swap( Action &other )
  {
    std::swap(this->name, other.name);
    std::swap(this->fullName, other.fullName);
    std::swap(this->id, other.id);
    std::swap(this->value, other.value);
    std::swap(this->frameRequired, other.frameRequired);
    std::swap(this->costMineral, other.costMineral);
    std::swap(this->costGas, other.costGas);
    std::swap(this->costSupply, other.costSupply);
    std::swap(this->dependencies, other.dependencies);
    std::swap(this->creator, other.creator);
    std::swap(this->race, other.race);
  }

  ostream& operator<<( ostream &os, const Action &a )
  {
    os
      << "Type: " <<  typeid(a).name() << endl
      << "Race: " <<  a.getRace() << endl
      << "Name: " << a.name << endl
      << "Full name: " << a.fullName << endl
      << "Id num: " << a.id << endl
      << "Frame required: " << a.frameRequired << endl
      << "Cost Mineral: " <<  a.costMineral << endl
      << "Cost Gas: " <<  a.costGas << endl
      << "Cost Supply: " <<  a.costSupply << endl
      << "Built/Trained/Researched/Upgraded/Morphed from: " <<  a.creator << endl
      << "Dependencies: ";
    
    for( auto& d : a.dependencies )
      os << d << "  "; 
    
    os << "-------" << endl;

    return os;
  }
}