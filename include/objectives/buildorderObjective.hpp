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
#include <map>
#include <memory>

#include "objective.hpp"
#include "../variables/action.hpp"
#include "../misc/actionMap.hpp"
#include "../domains/buildorderDomain.hpp"

using namespace std;

namespace ghost
{
  constexpr int goToBuild		= 5; //5
  constexpr int returnToMinerals	= 4; //4
  constexpr int fromBaseToMinerals	= 5; //2
  constexpr int fromMinToGas		= 5; //2

  constexpr double minRate		= 0.68; //1.07
  constexpr double gasRate		= 1.15; //1.66
    
  /***********************/
  /* BuildOrderObjective */
  /***********************/
  class BuildOrderObjective : public Objective<Action, BuildOrderDomain>
  {
  public:
    BuildOrderObjective( const string &name );
    BuildOrderObjective( const string &name, const vector< pair<string, int> > &input, vector<Action> &variables );

    void printBO() const;
    
  protected:
    double v_cost( vector< Action > *vecVariables, BuildOrderDomain *domain ) const;
    double costOpti( vector< Action > *vecVariables ) const;
    
    int v_heuristicVariable( const vector< int > &vecId, const vector< Action > *vecVariables, BuildOrderDomain *domain );
    int v_heuristicValue( const vector< double > &vecGlobalCosts, 
			  double &bestEstimatedCost,
			  int &bestValue ) const;
    void v_setHelper( const Action &b, const vector< Action > *vecVariables, const BuildOrderDomain *domain );

    double v_postprocessOptimization( vector< Action > *vecActions,
				      BuildOrderDomain *domain,
				      double &bestCost,
				      double opt_timeout );

    double v_postprocessSatisfaction( vector< Action > *vecVariables,
				      BuildOrderDomain *domain,
				      double &bestCost,
				      vector< Action > &bestSolution,
				      double sat_timeout ) const;
    
    struct ActionPrep
    {
      ActionPrep( ActionData action, int waitTime, int id )
    	: action(action), waitTime(waitTime), id(id) { }

      ActionData action;
      int waitTime;
      int id;
    };
    
    struct State
    {
      State()
      	: seconds(0),
      	  stockMineral(0.),
      	  stockGas(0.),
	  mineralsBooked(0),
	  gasBooked(0),
      	  mineralWorkers(0),
      	  gasWorkers(0),
      	  supplyUsed(5),
      	  supplyCapacity(9),
      	  numberBases(1),
      	  numberRefineries(0),
	  numberPylons(0),
      	  resources(),
	  busy{actionOf["Protoss_Probe"]},
	  inMove{ ActionPrep{ actionOf["Protoss_Mineral"], fromBaseToMinerals, 0 },
	      ActionPrep{ actionOf["Protoss_Mineral"], fromBaseToMinerals, 1 },
		ActionPrep{ actionOf["Protoss_Mineral"], fromBaseToMinerals, 2 },
		  ActionPrep{ actionOf["Protoss_Mineral"], fromBaseToMinerals, 3 } }
      { }
      
      State( int seconds,
	     double stockMineral,
	     double stockGas,
	     int mineralsBooked,
	     int gasBooked,
	     int mineralWorkers,
	     int gasWorkers,
	     int supplyUsed,
	     int supplyCapacity,
	     int numberBases,
	     int numberRefineries,
	     int numberPylons,
	     const map<string, pair<int, int> > &resources,
	     const vector< ActionData > &busy,
	     const vector< ActionPrep > &inMove )
	: seconds(seconds),
	  stockMineral(stockMineral),
	  stockGas(stockGas),
	  mineralsBooked(mineralsBooked),
	  gasBooked(gasBooked),
	  mineralWorkers(mineralWorkers),
	  gasWorkers(gasWorkers),
	  supplyUsed(supplyUsed),
	  supplyCapacity(supplyCapacity),
	  numberBases(numberBases),
	  numberRefineries(numberRefineries),
	  numberPylons(numberPylons),
	  resources(resources),
	  busy(busy),
	  inMove(inMove)
      { }

