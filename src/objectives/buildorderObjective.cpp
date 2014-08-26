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
#include <iomanip>

#include "../../include/objectives/buildorderObjective.hpp"
#include "../../include/variables/action.hpp"
#include "../../include/misc/actionMap.hpp"

using namespace std;

namespace ghost
{
  constexpr int goToBuild = 4;		//5
  constexpr int returnToMinerals = 0;	//4
  constexpr int fromBaseToMinerals = 0; //2
  constexpr int fromMinToGas = 0;	//2
  
  
  /***********************/
  /* BuildOrderObjective */
  /***********************/
  BuildOrderObjective::BuildOrderObjective( const string &name )
    : Objective<Action, BuildOrderDomain>( name, true ),
    currentState( State() ),
    goals( map<string, pair<int, int> >() ),
    bo( vector<BO>() )
  { }
  
  BuildOrderObjective::BuildOrderObjective( const string &name,
					    const vector< pair<string, int> > &input,
					    vector<Action> &variables )
    : Objective<Action, BuildOrderDomain>( name, true ),
    currentState( State() ),
    goals( map<string, pair<int, int> >() ),
    bo( vector<BO>() )
  {
    for( const auto &i : input)
      makeVecVariables( i, variables, goals );
  }

  void BuildOrderObjective::printBO() const
  {
    cout << endl << endl;
    string text;
    for( const auto &b : bo )
    {
      cout << b.fullName
	   << ": start at " << b.startTime
	   << ", finish at " << b.completedTime << endl;
    }
    cout << endl;
  }
  
  double BuildOrderObjective::v_cost( const vector< Action > *vecVariables, const BuildOrderDomain *domain ) const
  {
    currentState.reset();
    bo.clear();
    for( auto &g : goals)
      g.second.second = 0;

    auto actionToDo = vecVariables->begin();
    
    while( actionToDo != vecVariables->end() || !currentState.busy.empty() )
    {
      ++currentState.seconds;

      // update mineral / gas stocks
      currentState.stockMineral += currentState.mineralWorkers * 1.08; // 1.08 mineral per worker per second in average
      currentState.stockGas += currentState.gasWorkers * 1.68; // 1.68 gas per worker per second in average

      // update busy list
      updateBusy();

      // update inMove list
      updateInMove();

      if( actionToDo != vecVariables->end() )
      {
	dealWithWorkers();
	
	// build a pylon if I must, ie:
	// 1. if I am not currently making pylons
	// 2. if my supply cap cannot manage the next global unit production
	if( !makingPylons() )
	  youMustConstructAdditionalPylons();

	// can I handle the current action?
	if( handleActionToDo( *actionToDo ) )
	  ++actionToDo;
	else // can I handle the next action?
	{
	  auto nextAction = actionToDo + 1;
	  if( nextAction != vecVariables->end() )
	  {
	    // book resources for the current action
	    int mineralCost = actionToDo->getCostMineral();
	    int gasCost = actionToDo->getCostGas();
	    
	    currentState.mineralsBooked += mineralCost;
	    currentState.gasBooked += gasCost;
	    if ( canHandleBuilding( *nextAction ) || canHandleNotBuilding( *nextAction ) )
	    {
	      cout << "Swap " << actionToDo->getFullName() << ":" << actionToDo->getValue()
		   << " with " << nextAction->getFullName() << ":" << nextAction->getValue() << endl;

	      std::swap( *actionToDo, *nextAction );
	      actionToDo->swapValue( *nextAction );
	      currentState.mineralsBooked -= mineralCost;
	      currentState.gasBooked -= gasCost;
	      if( handleActionToDo( *actionToDo ) )
		++actionToDo;
	      else
	      {
		cout << "This should never append." << endl;
		exit(0);
	      }	      
	    }
	    else
	    {
	      currentState.mineralsBooked -= mineralCost;
	      currentState.gasBooked -= gasCost;
	    }
	  }
	}
      }
    }
    return static_cast<double>( currentState.seconds );
  }

