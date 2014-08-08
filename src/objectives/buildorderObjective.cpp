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
  /***********************/
  /* BuildOrderObjective */
  /***********************/
  BuildOrderObjective::BuildOrderObjective( const string &name )
    : Objective<Action, BuildOrderDomain>( name, true ),
    currentState( State() ),
    // goals( vector<Goal>() ),
    goals( map<string, pair<int, int> >() ),
    bo( vector<BO>() )
  { }
  
  BuildOrderObjective::BuildOrderObjective( const string &name,
					    const vector< pair<string, int> > &input,
					    vector<Action> &variables )
    : Objective<Action, BuildOrderDomain>( name, true ),
    currentState( State() ),
    // goals( vector<Goal>() ),
    goals( map<string, pair<int, int> >() ),
    bo( vector<BO>() )
  {
    for( const auto &i : input)
      makeVecVariables( i, variables, goals );
  }

  void BuildOrderObjective::printBO() const
  {
    for( const auto &b : bo )
      cout << b.fullName << ":" << b.completedTime << endl;
    cout << endl;
  }
  
  double BuildOrderObjective::v_cost( const vector< Action > *vecVariables, const BuildOrderDomain *domain ) const
  {
    return v_cost( vecVariables, domain, false );
  }
  
  double BuildOrderObjective::v_cost( const vector< Action > *vecVariables, const BuildOrderDomain *domain, bool optimization ) const
  {
    currentState.reset();
    bo.clear();
    auto nextAction = vecVariables->begin();
    string creator = nextAction->getCreator();

    cout << endl << endl;
    
    while( nextAction != vecVariables->end() || !currentState.busy.empty() )
    {
      ++currentState.seconds;

      // update mineral / gas stocks
      currentState.stockMineral += currentState.mineralWorkers * 1.08; // 1.08 mineral per worker per second in average
      currentState.stockGas += currentState.gasWorkers * 1.68; // 1.68 gas per worker per second in average

      // cout << "[" << currentState.seconds << "] current minerals: " << currentState.stockMineral << endl;
      // cout << "[" << currentState.seconds << "] current gas: " << currentState.stockGas << endl;
      
      // update busy list
      updateBusy();

      // update inMove list
      updateInMove();

      if( nextAction != vecVariables->end() )
      {
	// send workers to gas, if need and possible
	int comingForGas = count_if( begin(currentState.inMove), end(currentState.inMove), [](Tuple &t){return t.action.name.compare("Gas") == 0;} );
	if( currentState.gasWorkers + comingForGas < currentState.numberRefineries * 3 )
	{
	  // if we have few workers mining, do not sent them to gas
	  int toGas = min( 3, currentState.mineralWorkers - 3 );
	  Tuple tuple_gas( actionOf["Protoss_Gas"], 2, false );
	  for( int i = 0 ; i < toGas ; ++i )
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
	    currentState.resources["Protoss_Nexus"] > 0
	    &&
	    currentState.supplyUsed < currentState.supplyCapacity
	    &&
	    currentState.mineralWorkers < currentState.numberBases * 24 )
	{
	  currentState.stockMineral -= 50;
	  ++currentState.supplyUsed;
	  --currentState.resources["Protoss_Nexus"];
	  currentState.busy.push_back( actionOf["Protoss_Probe"] );

	  cout << std::left << setw(35) << "Start Protoss_Probe at " << setw(5) << currentState.seconds
	       << ",\t m = " << setw(9) << currentState.stockMineral
	       << ",\t g = " << setw(8) << currentState.stockGas
	       << ",\t mb = " << setw(5) << currentState.mineralsBooked
	       << ",\t gb = " << setw(4) << currentState.gasBooked
	       << ",\t mw = " << setw(3) << currentState.mineralWorkers
	       << ",\t gw = " << setw(3) << currentState.gasWorkers
	       << ",\t s = " << currentState.supplyUsed << "/" << currentState.supplyCapacity << ")" << endl;
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

	// cout << "[" << currentState.seconds << "] minerals in 5 seconds with " << currentState.mineralWorkers << " workers: " << mineralsIn(5) << endl;
	// cout << "[" << currentState.seconds << "] gas in 5 seconds with " << currentState.gasWorkers << " workers: " << gasIn(5) << endl;
	
	// can I handle the next action?
	// if the next action is building a building
	if( nextAction->getType() == ActionType::building )
	{
	  if( ( nextAction->getCostMineral() == 0 || currentState.stockMineral >= nextAction->getCostMineral() + currentState.mineralsBooked - mineralsIn(5) )
	      &&
	      ( nextAction->getCostGas() == 0 || currentState.stockGas >= nextAction->getCostGas() + currentState.gasBooked - gasIn(5) ) 
	      &&
	      currentState.mineralWorkers + currentState.gasWorkers > 0
	      &&
	      currentState.numberPylons > 0
	      &&
	      currentState.canBuild[ nextAction->getFullName() ]
	    )
	  {
	    currentState.mineralsBooked += nextAction->getCostMineral();
	    currentState.gasBooked += nextAction->getCostGas();

	    string text = "Go for " + nextAction->getFullName() + " at ";
	      cout << std::left << setw(35) << text << setw(5) << currentState.seconds
		 << ",\t m = " << setw(9) << currentState.stockMineral
		 << ",\t g = " << setw(8) << currentState.stockGas
		 << ",\t mb = " << setw(5) << currentState.mineralsBooked
		 << ",\t gb = " << setw(4) << currentState.gasBooked
		 << ",\t mw = " << setw(3) << currentState.mineralWorkers
		 << ",\t gw = " << setw(3) << currentState.gasWorkers
		 << ",\t s = " << currentState.supplyUsed << "/" << currentState.supplyCapacity << ")" << endl;
	    
	    currentState.inMove.push_back( Tuple( nextAction->getData(), 5, false ) );
	    if( currentState.mineralWorkers > 0 )
	      --currentState.mineralWorkers;
	    else
	      --currentState.gasWorkers;
	  
	  ++nextAction;
	  if( nextAction != vecVariables->end() )
	    creator = nextAction->getCreator();
	  }
	}
	// otherwise, it is a unit/research/upgrade
	else
	{
	  if( ( nextAction->getCostMineral() == 0 || currentState.stockMineral >= nextAction->getCostMineral() + currentState.mineralsBooked )
	      &&
	      ( nextAction->getCostGas() == 0 || currentState.stockGas >= nextAction->getCostGas() + currentState.gasBooked )
	      &&
	      currentState.supplyUsed + nextAction->getCostSupply() <= currentState.supplyCapacity
	      &&
	      ( creator.empty() || currentState.resources[ creator ] > 0 )
	      &&
	      currentState.canBuild[ nextAction->getFullName() ]
	    )
	  {
	    string text = "Start " + nextAction->getFullName() + " at ";
	      cout << std::left << setw(35) << text << setw(5) << currentState.seconds
		 << ",\t m = " << setw(9) << currentState.stockMineral
		 << ",\t g = " << setw(8) << currentState.stockGas
		 << ",\t mb = " << setw(5) << currentState.mineralsBooked
		 << ",\t gb = " << setw(4) << currentState.gasBooked
		 << ",\t mw = " << setw(3) << currentState.mineralWorkers
		 << ",\t gw = " << setw(3) << currentState.gasWorkers
		 << ",\t s = " << currentState.supplyUsed << "/" << currentState.supplyCapacity << ")" << endl;
	    
	    currentState.supplyUsed += nextAction->getCostSupply();
	    currentState.stockMineral -= nextAction->getCostMineral();
	    currentState.stockGas -= nextAction->getCostGas();

	    if( !creator.empty() && creator.compare("Protoss_Probe") != 0 )
	      --currentState.resources[ creator ];
	    
	    currentState.busy.push_back( nextAction->getData() );
	    
	    ++nextAction;
	    if( nextAction != vecVariables->end() )
	      creator = nextAction->getCreator();
	  }
	}
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
	  ++currentState.resources[ t.creator ];
	
	if( t.name.compare("Protoss_Probe") == 0 )
	  currentState.inMove.push_back( Tuple( actionOf["Protoss_Mineral"], 2, false ) );
	else
	{
	  if( t.name.compare("Protoss_Nexus") == 0 )
	  {
	    ++currentState.resources["Protoss_Nexus"];
	    ++currentState.numberBases;
	  }
	  else
	    if( t.name.compare("Protoss_Gateway") == 0 )
	    {
	      ++currentState.resources["Protoss_Gateway"];
	      currentState.canBuild["Protoss_Zealot"] = true;
	      currentState.canBuild["Protoss_Cybernetics_Core"] = true;
	    }
	    else
	      if( t.name.compare("Protoss_Cybernetics_Core") == 0 )
	      {
		++currentState.resources["Protoss_Cybernetics_Core"];
		currentState.canBuild["Protoss_Dragoon"] = true;
	      }
	      else
		if( t.name.compare("Protoss_Forge") == 0 )
		{
		  ++currentState.resources["Protoss_Forge"];
		  currentState.canBuild["Protoss_Ground_Weapons_1"] = true;
		}
		else
		  if( t.name.compare("Protoss_Pylon") == 0 )
		  {
		    currentState.supplyCapacity += 8;
		    ++currentState.numberPylons;
		  }
		  else
		    if( t.name.compare("Protoss_Assimilator") == 0 )
		    {
		      ++currentState.numberRefineries;
		      
		      // if we have few workers mining, do not sent them to gas
		      int toGas = min( 3, currentState.mineralWorkers - 3 );
		      for( int i = 0 ; i < toGas ; ++i )
		      {
			currentState.inMove.push_back( Tuple( actionOf["Protoss_Gas"], 2, false ) );
			--currentState.mineralWorkers;
		      }
		    }
	}

	string text = "Finish " + t.name + " at ";
	cout << std::left << setw(35) << text << setw(5) << currentState.seconds
	     << ",\t m = " << setw(9) << currentState.stockMineral
	     << ",\t g = " << setw(8) << currentState.stockGas
	     << ",\t mb = " << setw(5) << currentState.mineralsBooked
	     << ",\t gb = " << setw(4) << currentState.gasBooked
	     << ",\t mw = " << setw(3) << currentState.mineralWorkers
	     << ",\t gw = " << setw(3) << currentState.gasWorkers
	     << ",\t s = " << currentState.supplyUsed << "/" << currentState.supplyCapacity << ")" << endl;
	
	// if( goal.compare("Protoss_Probe") != 0 && goal.compare("Protoss_Pylon") != 0 )
	// {
	  bo.emplace_back( t.name, currentState.seconds );
	  if( goals.find( t.name ) != goals.end() )
	    ++goals.at( t.name ).second;
	// }
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
	      currentState.busy.push_back( t.action );
	      // warp building and return to mineral fields
	      currentState.inMove.push_back( Tuple( actionOf["Protoss_Mineral"], 4, false ) );
	      
	      currentState.stockMineral -= mineralCost;
	      currentState.stockGas -= gasCost;

	      currentState.mineralsBooked -= mineralCost;
	      currentState.gasBooked -= gasCost;

	      string text = "Start " + goal + " at ";
	      cout << std::left << setw(35) << text << setw(5) << currentState.seconds
		   << ",\t m = " << setw(9) << currentState.stockMineral
		   << ",\t g = " << setw(8) << currentState.stockGas
		   << ",\t mb = " << setw(5) << currentState.mineralsBooked
		   << ",\t gb = " << setw(4) << currentState.gasBooked
		   << ",\t mw = " << setw(3) << currentState.mineralWorkers
		   << ",\t gw = " << setw(3) << currentState.gasWorkers
		   << ",\t s = " << currentState.supplyUsed << "/" << currentState.supplyCapacity << ")" << endl;
	    }

	  t.done = true;
	  
	}
      }
    }

    auto itEnd = remove_if( begin( currentState.inMove ), end( currentState.inMove ), [](Tuple &t){return t.done;} );
    currentState.inMove.erase( itEnd, end( currentState.inMove ) );
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

  double BuildOrderObjective::mineralsIn( int seconds ) const
  {
    double futurProduction = 0.;
    int workers = currentState.mineralWorkers;

    // rough estimation
    futurProduction = workers * 1.08 * seconds;
    
    // for( int i = 1 ; i <= seconds ; ++i )
    // {
    //   for( const auto &t : currentState.inMove )
    // 	if( t.actor.compare("Protoss_Probe") == 0
    // 	    &&
    // 	    t.goal.compare("Mineral") == 0
    // 	    &&
    // 	    t.waitTime - i == 0 )
    // 	{
    // 	  ++workers;
    // 	}

    //   for( const auto &t : currentState.busy )
    // 	if( t.goal.compare("Protoss_Probe") == 0
    // 	    &&
    // 	    t.time + 2 - i == 0 )
    // 	{
    // 	  ++workers;
    // 	}
      
    //   futurProduction += workers * 1.08;
    // }

    return futurProduction;
  }
  
  double BuildOrderObjective::gasIn( int seconds ) const
  {
    double futurProduction = 0.;
    int workers = currentState.gasWorkers;

    // rough estimation
    futurProduction = workers * 1.68 * seconds;

    // for( int i = 1 ; i <= seconds ; ++i )
    // {
    //   for( const auto &t : currentState.inMove )
    // 	if( t.actor.compare("Protoss_Probe") == 0
    // 	    &&
    // 	    t.goal.compare("Gas") == 0
    // 	    &&
    // 	    t.waitTime - i == 0 )
    // 	{
    // 	  ++workers;
    // 	}
      
    //   futurProduction += workers * 1.68;
    // }

    return futurProduction;
  }

  void BuildOrderObjective::youMustConstructAdditionalPylons() const
  {
    // build the first pylon ASAP
    if( currentState.numberPylons == 0
	&& currentState.stockMineral >= 100 - mineralsIn( 4 ) )
    {
      currentState.inMove.push_back( Tuple( actionOf["Protoss_Pylon"], 5, false ) );

      currentState.mineralsBooked += 100;
      
      cout << std::left << setw(35) << "Go for first Protoss_Pylon at " << setw(5) << currentState.seconds
	   << ",\t m = " << setw(9) << currentState.stockMineral
	   << ",\t g = " << setw(8) << currentState.stockGas
	   << ",\t mb = " << setw(5) << currentState.mineralsBooked
	   << ",\t gb = " << setw(4) << currentState.gasBooked
	   << ",\t mw = " << setw(3) << currentState.mineralWorkers
	   << ",\t gw = " << setw(3) << currentState.gasWorkers
	   << ",\t s = " << currentState.supplyUsed << "/" << currentState.supplyCapacity << ")" << endl;
      
      if( currentState.mineralWorkers > 0 )
	--currentState.mineralWorkers;
      else
	--currentState.gasWorkers;
    }
    // otherwise build other pylons when needed
    else
    {
      int supplyConsumption = currentState.resources["Protoss_Nexus"] + 2*currentState.resources["Protoss_Gateway"];
      if( supplyConsumption + currentState.supplyUsed >= currentState.supplyCapacity )
      {
	int toBuild = ( (supplyConsumption + currentState.supplyUsed - currentState.supplyCapacity) / 8 ) + 1;

	// Besur we have enough mineral to build 'toBuild pylons'
	// if not, descrease toBuild till we can (eventually till toBuild == 0)
	while( currentState.stockMineral < toBuild*100 - mineralsIn( 4 ) && toBuild > 0 )
	{
	  --toBuild;
	}
	
	for( int i = 0 ; i < toBuild && i < currentState.mineralWorkers + currentState.gasWorkers ; ++i )
	{
	  currentState.inMove.push_back( Tuple( actionOf["Protoss_Pylon"], 5, false ) );

	  currentState.mineralsBooked += 100;

	  cout << std::left << setw(35) << "Go for Protoss_Pylon at " << setw(5) << currentState.seconds
	       << ",\t m = " << setw(9) << currentState.stockMineral
	       << ",\t g = " << setw(8) << currentState.stockGas
	       << ",\t mb = " << setw(5) << currentState.mineralsBooked
	       << ",\t gb = " << setw(4) << currentState.gasBooked
	       << ",\t mw = " << setw(3) << currentState.mineralWorkers
	       << ",\t gw = " << setw(3) << currentState.gasWorkers
	       << ",\t s = " << currentState.supplyUsed << "/" << currentState.supplyCapacity << ")" << endl;
	  
	  if( currentState.mineralWorkers > 0 )
	    --currentState.mineralWorkers;
	  else
	    --currentState.gasWorkers;
	}
      }
    }
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
    double bestHelp = numeric_limits<int>::max();

    for( int i = 0; i < vecGlobalCosts.size(); ++i )
    {
      if( vecGlobalCosts[i] < bestEstimatedCost
	  || ( vecGlobalCosts[i] == bestEstimatedCost
	       && vecGlobalCosts[i] < numeric_limits<int>::max()
	       && heuristicValueHelper.at( i ) < bestHelp ) )
      {
	bestEstimatedCost = vecGlobalCosts[i];
	bestValue = i;
	if( heuristicValueHelper.at( i ) < bestHelp )
	  bestHelp = heuristicValueHelper.at( i );
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
							 vector< Action > &bestSolution) const
  {
    chrono::time_point<chrono::high_resolution_clock> startPostprocess = chrono::high_resolution_clock::now(); 
    chrono::duration<double,micro> postprocesstimer(0);

    double optiCost = v_cost( vecVariables, domain );

    if( optiCost < bestCost )
    {
      bestCost = optiCost;
      copy( begin(*vecVariables), end(*vecVariables), begin(bestSolution) );
    }

    // printBO();
      
    postprocesstimer = chrono::high_resolution_clock::now() - startPostprocess;
    return postprocesstimer.count();
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

    printBO();

    postprocesstimer = chrono::high_resolution_clock::now() - startPostprocess;
    return postprocesstimer.count();
  }

  void BuildOrderObjective::makeVecVariables( const pair<string, int> &input, vector<Action> &variables, map< string, pair<int, int> > &goals )
  {
    Action action( actionOf[input.first] );
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
    	variables.push_back( Action( actionOf[ action.getFullName() ] ) );
      
      for( const auto &d : action.getDependencies() )
    	if( d.compare( "Protoss_High_Templar" ) == 0 || d.compare( "Protoss_Dark_Templar" ) == 0 )
    	  makeVecVariables( Action( actionOf[ d ] ), variables, 2 * count ); // Each (dark) archon needs 2 (dark) templars 
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
	      makeVecVariables( Action( actionOf[ d ] ), variables, 1 );
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
