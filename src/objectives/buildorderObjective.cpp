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


#include <vector>
#include <map>
#include <memory>
#include <algorithm>
#include <limits>
#include <cmath>
#include <chrono>
#include <ctime>
#include <numeric>

#include "../../include/objectives/buildorderObjective.hpp"
#include "../../include/variables/action.hpp"
#include "../../include/constraints/buildorderConstraint.hpp"
// #include "../../include/misc/actionFactory.hpp"

using namespace std;

namespace ghost
{
  /***********************/
  /* BuildOrderObjective */
  /***********************/
  BuildOrderObjective::BuildOrderObjective( const string &name )
    : Objective<Action, BuildOrderDomain>( name )//,
    // currentState( State() ),
    // goals( vector<Goal>() )
  { }
  
  BuildOrderObjective::BuildOrderObjective( const string &name,
					    const vector< pair<string, int> > &input,
					    vector<Action> &variables )
    : Objective<Action, BuildOrderDomain>( name )//,
    // currentState( State() ),
    // goals(vector<Goal>())
  {
    for( const auto &i : input)
      makeVecVariables( i, variables, goals );
  }

  double BuildOrderObjective::v_cost( const vector< Action > *vecVariables, const BuildOrderDomain *domain ) const
  {
    // currentState.reset();
    int seconds = 0;
    // auto nextAction = vecVariables->begin();
    // string creator = nextAction->getCreator();
    
    // while( nextAction != vecVariables->end() )
    // {
    //   ++seconds;
      
    //   // update mineral / gas stocks
    //   stockMineral += mineralWorkers * 1.08; // 1.08 mineral per worker per second in average
    //   stockGas += gasWorkers * 1.68; // 1.68 gas per worker per second in average

    //   // update busy list
    //   updateBusy( currentState );

    //   // update inMove list
    //   updateInMove( currentState );

    //   // produce a worker if I can, ie:
    //   // 1. if I have at least 50 minerals
    //   // 2. if I have at least one available Nexus
    //   // 3. if I am not supply blocked.
    //   if( stockMineral >= 50 && resources["Protoss_Nexus"] > 0 && supplyUsed < supplyCapacity )
    //   {
    // 	stockMineral -= 50;
    // 	++supplyUsed;
    // 	--resources["Protoss_Nexus"];
    // 	busy.push_back( Tuple("Protoss_Nexus", "Protoss_Probe", 20) );
    //   }

    //   // build a pylon if I must, ie:
    //   // 1. if my supply cap cannot manage the next global unit production
    //   youMustConstructAdditionalPylons( currentState );

    //   // can I handle the next action?
    //   if( stockMineral >= nextAction->getCostMineral()
    // 	  &&
    // 	  stockGas >= nextAction->getCostGas()
    // 	  &&
    // 	  supplyUsed + nextAction->getCostSupply() <= supplyCapacity
    // 	  &&
    // 	  ( creator.empty() || resources( creator ) > 0 ) )
    //   {
    // 	stockMineral -= nextAction->getCostMineral();
    // 	stockGas -= nextAction->getCostGas();
    // 	supplyUsed += nextAction->getCostSupply();

    // 	if( !creator.empty() )
    // 	  --resources( creator );

    // 	busy.push_back( Tuple( creator, nextAction->getFullName(), nextAction->getSecondsRequired() ) );
    // 	++nextAction;
    // 	if( nextAction != vecVariables->end() )
    // 	  creator = nextAction->getCreator();
    //   }
    // }

    // seconds += busy.at(0).time;
    
    return static_cast<double>( seconds );
  }

  int BuildOrderObjective::v_heuristicVariable( const vector< int > &vecId, const vector< Action > *vecVariables, BuildOrderDomain *domain )
  {
    return vecId[ randomVar.getRandNum( vecId.size() ) ];
  }

  void BuildOrderObjective::v_setHelper( const Action &b, const vector< Action > *vecVariables, const BuildOrderDomain *domain )
  {
    int pos = b.getValue();

    // the idea here is to favor larger values: if you have to move a
    // variable, move it as far as possible, in order to not disturb
    // too much what has been done so far.
    heuristicValueHelper.at( pos ) = domain->getSize() - pos;
  }