  double BuildOrderObjective::costOpti( vector< Action > *vecVariables ) const
  {
    currentState.reset();
    bo.clear();
    for( auto &g : goals)
      g.second.second = 0;

    vector< Action > copyVec = *vecVariables;
    
    auto actionToDo = copyVec.begin();

    while( actionToDo != copyVec.end() || !currentState.busy.empty() )
    {
      ++currentState.seconds;

      // update mineral / gas stocks
      currentState.stockMineral += currentState.mineralWorkers * 1.08; // 1.08 mineral per worker per second in average
      currentState.stockGas += currentState.gasWorkers * 1.68; // 1.68 gas per worker per second in average

      // update busy list
      updateBusy();

      // update inMove list
      updateInMove();

      if( actionToDo != copyVec.end() )
      {
	dealWithWorkers();
	
	// only used by postprocessingOptimization, to see if we can
	// shorten the makespan by making more production buildings,
	// like gateways for instance.
	ActionData action, creator;
	double real_time, simulated_time;
	int simulated_mineral, simulated_gas;
	int future_mineral, future_gas;
	  
	int to_produce;
	int creator_in_production;
	int totalNumber;
	for( const auto &g : goals )
	{
	  action = actionOf[g.first];
	  if( action.actionType == ActionType::building )
	    continue;

	  if( action.name.compare("Protoss_Archon") == 0 || action.name.compare("Protoss_Dark_Archon") == 0 )
	    continue;
	    
	  creator = actionOf[ action.creator ];

	  creator_in_production =
	    count_if( begin(currentState.inMove),
		      end(currentState.inMove),
		      [&creator](Tuple &t){return t.action.name.compare( creator.name ) == 0;})
	    + count_if( begin(currentState.busy),
			end(currentState.busy),
			[&creator](ActionData &a){return a.name.compare( creator.name ) == 0;});

	  totalNumber = currentState.resources[creator.name].first + creator_in_production;
	    
	  if( totalNumber == 0 )
	    continue;

	  // test is we are faster after making an additional production building
	  to_produce = g.second.first - g.second.second;
	  real_time = 0.;
	    
	  for( const auto &t : currentState.busy )
	    if( t.name.compare( action.name ) == 0 )
	    {
	      --to_produce;
	      real_time += t.secondsRequired;
	    }

	  if( to_produce <= 0 )
	    continue;

	  simulated_time = real_time + creator.secondsRequired;
	  real_time += to_produce * action.secondsRequired / ( currentState.resources[creator.name].first + creator_in_production );
	  simulated_time += to_produce * action.secondsRequired / ( currentState.resources[creator.name].first + creator_in_production + 1 );

	  if( simulated_time > real_time )
	    continue;

	  // test is we have enough money for making an additional production building
	  simulated_mineral = ( currentState.resources[creator.name].first + creator_in_production + 1 ) * action.costMineral;
	  simulated_gas = ( currentState.resources[creator.name].first + creator_in_production + 1 ) * action.costGas;

	  future_mineral = sharpMineralsIn( action.secondsRequired, creator.secondsRequired );
	  future_gas = sharpGasIn( action.secondsRequired, creator.secondsRequired );

	  // if we can make this additional building, do it! 
	  if( future_mineral >= simulated_mineral && future_gas >= simulated_gas
	      &&
	      ( creator.costMineral == 0 || currentState.stockMineral >= creator.costMineral + currentState.mineralsBooked - mineralsIn(goToBuild) )
	      &&
	      ( creator.costGas == 0 || currentState.stockGas >= creator.costGas + currentState.gasBooked - gasIn(goToBuild) ) 
	      &&
	      currentState.mineralWorkers + currentState.gasWorkers > 0
	      &&
	      currentState.numberPylons > 0
	      )
	  {
	    currentState.mineralsBooked += creator.costMineral;
	    currentState.gasBooked += creator.costGas;

#ifndef NDEBUG
	    string text = "Optimize " + creator.name + " at ";
	    cout << std::left << setw(35) << text << setw(5) << currentState.seconds
	    	 << "  m = " << setw(9) << currentState.stockMineral
	    	 << "  g = " << setw(8) << currentState.stockGas
	    	 << "  mb = " << setw(5) << currentState.mineralsBooked
	    	 << "  gb = " << setw(4) << currentState.gasBooked
	    	 << "  mw = " << setw(3) << currentState.mineralWorkers
	    	 << "  gw = " << setw(3) << currentState.gasWorkers
	    	 << "  s = " << currentState.supplyUsed << "/" << currentState.supplyCapacity << ")" << endl;
#endif
	    
	    auto it_find = std::find( vecVariables->begin(), vecVariables->end(), *actionToDo );
	    auto it = vecVariables->insert( it_find, Action( creator, it_find->getValue() ) );
	    std::for_each( it+1, vecVariables->end(), [](Action &a){a.shiftValue();} );
	      
	    currentState.inMove.push_back( Tuple( creator, goToBuild ) );
	    if( currentState.mineralWorkers > 0 )
	      --currentState.mineralWorkers;
	    else
	      --currentState.gasWorkers;
	  }	    
	}	  
	
	// build a pylon if I must, ie:
	// 1. if I am not currently making pylons
	// 2. if my supply cap cannot manage the next global unit production
	if( !makingPylons() )
	  youMustConstructAdditionalPylons();

	// can I handle the next action?
	if( handleActionToDo( *actionToDo ) )
	  ++actionToDo;
      }
    }
    return static_cast<double>( currentState.seconds );
  }

