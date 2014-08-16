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

#include "variable.hpp"
#include "../misc/races.hpp"

using namespace std;

namespace ghost
{
  enum ActionType{ building, unit, upgrade, research, special };

  /******************/
  /*** ActionData ***/
  /******************/  
  struct ActionData
  {
    ActionData();
    ActionData(int, int, int, int, ActionType, vector<string>, string, Race, string );
    ActionData(const ActionData&);
    ActionData& operator=(ActionData);

    inline int decreaseSeconds() { return --secondsRequired; }
    
    int			secondsRequired;
    int			costMineral;
    int			costGas;
    int			costSupply;
    ActionType		actionType;
    vector<string>	dependencies;
    string		creator; 
    Race		race;
    string		name;

    friend ostream& operator<<( ostream&, const ActionData& );

  private:
    void swap(ActionData&);
  };


  /**************/
  /*** Action ***/
  /**************/  
  class Action : public Variable
  {
  public:
    Action();
    Action(ActionData, int = -1);
    Action(const Action&);
    Action& operator=(Action);

    inline bool	isSelected()	const	{ return value != -1; }

    inline ActionData getData()		const { return data; }
    
    inline int getSecondsRequired()	const { return data.secondsRequired; }
    inline int getCostMineral()		const { return data.costMineral; }
    inline int getCostGas()		const { return data.costGas; }
    inline int getCostSupply()		const { return data.costSupply; }

    inline ActionType getType()		const { return data.actionType; }
    inline string getTypeString()	const	
    { 
      switch( data.actionType ) 
      {
      case building: return "Building";
      case unit: return "Unit";
      case upgrade: return "Upgrade";
      case research: return "Research";
      default: return "Unknown";
      }
    }
    
    inline vector<string> getDependencies()	const { return data.dependencies; }
    inline string	  getCreator()		const { return data.creator; }

    inline Race getRace()		const { return data.race; }
    inline string getRaceString()	const	
    { 
      switch( data.race ) 
      {
      case Terran: return "Terran";
      case Protoss: return "Protoss";
      case Zerg: return "Zerg";
      default: return "Unknown";
      }
    }

    inline void	swapValue( Action &other )	{ std::swap( value, other.value ); }

    inline bool operator==( const Action & other ) { return id == other.id; }
    
    friend ostream& operator<<( ostream&, const Action& );

  private:
    void swap(Action&);

    ActionData data;
  };
}
