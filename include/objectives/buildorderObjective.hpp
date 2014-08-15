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
    double v_cost( const vector< Action > *vecVariables, const BuildOrderDomain *domain ) const;
    double costOpti( vector< Action > *vecVariables ) const;
    
    int v_heuristicVariable( const vector< int > &vecId, const vector< Action > *vecVariables, BuildOrderDomain *domain );
    int v_heuristicValue( const std::vector< double > &vecGlobalCosts, 
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
    
    struct Tuple
    {
      Tuple( ActionData action, int waitTime, bool done = false )
    	: action(action), waitTime(waitTime), done(done) { }

      ActionData action;
      int waitTime;
      bool done;
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
	  canBuild(),
	  busy{actionOf["Protoss_Probe"]},
	  inMove{ Tuple{ actionOf["Protoss_Mineral"], 2, false },
	      Tuple{ actionOf["Protoss_Mineral"], 2, false },
	      Tuple{ actionOf["Protoss_Mineral"], 2, false },
	      Tuple{ actionOf["Protoss_Mineral"], 2, false } }
      {
	initCanBuild();
      }
      
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
	     const map<string, bool> &canBuild,
	     const vector< ActionData > &busy,
	     const vector< Tuple > &inMove )
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
	  canBuild(canBuild),
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
	initCanBuild();
      	busy.clear();
      	busy.push_back( actionOf["Protoss_Probe"] );
      	inMove.clear();
	for( int i = 0 ; i < 4 ; ++i )
	  inMove.push_back( Tuple{ actionOf["Protoss_Mineral"], 2, false } );
      }

      void initCanBuild()
      {
	canBuild.clear();
	canBuild["Protoss_Probe"] = true;
	canBuild["Protoss_Nexus"] = true;
	canBuild["Protoss_Pylon"] = true;
	canBuild["Protoss_Gateway"] = true;
	canBuild["Protoss_Assimilator"] = true;
	canBuild["Protoss_Forge"] = true;
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
      map<string, bool> canBuild;
      vector< ActionData > busy;
      vector< Tuple > inMove;      
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

    
    mutable State		currentState;
    mutable map< string, pair<int, int> > goals;
    mutable vector<BO>		bo;
    
  private:
    void updateBusy() const;
    void updateInMove() const;
    void dealWithWorkers() const;
    bool handleNextAction( const Action& ) const;
    bool makingPylons() const;
    void youMustConstructAdditionalPylons() const;
    void pushInBusy( ActionData ) const;
    bool dependenciesCheck( string ) const;
    
    // rough estimations
    inline double mineralsIn( int duration )	const { return currentState.mineralWorkers * 1.08 * duration; }
    inline double gasIn( int duration )		const { return currentState.gasWorkers * 1.68 * duration; }

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