  void BuildOrderObjective::updateBusy() const
  {
    for( auto &t : currentState.busy )
    {
      int time = t.decreaseSeconds();
      if( time == 0 )
      {
	if( t.creator.compare("Protoss_Probe") != 0 )
	{
	  ++currentState.resources[ t.creator ].second;
	}
	if( t.name.compare("Protoss_Probe") == 0 )
	  currentState.inMove.push_back( Tuple( actionOf["Protoss_Mineral"], fromBaseToMinerals ) );
	else
	{
	  if( t.name.compare("Protoss_Nexus") == 0 )
	  {
	    ++currentState.resources["Protoss_Nexus"].first;
	    ++currentState.resources["Protoss_Nexus"].second;
	    ++currentState.numberBases;
	  }
	  else if( t.name.compare("Protoss_Pylon") == 0 )
	  {
	    currentState.supplyCapacity += 8;
	    ++currentState.numberPylons;
	  }
	  else if( t.name.compare("Protoss_Assimilator") == 0 )
	  {
	    ++currentState.numberRefineries;
	    
	    // if we have few workers mining, do not sent them to gas
	    for( int i = 0 ; i < min( 3, currentState.mineralWorkers - 3 ) ; ++i )
	    {
	      currentState.inMove.push_back( Tuple( actionOf["Protoss_Gas"], fromMinToGas ) );
	      --currentState.mineralWorkers;
	    }
	  }
	  else if( t.actionType == ActionType::building )
	  {
	    ++currentState.resources[ t.name ].first;
	    ++currentState.resources[ t.name ].second;
	    currentState.inMove.push_back( Tuple( actionOf["Protoss_Mineral"], returnToMinerals ) );
	  }
	  else if( t.name.compare("Protoss_High_Templar") == 0
		   || t.name.compare("Protoss_Dark_Templar") == 0 )
	  {
	    ++currentState.resources[ t.name ].first;
	    ++currentState.resources[ t.name ].second;
	  }
	}

#ifndef NDEBUG
	string text = "Finish " + t.name + " at ";
	cout << std::left << setw(35) << text << setw(5) << currentState.seconds
	     << "  m = " << setw(9) << currentState.stockMineral
	     << "  g = " << setw(8) << currentState.stockGas
	     << "  mb = " << setw(5) << currentState.mineralsBooked
	     << "  gb = " << setw(4) << currentState.gasBooked
	     << "  mw = " << setw(3) << currentState.mineralWorkers
	     << "  gw = " << setw(3) << currentState.gasWorkers
	     << "  s = " << currentState.supplyUsed << "/" << currentState.supplyCapacity << ")" << endl;
#endif
      }
    }

    auto itEnd = remove_if( begin( currentState.busy ), end( currentState.busy ), [](ActionData &a){return a.secondsRequired == 0;} );
    currentState.busy.erase( itEnd, end( currentState.busy ) );
  }
  
