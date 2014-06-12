/*
 * GHOST (General meta-Heuristic Optimization Solving Tool) is a C++ library 
 * designed for StarCraft: Brood war. 
 * GHOST is a meta-heuristic solver aiming to solve any kind of combinatorial 
 * and optimization RTS-related problems represented by a CSP. 
 * It is an extension of the project Wall-in.
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


#include <cmath>
#include <chrono>
#include <ctime>
#include <limits>
#include <algorithm>
#include <functional>
#include <cassert>
#include <typeinfo>

#include "../include/solver.hpp"

constexpr int TABU	= 5;
constexpr int OPT_TIME	= 150;

namespace ghost
{
  void Solver::reset()
  {
    int xPos;
    int yPos;

    clearAllInGrid( vecVariables, domain );

    for( auto b : vecVariables )
    {
      // 1 chance over 3 to be placed on the domain
      if( randomVar.getRandNum(3) == 0)
      {
	b.setValue( domain.randomValue( b ) );
	domain->add( b );
      }
      else
	b.setValue( -1 );
    }

    updateConstraints( vecConstraints, domain );
  }

  double Solver::solve( double timeout )
  {
    chrono::duration<double,milli> elapsedTime;
    chrono::duration<double,milli> elapsedTimeTour;
    chrono::time_point<chrono::system_clock> start;
    chrono::time_point<chrono::system_clock> startTour;
    start = chrono::system_clock::now();

    // to time simulateCost and cost functions
    chrono::duration<double,milli> timeSimCost(0);
    chrono::time_point<chrono::system_clock> startSimCost; 

#ifndef NDEBUG
    chrono::duration<double,milli> toverlap(0), tbuildable(0), tnogaps(0), tstt(0);
    chrono::time_point<chrono::system_clock> soverlap, sbuildable, snogaps, sstt; 
#endif

    int sizeDomain = domain->getSize();
    vector< vector< double > >  vecConstraintsCosts( vecConstraints.size() );
    vector< double >		vecGlobalCosts( sizeDomain );
    vector< vector< double > >  vecVarSimCosts( sizeDomain );
    objective->initHelper( sizeDomain );

    bestCost = numeric_limits<int>::max();
    double beforePostProc = bestCost;
    double bestGlobalCost = numeric_limits<int>::max();
    double globalCost;
    double currentCost;
    double estimatedCost;
    double bestEstimatedCost;
    int    bestPosition;

    vector<int> worstVariables;
    double worstVariableCost;
    int worstVariableId;

    shared_ptr<Variable> oldVariable;
    vector<int> possiblePositions;
    vector<double> varSimCost( vecVariables.size() );
    vector<double> bestSimCost( vecVariables.size() );

    int tour = 0;

    do // optimization loop
    {
      startTour = chrono::system_clock::now();
      ++tour;
      globalCost = numeric_limits<int>::max();
      bestEstimatedCost = numeric_limits<int>::max();
      std::fill( varSimCost.begin(), varSimCost.end(), 0. );
      std::fill( bestSimCost.begin(), bestSimCost.end(), 0. );
      std::fill( vecConstraintsCosts.begin(), vecConstraintsCosts.end(), vector<double>( vecVariables.size(), 0. ) );
      std::fill( vecVarSimCosts.begin(), vecVarSimCosts.end(), vector<double>( vecVariables.size(), 0. ) );
      std::fill( variableCost.begin(), variableCost.end(), 0. );
      std::fill( tabuList.begin(), tabuList.end(), 0 );

      do // solving loop 
      {
	if( globalCost == numeric_limits<int>::max() )
	{
	  currentCost = 0.;

	  for( auto c : vecConstraints )
	    currentCost += c->cost( variableCost );

	  if( currentCost < globalCost )
	    globalCost = currentCost;
	  else
	  {
	    reset();
	    continue;
	  }
	}

	// make sure there is at least one untabu variable
	bool freeVariables = false;

	// Update tabu list
	for( int i = 0; i < tabuList.size(); ++i )
	{
	  if( tabuList[i] <= 1 )
	  {
	    tabuList[i] = 0;
	    if( !freeVariables )
	      freeVariables = true;      
	  }
	  else
	    --tabuList[i];
	}

	// Here, we look at neighbor configurations with the lowest cost.
	worstVariables.clear();
	worstVariableCost = 0;
	for( int i = 0; i < variableCost.size(); ++i )
	{
	  if( !freeVariables || tabuList[i] == 0 )
	  {
	    if( worstVariableCost < variableCost[i] )
	    {
	      worstVariableCost = variableCost[i];
	      worstVariables.clear();
	      worstVariables.push_back( i );
	    }
	    else 
	      if( worstVariableCost == variableCost[i] )
		worstVariables.push_back( i );	  
	  }
	}
      
	// can apply some heuristics here, according to the objective function
	worstVariableId = objective->heuristicVariable( worstVariables, vecVariables, domain );
	oldVariable = vecVariables[ worstVariableId ];
      
	// get possible positions for oldVariable.
	possiblePositions = domain->possibleValues( *oldVariable );

	// time simulateCost
	startSimCost = chrono::system_clock::now();

	// variable simulated costs
	fill( bestSimCost.begin(), bestSimCost.end(), 0. );

#ifndef NDEBUG
	soverlap = chrono::system_clock::now();
	vecConstraintsCosts[0] = vecConstraints[0]->simulateCost( *oldVariable, possiblePositions, vecVarSimCosts, objective );
	toverlap += chrono::system_clock::now() - soverlap;

	sbuildable = chrono::system_clock::now();
	vecConstraintsCosts[1] = vecConstraints[1]->simulateCost( *oldVariable, possiblePositions, vecVarSimCosts );
	tbuildable += chrono::system_clock::now() - sbuildable;

	snogaps = chrono::system_clock::now();
	vecConstraintsCosts[2] = vecConstraints[2]->simulateCost( *oldVariable, possiblePositions, vecVarSimCosts );
	tnogaps += chrono::system_clock::now() - snogaps;

	sstt = chrono::system_clock::now();
	vecConstraintsCosts[3] = vecConstraints[3]->simulateCost( *oldVariable, possiblePositions, vecVarSimCosts );
	tstt += chrono::system_clock::now() - sstt;
#else
	vecConstraintsCosts[0] = vecConstraints[0]->simulateCost( *oldVariable, possiblePositions, vecVarSimCosts, objective );
	for( int i = 1; i < vecConstraints.size(); ++i )
	  vecConstraintsCosts[i] = vecConstraints[i]->simulateCost( *oldVariable, possiblePositions, vecVarSimCosts );
#endif

	fill( vecGlobalCosts.begin(), vecGlobalCosts.end(), 0. );

	// sum all numbers in the vector vecConstraintsCosts[i] and put it into vecGlobalCosts[i] 
	for( auto v : vecConstraintsCosts )
	  transform( vecGlobalCosts.begin(), 
		     vecGlobalCosts.end(), 
		     v.begin(), 
		     vecGlobalCosts.begin(), 
		     plus<double>() );

	// replace all negative numbers by the max value for double
	replace_if( vecGlobalCosts.begin(), 
		    vecGlobalCosts.end(), 
		    bind( less<double>(), placeholders::_1, 0. ), 
		    numeric_limits<int>::max() );

	// look for the first smallest cost, according to objective heuristic
	int b = objective->heuristicValue( vecGlobalCosts, bestEstimatedCost, bestPosition, domain);
	bestSimCost = vecVarSimCosts[ b ];

	timeSimCost += chrono::system_clock::now() - startSimCost;

	currentCost = bestEstimatedCost;

	if( bestEstimatedCost < globalCost )
	{
	  globalCost = bestEstimatedCost;

	  if( globalCost < bestGlobalCost )
	    bestGlobalCost = globalCost;

	  variableCost = bestSimCost;
	  move( oldVariable, bestPosition );
	}
	else // local minima
	  tabuList[ worstVariableId ] = TABU;

	elapsedTimeTour = chrono::system_clock::now() - startTour;
      } while( globalCost != 0. && elapsedTimeTour.count() < timeout );

      // remove useless buildings
      if( globalCost == 0 )
	objective->postprocessSatisfaction( vecVariables, domain, bestCost, bestSolution );

      reset();
      elapsedTime = chrono::system_clock::now() - start;
    }
    while( ( ( objective->getName().compare("none") != 0 || loops == 0 )  && ( elapsedTime.count() < OPT_TIME ) )//|| ( elapsedTime.count() >= OPT_TIME && bestGlobalCost != 0 && elapsedTime.count() < 10 * OPT_TIME ) ) 
	   || ( objective->getName().compare("none") == 0 && elapsedTime.count() < timeout * loops ) );

    clearAllInGrid( vecVariables, domain );

    for( int i = 0; i < vecVariables.size(); ++i )
      vecVariables[i]->setValue( bestSolution[i] );
    
    addAllInGrid( vecVariables, domain );

    if( bestGlobalCost == 0 )
      objective->postprocessOptimization( vecVariables, domain, bestCost );
    
    // // For gap objective, try now to decrease the number of gaps.
    // if( ( objective->getName().compare("gap") == 0 || objective->getName().compare("techtree") == 0 ) && bestGlobalCost == 0 )
    // {
    //   //objective.reset( new GapObj("gap") );
    //   std::fill( tabuList.begin(), tabuList.end(), 0 );
        
    //   for( auto v : vecVariables )
    // 	buildingSameSize.insert( make_pair( v->getSurface(), v ) );

    //   vector<int> goodVar;
    //   shared_ptr<Variable> toSwap;
    //   bool mustSwap;

    //   chrono::time_point<chrono::system_clock> startPostprocess = chrono::system_clock::now(); 
    
    //   bestCost = objective->cost( vecVariables, domain );
    //   double currentCost = bestCost;
    //   beforePostProc = bestCost;

    //   while( (postprocessGap = chrono::system_clock::now() - startPostprocess).count() < static_cast<int>( ceil(OPT_TIME / 100) ) && bestCost > 0 )
    //   {
    // 	goodVar.clear();

    // 	for( int i = 0; i < tabuList.size(); ++i )
    // 	{
    // 	  if( tabuList[i] <= 1 )
    // 	    tabuList[i] = 0;
    // 	  else
    // 	    --tabuList[i];
    // 	}

    // 	for( int i = 0; i < vecVariables.size(); ++i )
    // 	{
    // 	  if( tabuList[i] == 0 )
    // 	    goodVar.push_back( i );
    // 	}

    // 	if( goodVar.empty() )
    // 	  for( int i = 0; i < vecVariables.size(); ++i )
    // 	    goodVar.push_back( i );	

    // 	int index = objective->heuristicVariable( goodVar, vecVariables, domain );
    // 	oldVariable = vecVariables[ index ];
    // 	auto surface = buildingSameSize.equal_range( oldVariable->getSurface() );
	
    // 	for( auto it = surface.first; it != surface.second; ++it )
    // 	{
    // 	  mustSwap = false;
    // 	  if( it->second->getId() != oldVariable->getId() )
    // 	  {
    // 	    domain->swap( *it->second, *oldVariable );
	    
    // 	    currentCost = objective->cost( vecVariables, domain );
    // 	    if( currentCost < bestCost )
    // 	    {
    // 	      bestCost = currentCost;
    // 	      toSwap = it->second;
    // 	      mustSwap = true;
    // 	    }

    // 	    domain->swap( *it->second, *oldVariable );
    // 	  }
	  
    // 	  if( mustSwap )
    // 	    domain->swap( *toSwap, *oldVariable );
    // 	}

    // 	tabuList[ index ] = 2;//std::max(2, static_cast<int>( ceil(TABU / 2) ) );
    //   }
    // }
 
    cout << "Domains:" << domain << endl;

    if( objective->getName().compare("none") == 0 )
      cout << "SATISFACTION run: try to find a sound wall only!" << endl;
    else
      cout << "OPTIMIZATION run with objective " << objective->getName() << endl;
      
    cout << "Elapsed time: " << elapsedTime.count() << endl
	 << "Global cost: " << bestGlobalCost << endl
	 << "Number of tours: " << tour << endl;

    if( objective->getName().compare("none") == 0 )
    {
      if( bestGlobalCost == 0 )
      {
	VariableObj bObj("building");
	GapObj gObj("gap");
	TechTreeObj tObj("techtree");
	
	cout << "Opt Cost if the objective was building: " << bObj.cost( vecVariables, domain ) << endl
	     << "Opt Cost if the objective was gap: \t" << gObj.cost( vecVariables, domain ) << endl
	     << "Opt Cost if the objective was techtree: " << tObj.cost( vecVariables, domain ) << endl;
      }
    }
    else
    {
      cout << "Optimization cost: " << bestCost << endl
	   << "Opt Cost BEFORE post-processing: " << beforePostProc << endl;
    }

    // if( objective->getName().compare("gap") == 0 || objective->getName().compare("techtree") == 0 )
    //   cout << "Post-processing time: " << postprocessGap.count() << endl; 

#ifndef NDEBUG
    cout << endl << "Elapsed time to simulate cost: " << timeSimCost.count() << endl
	 << "Overlap: " << toverlap.count() << endl
	 << "Buildable: " << tbuildable.count() << endl
	 << "NoGaps: " << tnogaps.count() << endl
	 << "STT: " << tstt.count() << endl;

    updateConstraints( vecConstraints, domain );

    // print cost for each constraint
    for( auto c : vecConstraints )
    {
      fill( varSimCost.begin(), varSimCost.end(), 0. );
      cout << "Cost of " << typeid(*c).name() << ": " << c->cost( varSimCost ) << " [";

      for( auto v : varSimCost )
	cout << " " << v;

      cout << " ]" << endl;
    }      
    
    cout << endl;
#endif

    if( objective->getName().compare("none") == 0 )
      return bestGlobalCost;
    else
      return bestCost;
  }
}
