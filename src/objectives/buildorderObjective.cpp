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
#include "../../include/misc/actionFactory.hpp"

using namespace std;

namespace ghost
{
  /***********************/
  /* BuildOrderObjective */
  /***********************/
  BuildOrderObjective::BuildOrderObjective( const string &name )
    : Objective<Action, BuildOrderDomain>( name ),
    currentState( State() ),
    // goals( vector<Goal>() ),
    goals( map<string, pair<int, int> >() ),
    bo( vector<BO>() )
  { }
  
  BuildOrderObjective::BuildOrderObjective( const string &name,
					    const vector< pair<string, int> > &input,
					    vector<Action> &variables )
    : Objective<Action, BuildOrderDomain>( name ),
    currentState( State() ),
    // goals( vector<Goal>() ),
    goals( map<string, pair<int, int> >() ),
    bo( vector<BO>() )
  {
    for( const auto &i : input)
      makeVecVariables( i, variables, goals );
  }

  double BuildOrderObjective::v_cost( const vector< Action > *vecVariables, const BuildOrderDomain *domain ) const
  {
    return v_cost( vecVariables, domain, false );
  }
  
  double BuildOrderObjective::v_cost( const vector< Action > *vecVariables, const BuildOrderDomain *domain, bool optimization ) const
  {
    currentState.reset();
    int seconds = 0;
    auto nextAction = vecVariables->begin();
    string creator = nextAction->getCreator();
    
    while( nextAction != vecVariables->end() && !currentState.busy.empty() )
    {
      ++seconds;
      
      // update mineral / gas stocks
      currentState.stockMineral += currentState.mineralWorkers * 1.08; // 1.08 mineral per worker per second in average
      currentState.stockGas += currentState.gasWorkers * 1.68; // 1.68 gas per worker per second in average

      // update busy list
      updateBusy( seconds );

      // update inMove list
      updateInMove();

      if( nextAction != vecVariables->end() )
      {
	// send workers to gas, if need and possible
	if( currentState.gasWorkers < currentState.numberRefineries * 3 )
	{
	  // if we have few workers mining, do not sent them to gas
	  int toGas = min( 3, currentState.mineralWorkers - 3 );
	  for( int i = 0 ; i < toGas ; ++i )
	  {
	    currentState.inMove.emplace_back( "Protoss_Probe", "Gas", 0, 2 );
	    --currentState.mineralWorkers;
	  }
	}
	
	// produce a worker if I can, ie:
	// 1. if I have at least 50 minerals
	// 2. if I have at least one available Nexus
	// 3. if I am not supply blocked
	// 4. if I don't reach the saturation number (ie 24 workers per base)
	if( currentState.stockMineral >= 50
	    &&
	    currentState.resources["Protoss_Nexus"] > 0
	    &&
	    currentState.supplyUsed < currentState.supplyCapacity
	    &&
	    currentState.mineralWorkers < currentState.numberBases * 24 )
	{
	  currentState.stockMineral -= 50;
	  ++currentState.supplyUsed;
	  --currentState.resources["Protoss_Nexus"];
	  currentState.busy.emplace_back( "Protoss_Nexus", "Protoss_Probe", 20 );
	}

	// only used by postprocessingOptimization, to see if we can
	// shorten the makespan by making more production buildings,
	// like gateways for instance.
	if( optimization )
	{
	  
	}
	
	// build a pylon if I must, ie:
	// 1. if I am not currently making pylons
	// 2. if my supply cap cannot manage the next global unit production
	if( !makingPylons() )
	  youMustConstructAdditionalPylons();

	// can I handle the next action?
	if( currentState.stockMineral >= nextAction->getCostMineral()
	    &&
	    currentState.stockGas >= nextAction->getCostGas()
	    &&
	    currentState.supplyUsed + nextAction->getCostSupply() <= currentState.supplyCapacity
	    &&
	    ( creator.empty() || currentState.resources[ creator ] > 0 )
	    &&
	    currentState.mineralWorkers + currentState.gasWorkers > 0 )
	{
	  currentState.stockMineral -= nextAction->getCostMineral();
	  currentState.stockGas -= nextAction->getCostGas();
	  currentState.supplyUsed += nextAction->getCostSupply();

	  if( !creator.empty() )
	    --currentState.resources[ creator ];

	  if( nextAction->getType() == ActionType::building )
	  {
	    currentState.inMove.emplace_back( creator, nextAction->getFullName(), nextAction->getSecondsRequired(), 5 );
	    if( currentState.mineralWorkers > 0 )
	      --currentState.mineralWorkers;
	    else
	      --currentState.gasWorkers;
	  }
	  else
	    currentState.busy.emplace_back( creator, nextAction->getFullName(), nextAction->getSecondsRequired() );

	  ++nextAction;
	  if( nextAction != vecVariables->end() )
	    creator = nextAction->getCreator();
	}
      }
    }

    return static_cast<double>( seconds );
  }

