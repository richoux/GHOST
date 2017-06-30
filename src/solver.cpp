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

#include <cassert>

#include "solver.hpp"

using namespace ghost;

Solver::Solver( vector< Variable > vecVariables, 
		vector< shared_ptr<Constraint> > vecConstraints,
		shared_ptr< Objective > obj )
  : vecVariables(vecVariables), 
    vecConstraints(vecConstraints),
    objective(obj),
    tabuList(vecVariables.size()),
    bestSolution(vecVariables.size()),
    objOriginalNull(false)
{ }

double Solver::solve( double sat_timeout, double opt_timeout )
{
  sat_timeout *= 1000; // timeouts in microseconds
  if( opt_timeout == 0 )
    opt_timeout = sat_timeout * 10;
  else
    opt_timeout *= 1000;

  int tabu_length = vecVariables.size() - 1;

  chrono::duration<double,micro> elapsedTime(0);
  chrono::duration<double,micro> elapsedTimeTour(0);
  chrono::time_point<chrono::high_resolution_clock> start;
  chrono::time_point<chrono::high_resolution_clock> startTour;
  start = chrono::high_resolution_clock::now();

  // to time simulateCost and cost functions
  chrono::duration<double,micro> timeSimCost(0);
  chrono::time_point<chrono::high_resolution_clock> startSimCost; 

  // double timerPostProcessSat = 0;
  double timerPostProcessOpt = 0;
      
  if( objective == nullptr && !objOriginalNull )
  {
    objective = make_shared< NullObjective >();
    objOriginalNull = true;
  }
      
  bestCost = numeric_limits<int>::max();
  double beforePostProc = bestCost;
  double bestGlobalCost = bestCost;;
  double globalCost;
  double currentCost;
  double bestEstimatedCost;
  int    bestValue = 0;

  vector<int> worstVariables;
  double worstVariableCost;
  int worstVariableId;

  Variable oldVariable;
  vector<int> possibleValues;

  int opt_loop = 0;
  int sat_loop = 0;

  do // optimization loop
  {
    startOpt_Loop = chrono::high_resolution_clock::now();
    ++opt_loop;
    globalCost = numeric_limits<int>::max();
    bestEstimatedCost = numeric_limits<int>::max();
    std::fill( tabuList.begin(), tabuList.end(), 0 );

    do // satisfaction loop 
    {
      ++sat_loop;
	  
      if( globalCost == numeric_limits<int>::max() )
      {
	for( auto& v : vecVariables )
	  v.do_random_initialization();

	currentCost = 0.;

	for( const auto &c : vecConstraints )
	  currentCost += c->cost();

	globalCost = currentCost;
	assert( globalCost < numeric_limits<int>::max() );
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
      oldVariable = &vecVariables.at( worstVariableId );
      
      // get possible values for oldVariable.
      possibleValues = domain->valuesOf( *oldVariable );

      // time simulateCost
      startSimCost = chrono::high_resolution_clock::now();

      // variable simulated costs
      fill( bestSimCost.begin(), bestSimCost.end(), 0. );

      if( !objOriginalNull )
	vecConstraintsCosts[0] = vecConstraints[0]->simulateCost( *oldVariable, possibleValues, vecVarSimCosts, objective );
      else
	vecConstraintsCosts[0] = vecConstraints[0]->simulateCost( *oldVariable, possibleValues, vecVarSimCosts );

      for( int i = 1; i < vecConstraints.size(); ++i )
	vecConstraintsCosts[i] = vecConstraints[i]->simulateCost( *oldVariable, possibleValues, vecVarSimCosts );

      fill( vecGlobalCosts.begin(), vecGlobalCosts.end(), 0. );

      // sum all numbers in the vector vecConstraintsCosts[i] and put it into vecGlobalCosts[i] 
      for( const auto &v : vecConstraintsCosts )
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
      int b = objective->heuristicValue( vecGlobalCosts, bestEstimatedCost, bestValue );
      bestSimCost = vecVarSimCosts[ b ];

      timeSimCost += chrono::high_resolution_clock::now() - startSimCost;

      currentCost = bestEstimatedCost;

      if( bestEstimatedCost < globalCost )
      {
	globalCost = bestEstimatedCost;

	if( objective->isPermutation() )
	  permut( oldVariable, bestValue );
	else
	  move( oldVariable, bestValue );

	if( globalCost < bestGlobalCost )
	{
	  bestGlobalCost = globalCost;
	  copy( begin(*vecVariables), end(*vecVariables), begin(bestSolution) );
	  // cout << "COPY BEST" << *domain << endl;
	}
	    
	variableCost = bestSimCost;
      }
      else // local minima
	tabuList[ worstVariableId ] = tabu_length;

      elapsedTimeOpt_Loop = chrono::high_resolution_clock::now() - startOpt_Loop;
      elapsedTime = chrono::high_resolution_clock::now() - start;
    } while( globalCost != 0. && elapsedTimeOpt_Loop.count() < sat_timeout && elapsedTime.count() < opt_timeout );

    // remove useless variables
    if( globalCost == 0 )
      objective->postprocessSatisfaction( vecVariables, domain, bestCost, bestSolution, sat_timeout );

    elapsedTime = chrono::high_resolution_clock::now() - start;
  }
  while( elapsedTime.count() < opt_timeout );

  domain->wipe( vecVariables );
  domain->copyBest( bestSolution, vecVariables );
  // copy( begin(bestSolution), end(bestSolution), begin(*vecVariables) );
  domain->rebuild( vecVariables );
  beforePostProc = bestCost;
      
  if( bestGlobalCost == 0 )
    timerPostProcessOpt = objective->postprocessOptimization( vecVariables, domain, bestCost, opt_timeout );

  // for( const auto &v : *vecVariables )
  // 	cout << v << endl;

  // cout << "Domains:" << *domain << endl;

#ifndef NDEBUG
  cout << "############" << endl;
      
  if( objOriginalNull )
    cout << "SATISFACTION run" << endl;
  else
    cout << "OPTIMIZATION run with objective " << objective->getName() << endl;
      
  cout << "Elapsed time: " << elapsedTime.count() / 1000 << endl
       << "Global cost: " << bestGlobalCost << endl
       << "Number of optization loops: " << opt_loop << endl
       << "Number of satisfaction loops: " << sat_loop << endl;

  if( !objOriginalNull )
  {
    cout << "Optimization cost: " << bestCost << endl
	 << "Opt Cost BEFORE post-processing: " << beforePostProc << endl;
  }
      
  if( timerPostProcessOpt != 0 )
    cout << "Post-processing time: " << timerPostProcessOpt / 1000 << endl; 

  cout << endl;
#endif
      
  if( objOriginalNull )
    return bestGlobalCost;
  else
    return bestCost;
}

void SolveR::move( TypeVariable *variable, int newValue )
{
  domain->clear( *variable );
  variable->setValue( newValue );
  domain->add( *variable );
}

void Solver::permut( TypeVariable *variable, int newValue )
{
  int backup = variable->getValue();

  if( backup == newValue )
    return;
      
  if( backup > newValue )
  {
    for( int i = backup ; i > newValue ; --i )
    {
      std::swap( vecVariables.at(i-1), vecVariables.at(i) );
      vecVariables.at(i).shiftValue();
    }

    vecVariables.at(newValue).setValue( newValue );	
  }
  else
  {
    for( int i = backup ; i < newValue ; ++i )
    {
      std::swap( vecVariables.at(i), vecVariables.at(i+1) );
      vecVariables.at(i).unshiftValue();
    }

    vecVariables.at(newValue).setValue( newValue );	
  }
}
