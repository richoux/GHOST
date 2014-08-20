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

#include "../../include/variables/unit.hpp"

namespace ghost
{
  /****************/
  /*** UnitData ***/
  /****************/  
  
  UnitData::UnitData() { }
  
  UnitData::UnitData( string name,
		      int hp,
		      int armor,
		      Size size,
		      int canShootIn,
		      int cooldown,
		      int damage,
		      DamageType damageType,
		      Range range,
		      Splash splashRadius )
    : name(name),
      hp(hp),
      armor(armor),
      size(size),
      canShootIn(canShootIn),
      cooldown(cooldown),
      damage(damage),
      damageType(damageType),
      range(range),
      splashRadius(splashRadius)
  { }

  UnitData::UnitData( const UnitData &other )
    : name(other.name),
      coord(other.coord),
      hp(other.hp),
      armor(other.armor),
      size(other.size),
      canShootIn(other.canShootIn),
      cooldown(other.cooldown),
      damage(other.damage),
      damageType(other.damageType),
      range(other.range),
      splashRadius(other.splashRadius)
  { }

  UnitData& UnitData::operator=( UnitData other )
  {
    this->swap( other );
    return *this;
  }

  void UnitData::swap( UnitData &other )
  {
    std::swap(this->name, other.name);
    std::swap(this->coord, other.coord);
    std::swap(this->hp, other.hp);
    std::swap(this->armor, other.armor);
    std::swap(this->size, other.size);
    std::swap(this->canShootIn, other.canShootIn);
    std::swap(this->cooldown, other.cooldown);
    std::swap(this->damage, other.damage);
    std::swap(this->damageType, other.damageType);
    std::swap(this->range, other.range);
    std::swap(this->splashRadius, other.splashRadius);
  }

  ostream& operator<<( ostream &os, const UnitData &u )
  {
    os
      << "Name: " << u.name << endl
      << "Can shoot in: " << u.canShootIn << " seconds" << endl
      << "Cooldown: " <<  u.cooldown << endl
      << "HP: " <<  u.hp << endl
      << "Damage: " <<  u.damage << endl
      << "Armor" <<  u.armor << endl
      << "-------" << endl;
    
    return os;
  }

  
  /**************/
  /*** Unit ***/
  /**************/  

  Unit::Unit() { }

  Unit::Unit(UnitData data,
	     Coord coord,
	     int value)
    : Variable( "", data.name, value ),
      data(data)
  {
    if( value == -1)
      value = id;

    data.coord = coord;
  }
  
  Unit::Unit(UnitData data,
	     int x,
	     int y,
	     int value)
    : Variable( "", data.name, value ),
      data(data)
  {
    if( value == -1)
      value = id;

    data.coord.x = x;
    data.coord.y = y;
  }

  Unit::Unit( const Unit &other )
    : Variable(other),
      data(other.data)
  { }

  Unit& Unit::operator=( Unit other )
  {
    this->swap( other );
    return *this;
  }

  void Unit::swap( Unit &other )
  {
    std::swap(this->name, other.name);
    std::swap(this->fullName, other.fullName);
    std::swap(this->id, other.id);
    std::swap(this->value, other.value);
    std::swap(this->data, other.data);
  }

  ostream& operator<<( ostream &os, const Unit &u )
  {
    os
      << "Type info: " <<  typeid(u).name() << endl
      << "Full name: " << u.fullName << endl
      << "Coord: (" << u.data.coord.x << ", " << u.data.coord.y << ")" << endl 
      << "Id num: " << u.id << endl
      << "Value: " <<  u.value << endl
      << u.data;
    
    return os;
  }
}