      void reset()
      {
      	seconds		= 0;
      	stockMineral	= 0.;
      	stockGas	= 0.;
	mineralsBooked	= 0;
	gasBooked	= 0;
      	mineralWorkers	= 0;
      	gasWorkers	= 0;
      	supplyUsed	= 5;
      	supplyCapacity	= 9;
      	numberBases	= 1;
      	numberRefineries = 0;
	numberPylons	= 0;
      	resources.clear();
	resources["Protoss_Nexus"].first = 1;
	resources["Protoss_Nexus"].second = 0;	
      	busy.clear();
      	busy.push_back( actionOf["Protoss_Probe"] );
      	inMove.clear();
	for( int i = 0 ; i < 4 ; ++i )
	  inMove.push_back( ActionPrep{ actionOf["Protoss_Mineral"], fromBaseToMinerals, i } );
      }

      int	seconds;
      double	stockMineral;
      double	stockGas;
      int	mineralsBooked;
      int	gasBooked;
      int	mineralWorkers;
      int	gasWorkers;
      int	supplyUsed;
      int	supplyCapacity;
      int	numberBases;
      int	numberRefineries;
      int	numberPylons;
      map<string, pair<int, int> > resources;
      vector< ActionData > busy;
      vector< ActionPrep > inMove;      
    };

    struct BO
    {
      BO() { }

      BO( string fullName, int startTime, int completedTime )
	: fullName(fullName), startTime(startTime), completedTime(completedTime) { }
      
      string fullName;
      int startTime;
      int completedTime;
    };
    
    void makeVecVariables( const pair<string, int> &input, vector<Action> &variables, map< string, pair<int, int> > &goals );
    void makeVecVariables( const Action &action, vector<Action> &variables, int count );

    
    mutable State				currentState;
    mutable map< string, pair<int, int> >	goals;
    mutable vector<BO>				bo;
    mutable vector<BO>				bestBO;
    
  private:
    void updateBusy() const;
    void updateInMove() const;
    void dealWithWorkers() const;
    bool canHandleBuilding( const Action& ) const;
    bool canHandleNotBuilding( const Action& ) const;
    bool handleActionToDo( const Action& ) const;
    void produceUnitsFirst( vector<Action>::iterator&, vector<Action>* ) const;
    bool makingPylons() const;
    void youMustConstructAdditionalPylons() const;
    void pushInBusy( const string& ) const;
    bool dependenciesCheck( const string& ) const;
    
    // rough estimations
    inline double mineralsIn( int duration )	const { return currentState.mineralWorkers * minRate * duration; }
    inline double gasIn( int duration )		const { return currentState.gasWorkers * gasRate * duration; }

    // sharp estimations
    double sharpMineralsIn( int duration, int inSeconds = 0 ) const;
    double sharpGasIn( int duration, int inSeconds = 0 ) const;
  };
  
  
  /*******************/
  /* MakeSpanMinCost */
  /*******************/
  class MakeSpanMinCost : public BuildOrderObjective
  {
  public:
    MakeSpanMinCost();
    MakeSpanMinCost( const vector< pair<string, int> > &input, vector<Action> &variables );

  // private:
  //   double v_postprocessOptimization( vector< Action > *vecVariables, BuildOrderDomain *domain, double &bestCost );
  };

  /*******************/
  /* MakeSpanMaxProd */
  /*******************/
  class MakeSpanMaxProd : public BuildOrderObjective
  {
  public:
    MakeSpanMaxProd();
    MakeSpanMaxProd( const vector< pair<string, int> > &input, vector<Action> &variables );

  // private:
  //   double v_postprocessOptimization( vector< Action > *vecVariables, BuildOrderDomain *domain, double &bestCost );
  };
}
