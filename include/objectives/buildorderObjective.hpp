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
#include "../misc/actionType.hpp"
#include "../domains/buildorderDomain.hpp"
// #include "../misc/raceActions.hpp"

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
    
    int v_heuristicVariable( const vector< int > &vecId, const vector< Action > *vecVariables, BuildOrderDomain *domain );
    int v_heuristicValue( const std::vector< double > &vecGlobalCosts, 
			  double &bestEstimatedCost,
			  int &bestValue ) const;
    void v_setHelper( const Action &b, const vector< Action > *vecVariables, const BuildOrderDomain *domain );

    virtual double v_postprocessOptimization( vector< Action > *vecActions,
					      BuildOrderDomain *domain,
					      double &bestCost );

    double v_postprocessSatisfaction( vector< Action > *vecVariables,
				      BuildOrderDomain *domain,
				      double &bestCost,
				      vector< Action > &bestSolution) const;
    
    // struct Tuple
    // {
    //   Tuple( string actor, string goal, int time, int waitTime = 0 )
    // 	: actor(actor), goal(goal), time(time), waitTime(waitTime) { }
      
    //   string actor;
    //   string goal;
    //   int time;
    //   int waitTime;
    // };
    
    struct State
    {
      State()
      	: seconds(0),
      	  stockMineral(0.),
      	  stockGas(0.),
      	  mineralWorkers(0),
      	  gasWorkers(0),
      	  supplyUsed(5),
      	  supplyCapacity(8),
      	  numberBases(1),
      	  numberRefineries(0),
	  numberPylons(0),
      	  resources(),
	  canBuild(),
	  busy{},
	  inMove{}
      	  // busy{Tuple("Protoss_Nexus", "Protoss_Probe", 20)},
      	  // inMove{ Tuple("Protoss_Probe", "Mineral", 0, 2),
	  //     Tuple("Protoss_Probe", "Mineral", 0, 2),
	  //     Tuple("Protoss_Probe", "Mineral", 0, 2),
	  //     Tuple("Protoss_Probe", "Mineral", 0, 2) }
      {
	initCanBuild();
      }
      
      State( int seconds,
	     double stockMineral,
	     double stockGas,
	     int mineralWorkers,
	     int gasWorkers,
	     int supplyUsed,
	     int supplyCapacity,
	     int numberBases,
	     int numberRefineries,
	     int numberPylons,
	     const map<string, int> &resources,
	     const map<string, bool> &canBuild,
	     const vector< Action > &busy,
	     const vector< std::pair<Action, int> > &inMove )
	: seconds(seconds),
	  stockMineral(stockMineral),
	  stockGas(stockGas),
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
      	seconds = 0;
      	stockMineral = 0.;
      	stockGas = 0.;
      	mineralWorkers = 0;
      	gasWorkers = 0;
      	supplyUsed = 5;
      	supplyCapacity = 8;
      	numberBases = 1;
      	numberRefineries = 0;
	numberPylons = 0;
      	resources.clear();
	initCanBuild();
      	busy.clear();
      	busy.emplace_back( "Protoss_Nexus", "Protoss_Probe", 20 );
      	inMove.clear();
	for( int i = 0 ; i < 4 ; ++i )
	  inMove.emplace_back( "Protoss_Probe", "Mineral", 0, 2 );
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
      int	mineralWorkers;
      int	gasWorkers;
      int	supplyUsed;
      int	supplyCapacity;
      int	numberBases;
      int	numberRefineries;
      int	numberPylons;
      map<string, int> resources;
      map<string, bool> canBuild;
      vector< Action > busy;
      vector< std::pair<Action, int> > inMove;      
    };

    struct BO
    {
      BO() { }

      BO( string fullName, int completedTime )
	: fullName(fullName), completedTime(completedTime) { }
      
      string fullName;
      int completedTime;
    };
    
    // struct Goal
    // {
    //   Goal() {}
      
    //   Goal( string fullName, string type, int toHave, int current )
    // 	: fullName(fullName), type(type), toHave(toHave), current(current)
    //   { }
      
    //   string	fullName;
    //   string	type;
    //   int	toHave;
    //   int	current;
    // };


    void makeVecVariables( const pair<string, int> &input, vector<Action> &variables, map< string, pair<int, int> > &goals );
    void makeVecVariables( const Action &action, vector<Action> &variables, int count );

    
    mutable State		currentState;
    // mutable vector<Goal>	goals;
    mutable map< string, pair<int, int> > goals;
    mutable vector<BO>		bo;
    // mutable RaceActions		raceActions;
    
  private:
    double v_cost( const vector< Action > *vecVariables, const BuildOrderDomain *domain, bool optimization ) const;
    void updateBusy() const;
    void updateInMove() const;
    bool makingPylons() const;
    void youMustConstructAdditionalPylons() const;
    double mineralsIn( int seconds ) const;
    double gasIn( int seconds ) const;    
  };
  

  /*******************/
  /* MakeSpanMinCost */
  /*******************/
  class MakeSpanMinCost : public BuildOrderObjective
  {
  public:
    MakeSpanMinCost();
    MakeSpanMinCost( const vector< pair<string, int> > &input, vector<Action> &variables );

  private:
    double v_postprocessOptimization( vector< Action > *vecVariables, BuildOrderDomain *domain, double &bestCost );
  };

  /*******************/
  /* MakeSpanMaxProd */
  /*******************/
  class MakeSpanMaxProd : public BuildOrderObjective
  {
  public:
    MakeSpanMaxProd();
    MakeSpanMaxProd( const vector< pair<string, int> > &input, vector<Action> &variables );

  private:
    double v_postprocessOptimization( vector< Action > *vecVariables, BuildOrderDomain *domain, double &bestCost );
  };
}