  void BuildOrderObjective::updateBusy( int seconds ) const
  {
    for( auto &t : currentState.busy )
    {
      --t.time;
      if( t.time == 0 )
      {
	if( t.actor.compare("Protoss_Probe") != 0 )
	  ++currentState.resources[ t.actor ];
	
	if( t.goal.compare("Protoss_Probe") == 0 )
	  currentState.inMove.emplace_back( "Protoss_Probe", "Mineral", 0, 2 );
	else
	{
	  if( t.goal.compare("Protoss_Nexus") == 0 )
	  {
	    ++currentState.resources["Protoss_Nexus"];
	    ++currentState.numberBases;
	  }
	  else
	    if( t.goal.compare("Protoss_Gateway") == 0 )
	      ++currentState.resources["Protoss_Gateway"];
	    else
	      if( t.goal.compare("Protoss_Cybernetics_Core") == 0 )
		++currentState.resources["Protoss_Cybernetics_Core"];
	      else
		if( t.goal.compare("Protoss_Forge") == 0 )
		  ++currentState.resources["Protoss_Forge"];
		else
		  if( t.goal.compare("Protoss_Pylon") == 0 )
		    currentState.supplyCapacity += 8;
		  else
		    if( t.goal.compare("Protoss_Assimilator") == 0 )
		    {
		      ++currentState.numberRefineries;
		      
		      // if we have few workers mining, do not sent them to gas
		      int toGas = min( 3, currentState.mineralWorkers - 3 );
		      for( int i = 0 ; i < toGas ; ++i )
		      {
			currentState.inMove.emplace_back( "Protoss_Probe", "Gas", 0, 2 );
			--currentState.mineralWorkers;
		      }
		    }
	}
	
	if( t.goal.compare("Protoss_Probe") != 0 && t.goal.compare("Protoss_Pylon") != 0 )
	{
	  bo.emplace_back( t.goal, seconds );
	  if( goals.find( t.goal ) != goals.end() )
	    ++goals.at( t.goal ).second;
	}
      }
    }

    auto itEnd = remove_if( begin( currentState.busy ), end( currentState.busy ), [](Tuple &t){return t.time == 0;} );
    currentState.busy.erase( itEnd, end( currentState.busy ) );
  }
  
  void BuildOrderObjective::updateInMove() const
  {
    for( auto &t : currentState.inMove )
    {
      --t.waitTime;
      if( t.waitTime == 0 )
      {
	if( t.actor.compare("Protoss_Probe") == 0 )
	{
	  if( t.goal.compare("Mineral") == 0 ) 
	    ++currentState.mineralWorkers;
	  else
	    if( t.goal.compare("Gas") == 0 ) 
	      ++currentState.gasWorkers;
	    else // ie, the worker is about to build something
	    {
	      currentState.busy.emplace_back( t.actor, t.goal, t.time );
	      // warp building and return to mineral fields
	      currentState.inMove.emplace_back( "Protoss_Probe", "Mineral", 0, 4 );
	    }
	}
      }
    }

    auto itEnd = remove_if( begin( currentState.inMove ), end( currentState.inMove ), [](Tuple &t){return t.waitTime == 0;} );
    currentState.inMove.erase( itEnd, end( currentState.inMove ) );
  }
  
  bool BuildOrderObjective::makingPylons() const
  {
    for( const auto &t : currentState.busy )
      if( t.goal.compare("Protoss_Pylon") == 0 )
	return true;

    for( const auto &t : currentState.inMove )
      if( t.goal.compare("Protoss_Pylon") == 0 )
	return true;    

    return false;
  }

  void BuildOrderObjective::youMustConstructAdditionalPylons() const
  {
    int supplyConsumption = currentState.resources["Protoss_Nexus"] + 2*currentState.resources["Protoss_Gateway"];
    if( supplyConsumption + currentState.supplyUsed >= currentState.supplyCapacity )
    {
      int toBuild = ( (supplyConsumption + currentState.supplyUsed - currentState.supplyCapacity) / 8 ) + 1;
      for( int i = 0 ; i < toBuild && i < currentState.mineralWorkers + currentState.gasWorkers ; ++i )
      {
	currentState.inMove.emplace_back( "Protoss_Probe", "Protoss_Pylon", 30, 5 );
	if( currentState.mineralWorkers > 0 )
	  --currentState.mineralWorkers;
	else
	  --currentState.gasWorkers;
      }
    }
  }
  
  int BuildOrderObjective::v_heuristicVariable( const vector< int > &vecId, const vector< Action > *vecVariables, BuildOrderDomain *domain )
  {
    return vecId[ randomVar.getRandNum( vecId.size() ) ];
  }

