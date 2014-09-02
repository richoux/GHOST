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


#pragma once

#include <vector>
#include <cmath>

#include "variable.hpp"
#include "../misc/races.hpp"
#include "../misc/sizes.hpp"
#include "../misc/damageTypes.hpp"

using namespace std;

namespace ghost
{
  struct Coord
  {
    int x, y;
  };

  struct Range
  {
    double min, max;
  };

  struct Splash
  {
    double ray1, ray2, ray3;
  };

  /****************/
  /*** UnitData ***/
  /****************/  
  struct UnitData
  {
    UnitData();
    UnitData( string, int, int, Size, int, int, int, DamageType, Range, Splash );
    UnitData( const UnitData& );
    UnitData& operator=( UnitData );

    inline int takeHit( int point )		{ return hp -= point; }
    inline bool isDead()		const	{ return hp <= 0; }
    inline bool canShoot()		const	{ return canShootIn == 0; }
    inline void justShot()			{ canShootIn = cooldown; }
    inline void oneStep()			{ --canShootIn; }

    inline double distanceFrom( const UnitData &u ) const
    { return sqrt( pow( u.coord.x - coord.x, 2 ) + pow( u.coord.y - coord.y, 2 ) ); }

    inline bool isInRange( const UnitData &u )  const
    { return distanceFrom(u) >= range.min && distanceFrom(u) <= range.max; }
    
    string	name;
    Coord	coord;
    int		hp;
    int		armor;
    Size	size;
    int		canShootIn;
    int		cooldown;
    int		damage;
    DamageType	damageType;
    Range	range;
    Splash	splashRadius;

    friend ostream& operator<<( ostream&, const UnitData& );

  private:
    void swap( UnitData& );
  };


  /************/
  /*** Unit ***/
  /************/  
  class Unit : public Variable
  {
  public:
    Unit();
    Unit( UnitData, Coord, int = -1 );
    Unit( UnitData, int, int, int = -1 );
    Unit( const Unit& );
    Unit& operator=( Unit );

    inline bool	isSelected()		const { return value != -1; }
    inline int takeHit( int point )	      { return data.takeHit( point ); }
    inline bool isDead()		const { return data.isDead(); }
    inline bool canShoot()		const { return data.canShoot(); }
    inline void justShot()		      { data.justShot(); }
    inline void oneStep()		      { data.oneStep(); }

    inline double distanceFrom( const Unit &u ) const
    { return data.distanceFrom( u.data ); }

    inline bool isInRange( const Unit &u ) const
    { return data.isInRange( u.data ); }

    inline bool isInRange( const UnitData &ud ) const
    { return data.isInRange( ud ); }

    inline UnitData getData()		const { return data; }
    inline void setData( UnitData u )	      { data = u; }

    inline Coord getCoord()		const { return data.coord; }
    inline int getX()			const { return data.coord.x; }
    inline int getY()			const { return data.coord.y; }

    inline void setCoord( Coord c )	      { data.coord = c; }
    inline void setX( int x )		      { data.coord.x = x; }
    inline void setY( int y )		      { data.coord.y = y; }
    
    inline int getHP()			const { return data.hp; }
    inline int canShootIn()		const { return data.canShootIn; }
    inline int getArmor()		const { return data.armor; }
    inline Size getSize()		const { return data.size; }
    inline string getSizeString()	const
    {
      switch( data.size ) 
      {
      case Small: return "small";
      case Medium: return "medium";
      case Large: return "large";
      default: return "unknown";
      }
    }

    inline int getCooldown()		const { return data.cooldown; }
    inline int getDamage()		const { return data.damage; }

    inline DamageType getDamageType()	const { return data.damageType; }
    inline string getDamageTypeString()	const	
    { 
      switch( data.damageType ) 
      {
      case Concussive: return "concussive";
      case Normal: return "normal";
      case Explosive: return "explosive";
      default: return "unknown";
      }
    }
    
    inline Range getRange()		const { return data.range; }
    inline double getRangeMin()		const { return data.range.min; }
    inline double getRangeMax()		const { return data.range.max; }
    inline Splash getSplashRadius()	const { return data.splashRadius; }
    inline double getSplashRadiusMin()	const { return data.splashRadius.ray1; }
    inline double getSplashRadiusMed()	const { return data.splashRadius.ray2; }
    inline double getSplashRadiusMax()	const { return data.splashRadius.ray3; }

    inline void	swapValue( Unit &other )	{ std::swap( value, other.value ); }

    inline bool operator==( const Unit & other ) { return id == other.id; }
    
    friend ostream& operator<<( ostream&, const Unit& );

  private:
    void swap( Unit& );

    UnitData	data;
  };
}