  double BuildOrderObjective::v_postprocessOptimization( vector< Action > *vecVariables, BuildOrderDomain *domain, double &bestCost ) 
  {
    chrono::time_point<chrono::system_clock> startPostprocess = chrono::system_clock::now(); 
    chrono::duration<double,milli> postprocesstimer(0);

    // vector<int> tabuList( vecVariables->size() );
    // std::fill( tabuList.begin(), tabuList.end(), 0 );

    // multimap<int, Action> actionSameSize;
    
    // for( const auto &v : *vecVariables )
    //   actionSameSize.insert( make_pair( v.getSurface(), v ) );

    // Action *oldVariable;
    // vector<int> goodVar;
    // Action *toSwap;
    // bool mustSwap;
    
    // bestCost = v_cost( vecVariables, domain );
    // double currentCost = bestCost;

    // while( (postprocesstimer = chrono::system_clock::now() - startPostprocess).count() < static_cast<int>( ceil(OPT_TIME / 100) ) && bestCost > 0 )
    // {
    //   goodVar.clear();

    //   for( int i = 0; i < tabuList.size(); ++i )
    //   {
    // 	if( tabuList[i] <= 1 )
    // 	  tabuList[i] = 0;
    // 	else
    // 	  --tabuList[i];
    //   }

    //   for( int i = 0; i < vecVariables->size(); ++i )
    //   {
    // 	if( tabuList[i] == 0 )
    // 	  goodVar.push_back( i );
    //   }

    //   if( goodVar.empty() )
    // 	for( int i = 0; i < vecVariables->size(); ++i )
    // 	  goodVar.push_back( i );	

    //   int index = v_heuristicVariable( goodVar, vecVariables, domain );
    //   oldVariable = &vecVariables->at( index );
      // auto surface = actionSameSize.equal_range( oldVariable->getSurface() );
	
      // for( auto &it = surface.first; it != surface.second; ++it )
      // {
      // 	mustSwap = false;
      // 	if( it->second.getId() != oldVariable->getId() )
      // 	{
      // 	  domain->swap( it->second, *oldVariable );
	    
      // 	  currentCost = v_cost( vecVariables, domain );
      // 	  if( currentCost < bestCost )
      // 	  {
      // 	    bestCost = currentCost;
      // 	    toSwap = &it->second;
      // 	    mustSwap = true;
      // 	  }

      // 	  domain->swap( it->second, *oldVariable );
      // 	}
	  
      // 	if( mustSwap )
      // 	  domain->swap( *toSwap, *oldVariable );
      // }

    //   tabuList[ index ] = 2;//std::max(2, static_cast<int>( ceil(TABU / 2) ) );
    // }

    return postprocesstimer.count();
  }

  void BuildOrderObjective::makeVecVariables( const pair<string, int> &input, vector<Action> &variables, vector<Goal> &goals )
  {
    // Action action = factoryAction( input.first );
    // Goal goal( action.getFullName(), action.getTypeString(), input.second, 0 );
    // goals.push_back( goal );
    // makeVecVariables( action, variables, input.second );
  }

  void BuildOrderObjective::makeVecVariables( const Action &action, vector<Action> &variables, int count )
  {
    string name = action.getFullName();

    // test if we don't already have this action into variables.
    // make exceptions for HT and DT
    // if( name.compare( "Protoss_High_Templar" ) != 0 && name.compare( "Protoss_Dark_Templar" ) != 0 )
    //   for( const auto &v : variables )
    // 	if( v.getFullName().compare( name ) == 0)
    // 	  return;
    
    // if( count > 0 )
    // {
    //   variables.push_back( action );
    //   for( int i = 1; i < count; ++i )
    // 	variables.push_back( factoryAction( action.getFullName() ) );
      
    //   for( const auto &d : action.getDependencies() )
    // 	if( d.compare( "Protoss_High_Templar" ) == 0 || d.compare( "Protoss_Dark_Templar" ) == 0 )
    // 	  makeVecVariables( factoryAction( d ), variables, 2 * count ); // Each (dark) archon needs 2 (dark) templars 
    // 	else
    // 	  if( d.compare( "Protoss_Nexus" ) != 0 )
    // 	    makeVecVariables( factoryAction( d ), variables, 1 );
    // }    
  }
  
  /***********/
  /* NoneObj */
  /***********/
  // NoneObj::NoneObj() : BuildOrderObjective( "buildOrderNone" ) { }

  // double NoneObj::v_cost( const vector< Action > *vecVariables, const BuildOrderDomain *domain ) const
  // {
  //   return 0;
  // }

  // int NoneObj::v_heuristicVariable( const vector< int > &vecId, const vector< Action > *vecVariables, BuildOrderDomain *domain )
  // {
  //   return 0;
  // }

  // void NoneObj::v_setHelper( const Action &b, const vector< Action > *vecVariables, const BuildOrderDomain *domain )
  // {
  // }

  // double NoneObj::v_postprocessOptimization( vector< Action > *vecVariables, BuildOrderDomain *domain, double &bestCost ) { return 0.; }
  

  /*******************/
  /* MakeSpanMinCost */
  /*******************/
  MakeSpanMinCost::MakeSpanMinCost() : BuildOrderObjective( "MakeSpanMinCost" ) { }

  double MakeSpanMinCost::v_postprocessOptimization( vector< Action > *vecVariables, BuildOrderDomain *domain, double &bestCost )
  {

    return 0;
  }

  
  /*******************/
  /* MakeSpanMaxProd */
  /*******************/
  MakeSpanMaxProd::MakeSpanMaxProd() : BuildOrderObjective( "MakeSpanMaxProd" ) { }

  double MakeSpanMaxProd::v_postprocessOptimization( vector< Action > *vecVariables, BuildOrderDomain *domain, double &bestCost )
  {

    return 0;
  }
}
