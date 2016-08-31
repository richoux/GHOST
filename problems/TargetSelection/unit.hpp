/*
 * GHOST (General meta-Heuristic Optimization Solving Tool) is a C++ library 
 * designed to help developers to model and implement optimization problem 
 * solving. 
 * GHOST is a meta-heuristic solver aiming to solve any kind of combinatorial 
 * and optimization real-time problems represented by a CSP/COP. 
 * GHOST has been first developped to help making AI for the RTS game
 * StarCraft: Brood war, but can be used for any kind of applications where 
 * solving combinatorial and optimization problems within some tenth of 
 * milliseconds is needed. It is a generalization of the Wall-in project.
 * Please visit https://github.com/richoux/GHOST for further information.
 * 
 * Copyright (C) 2014-2016 Florian Richoux
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

#include "../../src/variable.hpp"
#include "races.hpp"
#include "sizes.hpp"
#include "damageTypes.hpp"

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
    UnitData( string, double, int, Size, int, int, int, DamageType, Range, Splash, bool, bool = false );
    UnitData( const UnitData& );
    UnitData& operator=( UnitData );

    inline double takeHit( double point )	{ return hp -= point; }
    inline bool isDead()		const	{ return hp <= 0.; }
    inline bool canShoot()		const	{ return canShootIn == 0; }
    inline void justShot()			{ canShootIn = cooldown; }
    inline void oneStep()			{ --canShootIn; }

    string	name;
    double	hp;
    double	initialHP;
    int		armor;
    Size	size;
    int		canShootIn;
    int		cooldown;
    int		damage;
    DamageType	damageType;
    Range	range;
    Splash	splashRadius;
    bool	doSplash;
    bool	doLinearSplash;

    friend ostream& operator<<( ostream&, const UnitData& );

  private:
    void swap( UnitData& );
  };


  
  /*****************/
  /*** UnitEnemy ***/
  /*****************/
  class Unit;
  struct UnitEnemy
  {
    UnitData data;
    Coord coord;

    UnitEnemy( UnitData data )
      : data(data)
    { }

    UnitEnemy( UnitData data, Coord coord )
      : data(data), coord(coord)
    { }

    bool isDead()				const;
    bool canShoot()				const;
    void justShot();
    void oneStep();
    double distanceFrom( const Unit& )		const;
    double distanceFrom( const UnitEnemy& )	const;
    bool isInRange( const Unit& )		const;
    bool isInRangeAndAlive( const Unit& )	const;
    double doDamageAgainst( int index, vector<Unit> &vecUnit, int num );
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
    inline double takeHit( double point )     { return data.takeHit( point ); }
    inline bool isDead()		const { return data.isDead(); }
    inline bool canShoot()		const { return data.canShoot(); }
    inline void justShot()		      { data.justShot(); }
    inline void oneStep()		      { data.oneStep(); }

    inline double distanceFrom( const UnitEnemy &u ) const
    { return sqrt( pow( u.coord.x - coord.x, 2 ) + pow( u.coord.y - coord.y, 2 ) ); }

    inline double distanceFrom( const Unit &u ) const
    { return sqrt( pow( u.coord.x - coord.x, 2 ) + pow( u.coord.y - coord.y, 2 ) ); }

    inline bool isInRange( const UnitEnemy &u ) const
    { return distanceFrom(u) >= data.range.min && distanceFrom(u) <= data.range.max; }

    inline bool isInRangeAndAlive( const UnitEnemy &u ) const
    { return !u.isDead() && isInRange( u ); }

    inline UnitData getData()		const { return data; }
    inline void setData( UnitData u )	      { data = u; }

    inline Coord getCoord()		const { return coord; }
    inline int getX()			const { return coord.x; }
    inline int getY()			const { return coord.y; }

    inline void setCoord( Coord c )	      { coord = c; }
    inline void setX( int x )		      { coord.x = x; }
    inline void setY( int y )		      { coord.y = y; }
    
    inline double getHP()		const { return data.hp; }
    inline double getInitialHP()	const { return data.initialHP; }
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
      case Explosive: return "explosive";\
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

    inline bool isSplash()		const { return data.doSplash; }
    inline bool isLinearSplash()	const { return data.doLinearSplash; }
    
    double doDamage( vector<UnitEnemy> &vecEnemy );
    vector<double> computeDamage( vector<UnitEnemy> *vecEnemy ) const;
    
    inline void	swapValue( Unit &other )	{ std::swap( value, other.value ); }

    inline bool operator==( const Unit & other ) { return id == other.id; }
    
    friend ostream& operator<<( ostream&, const Unit& );

  private:
    void swap( Unit& );

    UnitData	data;
    Coord	coord;
  };
}