  void BuildOrderObjective::updateInMove() const
  {
    for( auto &t : currentState.inMove )
    {
      if( t.waitTime > 0 )
	--t.waitTime;

      if( t.waitTime == 0
	  && !t.done
	  && ( t.action.costMineral == 0 || currentState.stockMineral >= t.action.costMineral )
	  && ( t.action.costGas == 0 || currentState.stockGas >= t.action.costGas )
	)
      {
	string creator = t.action.creator;
	string goal = t.action.name;

	int mineralCost = t.action.costMineral;
	int gasCost = t.action.costGas;

	if( creator.compare("Protoss_Probe") == 0 )
	{
	  if( goal.compare("Mineral") == 0 ) 
	    ++currentState.mineralWorkers;
	  else
	    if( goal.compare("Gas") == 0 ) 
	      ++currentState.gasWorkers;
	    else // ie, the worker is about to build something
	    {
	      pushInBusy( t.action.name );
	      // warp building and return to mineral fields
	      currentState.inMove.push_back( Tuple( actionOf["Protoss_Mineral"], returnToMinerals ) );
	      
	      currentState.stockMineral -= mineralCost;
	      currentState.stockGas -= gasCost;

	      currentState.mineralsBooked -= mineralCost;
	      currentState.gasBooked -= gasCost;

#ifndef NDEBUG
	      string text = "Start " + goal + " at ";
	      cout << std::left << setw(35) << text << setw(5) << currentState.seconds
	      	   << "  m = " << setw(9) << currentState.stockMineral
	      	   << "  g = " << setw(8) << currentState.stockGas
	      	   << "  mb = " << setw(5) << currentState.mineralsBooked
	      	   << "  gb = " << setw(4) << currentState.gasBooked
	      	   << "  mw = " << setw(3) << currentState.mineralWorkers
	      	   << "  gw = " << setw(3) << currentState.gasWorkers
	      	   << "  s = " << currentState.supplyUsed << "/" << currentState.supplyCapacity << ")" << endl;
#endif
	    }

	  t.done = true;
	  
	}
      }
    }

    auto itEnd = remove_if( begin( currentState.inMove ), end( currentState.inMove ), [](Tuple &t){return t.done;} );
    currentState.inMove.erase( itEnd, end( currentState.inMove ) );
  }

