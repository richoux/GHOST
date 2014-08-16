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
  /******************/
  /*** ActionData ***/
  /******************/  
  
  ActionData::ActionData() { }
  
  ActionData::ActionData(int second, 
			 int mineral, 
			 int gas, 
			 int supply,
			 ActionType actionType,
			 vector<string> dep, 
			 string creator, 
			 Race race,
			 string name)
    : secondsRequired(second),
      costMineral(mineral),
      costGas(gas), 
      costSupply(supply),
      actionType(actionType),
      dependencies(dep),
      creator(creator),
      race(race),
      name(name)
  { }

  ActionData::ActionData( const ActionData &other )
    : secondsRequired(other.secondsRequired),
      costMineral(other.costMineral),
      costGas(other.costGas), 
      costSupply(other.costSupply), 
      actionType(other.actionType), 
      dependencies(other.dependencies), 
      creator(other.creator),
      race(other.race),
      name(other.name)
  { }

  ActionData& ActionData::operator=( ActionData other )
  {
    this->swap( other );
    return *this;
  }

  void ActionData::swap( ActionData &other )
  {
    std::swap(this->secondsRequired, other.secondsRequired);
    std::swap(this->costMineral, other.costMineral);
    std::swap(this->costGas, other.costGas);
    std::swap(this->costSupply, other.costSupply);
    std::swap(this->actionType, other.actionType);
    std::swap(this->dependencies, other.dependencies);
    std::swap(this->creator, other.creator);
    std::swap(this->race, other.race);
    std::swap(this->name, other.name);
  }

  ostream& operator<<( ostream &os, const ActionData &a )
  {
    os
      << "Name: " << a.name << endl
      << "Seconds required: " << a.secondsRequired << endl
      << "Cost Mineral: " <<  a.costMineral << endl
      << "Cost Gas: " <<  a.costGas << endl
      << "Cost Supply: " <<  a.costSupply << endl
      << "Built/Trained/Researched/Upgraded/Morphed from: " <<  a.creator << endl
      << "Dependencies: ";
    
    for( const auto& d : a.dependencies )
      os << d << "  "; 
    
    os << endl << "-------" << endl;
    
    return os;
  }

  
  /**************/
  /*** Action ***/
  /**************/  

  Action::Action() { }
  
  Action::Action(ActionData data,
		 int value)
    : Variable( "", data.name, value ),
      data(data)
  {
    if( value == -1)
      value = id;
  }

  Action::Action( const Action &other )
    : Variable(other),
      data(other.data)
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
    std::swap(this->data, other.data);
  }

  ostream& operator<<( ostream &os, const Action &a )
  {
    os
      << "Type info: " <<  typeid(a).name() << endl
      << "Full name: " << a.fullName << endl
      << "Id num: " << a.id << endl
      << "Value: " <<  a.value << endl
      << "Type: " <<  a.getTypeString() << endl
      << "Race: " <<  a.getRaceString() << endl
      << a.data;
    
    return os;
  }
}