  void BuildOrderObjective::v_setHelper( const Action &b, const vector< Action > *vecVariables, const BuildOrderDomain *domain )
  {
    // int pos = b.getValue();

    // // the idea here is to favor larger values: if you have to move a
    // // variable, move it as far as possible, in order to not disturb
    // // too much what has been done so far.
    // heuristicValueHelper.at( pos ) = domain->getSize() - pos;

    heuristicValueHelper.at( b.getValue() ) = randomVar.getRandNum( 1000 );
  }


  double BuildOrderObjective::v_postprocessOptimization( vector< Action > *vecVariables, BuildOrderDomain *domain, double &bestCost ) 
  {
    chrono::time_point<chrono::high_resolution_clock> startPostprocess = chrono::high_resolution_clock::now(); 
    chrono::duration<double,micro> postprocesstimer(0);

    double optiCost = v_cost( vecVariables, domain, true );

    if( optiCost < bestCost )
    {
      bestCost = optiCost;
    }

    postprocesstimer = chrono::high_resolution_clock::now() - startPostprocess;
    return postprocesstimer.count();

    
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

    // while( (postprocesstimer = chrono::high_resolution_clock::now() - startPostprocess).count() < static_cast<int>( ceil(OPT_TIME / 100) ) && bestCost > 0 )
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
    //   auto surface = actionSameSize.equal_range( oldVariable->getSurface() );
	
    //   for( auto &it = surface.first; it != surface.second; ++it )
    //   {
    //   	mustSwap = false;
    //   	if( it->second.getId() != oldVariable->getId() )
    //   	{
    //   	  domain->swap( it->second, *oldVariable );
	    
    //   	  currentCost = v_cost( vecVariables, domain );
    //   	  if( currentCost < bestCost )
    //   	  {
    //   	    bestCost = currentCost;
    //   	    toSwap = &it->second;
    //   	    mustSwap = true;
    //   	  }

    //   	  domain->swap( it->second, *oldVariable );
    //   	}
	  
    //   	if( mustSwap )
    //   	  domain->swap( *toSwap, *oldVariable );
    //   }

    //   tabuList[ index ] = 2;//std::max(2, static_cast<int>( ceil(TABU / 2) ) );
    // }
  }

  void BuildOrderObjective::makeVecVariables( const pair<string, int> &input, vector<Action> &variables, map< string, pair<int, int> > &goals )
  {
    Action action = factoryAction( input.first );
    // Goal goal( action.getFullName(), action.getTypeString(), input.second, 0 );
    // goals.push_back( goal );
    goals.emplace( action.getFullName(), make_pair<int, int>( static_cast<int>( input.second ), 0 ) );
    
    makeVecVariables( action, variables, input.second );
  }

  void BuildOrderObjective::makeVecVariables( const Action &action, vector<Action> &variables, int count )
  {
    string name = action.getFullName();
    
    if( count > 0 )
    {
      variables.push_back( action );
      for( int i = 1; i < count; ++i )
    	variables.push_back( factoryAction( action.getFullName() ) );
      
      for( const auto &d : action.getDependencies() )
    	if( d.compare( "Protoss_High_Templar" ) == 0 || d.compare( "Protoss_Dark_Templar" ) == 0 )
    	  makeVecVariables( factoryAction( d ), variables, 2 * count ); // Each (dark) archon needs 2 (dark) templars 
    	else
    	  if( d.compare( "Protoss_Nexus" ) != 0 )
	  {
	    bool isAlreadyInVar = false;
	    for( const auto &v : variables )
	      if( d.compare( v.getFullName() ) == 0)
	      {
		isAlreadyInVar = true;
		break;
	      }

	    if( !isAlreadyInVar )
	      makeVecVariables( factoryAction( d ), variables, 1 );
	  }
    }
  }
  
  /*******************/
  /* MakeSpanMinCost */
  /*******************/
  MakeSpanMinCost::MakeSpanMinCost() : BuildOrderObjective( "MakeSpanMinCost" ) { }
  MakeSpanMinCost::MakeSpanMinCost( const vector< pair<string, int> > &input, vector<Action> &variables )
    : BuildOrderObjective( "MakeSpanMinCost", input, variables ) { }

  
  double MakeSpanMinCost::v_postprocessOptimization( vector< Action > *vecVariables, BuildOrderDomain *domain, double &bestCost )
  {

    return 0;
  }

  
  /*******************/
  /* MakeSpanMaxProd */
  /*******************/
  MakeSpanMaxProd::MakeSpanMaxProd() : BuildOrderObjective( "MakeSpanMaxProd" ) { }
  MakeSpanMaxProd::MakeSpanMaxProd( const vector< pair<string, int> > &input, vector<Action> &variables )
    : BuildOrderObjective( "MakeSpanMaxProd", input, variables ) { }

  double MakeSpanMaxProd::v_postprocessOptimization( vector< Action > *vecVariables, BuildOrderDomain *domain, double &bestCost )
  {

    return 0;
  }
}