  void BuildOrderObjective::dealWithWorkers() const
  {
    // send workers to gas, if need and possible
    if( currentState.gasWorkers + count_if( begin(currentState.inMove),
					    end(currentState.inMove),
					    [](Tuple &t){return t.action.name.compare("Gas") == 0;} )
	< currentState.numberRefineries * 3 )
    {
      // if we have few workers mining, do not sent them to gas
      Tuple tuple_gas( actionOf["Protoss_Gas"], fromMinToGas );
      for( int i = 0 ; i < min( 3, currentState.mineralWorkers - 3 ) ; ++i )
      {
	currentState.inMove.push_back( tuple_gas );
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
	currentState.resources["Protoss_Nexus"].second > 0
	&&
	currentState.supplyUsed < currentState.supplyCapacity
	&&
	currentState.mineralWorkers + count_if( begin(currentState.inMove),
						end(currentState.inMove),
						[](const Tuple &t){return t.action.creator.compare("Protoss_Probe") == 0;})
	< currentState.numberBases * 24 )
    {
      currentState.stockMineral -= 50;
      ++currentState.supplyUsed;
      --currentState.resources["Protoss_Nexus"].second;
      pushInBusy( "Protoss_Probe" );
      
#ifndef NDEBUG
      cout << std::left << setw(35) << "Start Protoss_Probe at " << setw(5) << currentState.seconds
      	   << "  m = " << setw(9) << currentState.stockMineral
      	   << "  g = " << setw(8) << currentState.stockGas
      	   << "  mb = " << setw(5) << currentState.mineralsBooked
      	   << "  gb = " << setw(4) << currentState.gasBooked
      	   << "  mw = " << setw(3) << currentState.mineralWorkers
      	   << "  gw = " << setw(3) << currentState.gasWorkers
      	   << "  s = " << currentState.supplyUsed << "/" << currentState.supplyCapacity << ")" << endl;
#endif
    }
  }

  bool BuildOrderObjective::canHandleBuilding( const Action &actionToDo ) const
  {
    if( actionToDo.getType() != ActionType::building )
      return false;

    if( ( actionToDo.getCostMineral() == 0 || currentState.stockMineral >= actionToDo.getCostMineral() + currentState.mineralsBooked - mineralsIn(goToBuild) )
	&&
	( actionToDo.getCostGas() == 0 || currentState.stockGas >= actionToDo.getCostGas() + currentState.gasBooked - gasIn(goToBuild) ) 
	&&
	currentState.mineralWorkers + currentState.gasWorkers > 0
	&&
	currentState.numberPylons > 0
	&&
	dependenciesCheck( actionToDo.getFullName() )
      )
    {
      return true;
    }
    else
      return false;
  }
  
  bool BuildOrderObjective::canHandleNotBuilding( const Action &actionToDo ) const
  {
    if( actionToDo.getType() == ActionType::building )
      return false;

    string creator = actionToDo.getCreator();
        
    if( ( actionToDo.getCostMineral() == 0 || currentState.stockMineral >= actionToDo.getCostMineral() + currentState.mineralsBooked )
	&&
	( actionToDo.getCostGas() == 0 || currentState.stockGas >= actionToDo.getCostGas() + currentState.gasBooked )
	&&
	currentState.supplyUsed + actionToDo.getCostSupply() <= currentState.supplyCapacity
	&&
	( creator.empty() || currentState.resources[ creator ].second > 0 )
	&&
	dependenciesCheck( actionToDo.getFullName() )
      )
    {
      return true;
    }
    else
      return false;
  }

  bool BuildOrderObjective::handleActionToDo( const Action &actionToDo ) const
  {
    // if the next action is building a building
    if( actionToDo.getType() == ActionType::building )
    {
      if( canHandleBuilding( actionToDo ) )
      {
	currentState.mineralsBooked += actionToDo.getCostMineral();
	currentState.gasBooked += actionToDo.getCostGas();
	
#ifndef NDEBUG
	string text = "Go for " + actionToDo.getFullName() + " at ";
	cout << std::left << setw(35) << text << setw(5) << currentState.seconds
	     << "  m = " << setw(9) << currentState.stockMineral
	     << "  g = " << setw(8) << currentState.stockGas
	     << "  mb = " << setw(5) << currentState.mineralsBooked
	     << "  gb = " << setw(4) << currentState.gasBooked
	     << "  mw = " << setw(3) << currentState.mineralWorkers
	     << "  gw = " << setw(3) << currentState.gasWorkers
	     << "  s = " << currentState.supplyUsed << "/" << currentState.supplyCapacity << ")" << endl;
#endif
	
	currentState.inMove.push_back( Tuple( actionToDo.getData(), goToBuild ) );
	if( currentState.mineralWorkers > 0 )
	  --currentState.mineralWorkers;
	else
	  --currentState.gasWorkers;
	
	return true;
      }
    }
    // otherwise, it is a unit/research/upgrade
    else
    {
      string creator = actionToDo.getCreator();

      if( canHandleNotBuilding( actionToDo ) )
      {
#ifndef NDEBUG
	string text = "Start " + actionToDo.getFullName() + " at ";
	cout << std::left << setw(35) << text << setw(5) << currentState.seconds
	     << "  m = " << setw(9) << currentState.stockMineral
	     << "  g = " << setw(8) << currentState.stockGas
	     << "  mb = " << setw(5) << currentState.mineralsBooked
	     << "  gb = " << setw(4) << currentState.gasBooked
	     << "  mw = " << setw(3) << currentState.mineralWorkers
	     << "  gw = " << setw(3) << currentState.gasWorkers
	     << "  s = " << currentState.supplyUsed << "/" << currentState.supplyCapacity << ")" << endl;
#endif
	
	currentState.supplyUsed += actionToDo.getCostSupply();
	currentState.stockMineral -= actionToDo.getCostMineral();
	currentState.stockGas -= actionToDo.getCostGas();
	
	if( !creator.empty() && creator.compare("Protoss_Probe") != 0 )
	  --currentState.resources[ creator ].second;
	
	pushInBusy( actionToDo.getFullName() );
	
	return true;
      }
    }

    return false;
  }
  
  bool BuildOrderObjective::makingPylons() const
  {
    for( const auto &t : currentState.busy )
      if( t.name.compare("Protoss_Pylon") == 0 )
	return true;

    for( const auto &t : currentState.inMove )
      if( t.action.name.compare("Protoss_Pylon") == 0 )
	return true;    

    return false;
  }

  void BuildOrderObjective::youMustConstructAdditionalPylons() const
  {
    // build the first pylon ASAP
    if( currentState.numberPylons == 0
	&& currentState.stockMineral >= 100 - mineralsIn( returnToMinerals ) )
    {
      currentState.inMove.push_back( Tuple( actionOf["Protoss_Pylon"], goToBuild ) );

      currentState.mineralsBooked += 100;
      
#ifndef NDEBUG
      cout << std::left << setw(35) << "Go for first Protoss_Pylon at " << setw(5) << currentState.seconds
      	   << "  m = " << setw(9) << currentState.stockMineral
      	   << "  g = " << setw(8) << currentState.stockGas
      	   << "  mb = " << setw(5) << currentState.mineralsBooked
      	   << "  gb = " << setw(4) << currentState.gasBooked
      	   << "  mw = " << setw(3) << currentState.mineralWorkers
      	   << "  gw = " << setw(3) << currentState.gasWorkers
      	   << "  s = " << currentState.supplyUsed << "/" << currentState.supplyCapacity << ")" << endl;
#endif
      
      if( currentState.mineralWorkers > 0 )
	--currentState.mineralWorkers;
      else
	--currentState.gasWorkers;
    }
    // otherwise build other pylons when needed
    else
    {
      int productionCapacity =
	currentState.resources["Protoss_Nexus"].first
	+ count_if( begin(currentState.busy), end(currentState.busy), [](ActionData &a){return a.name.compare( "Protoss_Nexus" ) == 0;})

	+ 2 * ( currentState.resources["Protoss_Gateway"].first
		+ count_if( begin(currentState.busy), end(currentState.busy), [](ActionData &a){return a.name.compare( "Protoss_Gateway" ) == 0;}) )
	+ 4 * ( currentState.resources["Protoss_Robotics_Facility"].first
		+ count_if( begin(currentState.busy), end(currentState.busy), [](ActionData &a){return a.name.compare( "Protoss_Robotics_Facility" ) == 0;}) )
	+ 6 * ( currentState.resources["Protoss_Stargate"].first
		+ count_if( begin(currentState.busy), end(currentState.busy), [](ActionData &a){return a.name.compare( "Protoss_Stargate" ) == 0;}) );

      int plannedSupply = currentState.supplyCapacity
	+ 8 * count_if( begin(currentState.busy), end(currentState.busy), [](ActionData &a){return a.name.compare( "Protoss_Pylon" ) == 0;} )
	+ 8 * count_if( begin(currentState.inMove), end(currentState.inMove), [](Tuple &t){return t.action.name.compare("Protoss_Pylon") == 0;} )
	+ 9 * count_if( begin(currentState.busy), end(currentState.busy), [](ActionData &a){return a.name.compare( "Protoss_Nexus" ) == 0;} )
	+ 9 * count_if( begin(currentState.inMove), end(currentState.inMove), [](Tuple &t){return t.action.name.compare("Protoss_Nexus") == 0;} );
     
      if( plannedSupply <= productionCapacity + currentState.supplyUsed )
      {
	currentState.inMove.push_back( Tuple( actionOf["Protoss_Pylon"], goToBuild ) );
	currentState.mineralsBooked += 100;

#ifndef NDEBUG
	cout << std::left << setw(35) << "Go for Protoss_Pylon at " << setw(5) << currentState.seconds
	     << "  m = " << setw(9) << currentState.stockMineral
	     << "  g = " << setw(8) << currentState.stockGas
	     << "  mb = " << setw(5) << currentState.mineralsBooked
	     << "  gb = " << setw(4) << currentState.gasBooked
	     << "  mw = " << setw(3) << currentState.mineralWorkers
	     << "  gw = " << setw(3) << currentState.gasWorkers
	     << "  s = " << currentState.supplyUsed << "/" << currentState.supplyCapacity << ")" << endl;
#endif
	
	if( currentState.mineralWorkers > 0 )
	  --currentState.mineralWorkers;
	else
	  --currentState.gasWorkers;
      }
    }
  }

  void BuildOrderObjective::pushInBusy( const string  &name ) const
  {
    ActionData a  = actionOf[ name ];
    currentState.busy.push_back( a );
    
    // if( goal.compare("Protoss_Probe") != 0 && goal.compare("Protoss_Pylon") != 0 )
    // {
    bo.emplace_back( a.name, currentState.seconds, currentState.seconds + a.secondsRequired );
    if( goals.find( a.name ) != goals.end() )
      ++goals.at( a.name ).second;
    // }
  }

  bool BuildOrderObjective::dependenciesCheck( const string &s ) const
  {
    ActionData data = actionOf[ s ]; 
    if( data.costGas > 0 && currentState.numberRefineries == 0)
      return false;

    if( any_of( begin(data.dependencies), end(data.dependencies), [&](const string &n){return currentState.resources[ n ].first == 0;} ) )
      return false;

    return true;
  }

  double BuildOrderObjective::sharpMineralsIn( int duration, int InSeconds ) const
  {
    double futurProduction = 0.;
    int workers = currentState.mineralWorkers;

    int min_time = std::min( InSeconds, 20 );
    vector<int> last_build;

    // simulation time from now till InSeconds
    for( int i = 1 ; i <= min_time ; ++i )
    {
      for( const auto &t : currentState.inMove )
	if( t.action.creator.compare("Protoss_Probe") == 0
	    &&
	    t.action.name.compare("Mineral") == 0
	    &&
	    t.waitTime - i == 0 )
	{
	  ++workers;
	}
      
      for( const auto &t : currentState.busy )
	if( t.name.compare("Protoss_Probe") == 0
	    &&
	    t.secondsRequired + 2 - i == 0 )
	{
	  ++workers;
	  last_build.push_back(i);
	}
    }

    for( int i = min_time + 1 ; i <= InSeconds ; ++i )
    {
      for( const auto &l : last_build )
	if( ( i + 2 - l ) % 20 == 0 )
	  ++workers;
    }

    // start to count income from InSeconds till InSeconds + duration
    for( int i = InSeconds + 1 ; i <= InSeconds + duration ; ++i )
    {
      for( const auto &l : last_build )
	if( ( i + 2 - l ) % 20 == 0 )
	  ++workers;
      
      futurProduction += workers * 1.08;
    }
    
    return futurProduction;
  }
  
  double BuildOrderObjective::sharpGasIn( int duration, int InSeconds ) const
  {
    double futurProduction = 0.;
    int workers = currentState.gasWorkers;

    // simulation time from now till InSeconds
    for( int i = 1 ; i <= InSeconds ; ++i )
    {
      for( const auto &t : currentState.inMove )
	if( t.action.creator.compare("Protoss_Probe") == 0
	    &&
	    t.action.name.compare("Gas") == 0
	    &&
	    t.waitTime - i == 0 )
	{
	  ++workers;
	}
    }

    // start to count income from InSeconds till InSeconds + duration
    for( int i = InSeconds + 1 ; i <= InSeconds + duration ; ++i )
      futurProduction += workers * 1.08;

    return futurProduction;
  }

  int BuildOrderObjective::v_heuristicVariable( const vector< int > &vecId, const vector< Action > *vecVariables, BuildOrderDomain *domain )
  {
    return vecId[ randomVar.getRandNum( vecId.size() ) ];
  }

  int BuildOrderObjective::v_heuristicValue( const std::vector< double > &vecGlobalCosts, 
					     double &bestEstimatedCost,
					     int &bestValue ) const
  {
    int best = 0;
    // double bestHelp = numeric_limits<int>::max();

    for( int i = 0; i < vecGlobalCosts.size(); ++i )
    {
      // if( vecGlobalCosts[i] < bestEstimatedCost
      // 	  || ( vecGlobalCosts[i] == bestEstimatedCost
      // 	       && vecGlobalCosts[i] < numeric_limits<int>::max()
      // 	       && heuristicValueHelper.at( i ) < bestHelp ) )
      if( vecGlobalCosts[i] < bestEstimatedCost )
      {
	bestEstimatedCost = vecGlobalCosts[i];
	bestValue = i;
	best = i;
      }
    }
    
    return best;
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

  double BuildOrderObjective::v_postprocessSatisfaction( vector< Action > *vecVariables,
							 BuildOrderDomain *domain,
							 double &bestCost,
							 vector< Action > &bestSolution,
							 double sat_timeout ) const
  {
    chrono::time_point<chrono::high_resolution_clock> startPostprocess = chrono::high_resolution_clock::now(); 
    chrono::duration<double,micro> postprocesstimer(0);

    double cost = v_cost( vecVariables, domain );
    
    if( cost < bestCost )
    {
      bestCost = cost;
      copy( begin(*vecVariables), end(*vecVariables), begin(bestSolution) );
    }

    postprocesstimer = chrono::high_resolution_clock::now() - startPostprocess;
    return postprocesstimer.count();
  }


  double BuildOrderObjective::v_postprocessOptimization( vector< Action > *vecVariables,
							 BuildOrderDomain *domain,
							 double &bestCost,
							 double opt_timeout ) 
  {
    chrono::time_point<chrono::high_resolution_clock> startPostprocess = chrono::high_resolution_clock::now(); 
    chrono::duration<double,micro> postprocesstimer(0);

    double optiCost = costOpti( vecVariables );

    if( optiCost < bestCost )
    {
      bestCost = optiCost;
    }

    bestCost = costOpti( vecVariables );
    // printBO();

    postprocesstimer = chrono::high_resolution_clock::now() - startPostprocess;
    return postprocesstimer.count();
  }

  void BuildOrderObjective::makeVecVariables( const pair<string, int> &input, vector<Action> &variables, map< string, pair<int, int> > &goals )
  {
    Action action( actionOf[input.first] );
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
    	variables.push_back( Action( actionOf[ action.getFullName() ] ) );
      
      for( const auto &d : action.getDependencies() )
    	if( d.compare( "Protoss_High_Templar" ) == 0 || d.compare( "Protoss_Dark_Templar" ) == 0 )
    	  makeVecVariables( Action( actionOf[ d ] ), variables, 2 * count ); // Each (dark) archon needs 2 (dark) templars 
    	else
    	  if( d.compare( "Protoss_Nexus" ) != 0 )
	  {
	    if( none_of( begin(variables), end(variables), [&d](Action &a){return d.compare(a.getFullName()) == 0;} ) )
	      makeVecVariables( Action( actionOf[ d ] ), variables, 1 );
	  }

      if( action.getCostGas() > 0 )
      {
	if( none_of( begin(variables), end(variables), [](Action &a){return a.getFullName().compare("Protoss_Assimilator") == 0;} ) )
	{
	  makeVecVariables( Action( actionOf[ "Protoss_Assimilator" ] ), variables, 1 );
	}
      }
    }
  }
  
  /*******************/
  /* MakeSpanMinCost */
  /*******************/
  MakeSpanMinCost::MakeSpanMinCost() : BuildOrderObjective( "MakeSpanMinCost" ) { }
  MakeSpanMinCost::MakeSpanMinCost( const vector< pair<string, int> > &input, vector<Action> &variables )
    : BuildOrderObjective( "MakeSpanMinCost", input, variables ) { }

  
  // double MakeSpanMinCost::v_postprocessOptimization( vector< Action > *vecVariables, BuildOrderDomain *domain, double &bestCost )
  // {

  //   return 0;
  // }

  
  /*******************/
  /* MakeSpanMaxProd */
  /*******************/
  MakeSpanMaxProd::MakeSpanMaxProd() : BuildOrderObjective( "MakeSpanMaxProd" ) { }
  MakeSpanMaxProd::MakeSpanMaxProd( const vector< pair<string, int> > &input, vector<Action> &variables )
    : BuildOrderObjective( "MakeSpanMaxProd", input, variables ) { }

  // double MakeSpanMaxProd::v_postprocessOptimization( vector< Action > *vecVariables, BuildOrderDomain *domain, double &bestCost )
  // {

  //   return 0;
  // }
}
