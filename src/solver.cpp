/*
 * GHOST (General meta-Heuristic Optimization Solving Tool) is a C++ library 
 * designed to help developers to model and implement optimization problem 
 * solving. It contains a meta-heuristic solver aiming to solve any kind of 
 * combinatorial and optimization real-time problems represented by a CSP/COP. 
 *
 * GHOST has been first developped to help making AI for the RTS game
 * StarCraft: Brood war, but can be used for any kind of applications where 
 * solving combinatorial and optimization problems within some tenth of 
 * milliseconds is needed. It is a generalization of the Wall-in project.
 * Please visit https://github.com/richoux/GHOST for further information.
 * 
 * Copyright (C) 2014-2017 Florian Richoux
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
#include <limits>

#include "solver.hpp"

using namespace ghost;

Solver::Solver( vector< Variable >& vecVariables, 
		vector< shared_ptr<Constraint> > vecConstraints,
		shared_ptr< Objective > objective,
		bool permutationProblem )
  : _vecVariables(vecVariables), 
    _vecConstraints(vecConstraints),
    _objective(objective),
    _permutationProblem(permutationProblem),
    _tabuList(vecVariables.size()),
    _bestSolution(vecVariables.size()),
    _objOriginalNull(false)
{ }

double Solver::solve( double satTimeout, double optTimeout )
{
  satTimeout *= 1000; // timeouts in microseconds
  if( optTimeout == 0 )
    optTimeout = satTimeout * 10;
  else
    optTimeout *= 1000;

  // The only parameter of Solver::solve outside timeouts
  int tabuTime = _vecVariables.size() - 1;

  chrono::duration<double,micro> elapsedTime(0);
  chrono::duration<double,micro> elapsedTimeTour(0);
  chrono::time_point<chrono::steady_clock> start;
  chrono::time_point<chrono::steady_clock> startTour;
  start = chrono::steady_clock::now();

  // To time cost functions
  chrono::duration<double,micro> timeSimCost(0);
  chrono::time_point<chrono::steady_clock> startSimCost; 

  // double timerPostProcessSat = 0;
  double timerPostProcessOpt = 0;
      
  if( _objective == nullptr )
  {
    _objective = make_shared< NullObjective >();
    _objOriginalNull = true;
  }
      
  int optLoop = 0;
  int satLoop = 0;

  vector< Variable > worstVariableList;
  Variable* worstVariable;
  double currentSatCost;
  double currentOptCost;
  vector< double > costConstraints( _vecConstraints.size(), 0. );
  vector< double > costVariables( _vecVariables.size(), 0. );
  
  do // optimization loop
  {
    startOptLoop = chrono::steady_clock::now();
    ++optLoop;

    // Reset tabu list
    std::fill( tabuList.begin(), tabuList.end(), 0 );

    do // satisfaction loop 
    {
      ++satLoop;

      // Reset variables and constraints costs
      std::fill( costConstraints.begin(), costConstraints.end(), 0. );
      std::fill( costVariables.begin(), costVariables.end(), 0. );

      currentSatCost = compute_constraints_costs( costConstraints );
      compute_variables_costs( costConstraints, costVariables );
      
      bool freeVariables = false;
      decay_tabu_list( freeVariables );
      worstVariableList = compute_worst_variables( freeVariables );

      // If several variables share the same worst variable cost,
      // call Objective::heuristicVariable has a tie-break.
      // By default, Objective::heuristicVariable returns a random variable
      // among the vector of Variables given in argument.
      if( worstVariableList.size() > 1 )
	worstVariable = _objective->heuristicVariable( worstVariableList );
      else
	worstVariable = &worstVariableList[0];

      if( _permutationProblem )
	permutation_move( worstVariable );
      else
	local_move( worstVariable );

      
      timeSimCost += chrono::steady_clock::now() - startSimCost;

      else // local minima
	tabuList[ worstVariableId ] = tabuTime;

      elapsedTimeOptLoop = chrono::steady_clock::now() - startOptLoop;
      elapsedTime = chrono::steady_clock::now() - start;
    } while( globalCost != 0. && elapsedTimeOptLoop.count() < satTimeout && elapsedTime.count() < optTimeout );

    // remove useless variables
    if( globalCost == 0 )
      objective->postprocessSatisfaction( vecVariables, domain, bestCost, bestSolution, satTimeout );

    elapsedTime = chrono::steady_clock::now() - start;
  }
  while( elapsedTime.count() < optTimeout );


  
  if( bestGlobalCost == 0 )
    timerPostProcessOpt = objective->postprocessOptimization( vecVariables, domain, bestCost, optTimeout );


  
#ifndef NDEBUG
  cout << "############" << endl;
      
  if( objOriginalNull )
    cout << "SATISFACTION run" << endl;
  else
    cout << "OPTIMIZATION run with objective " << objective->getName() << endl;
      
  cout << "Elapsed time: " << elapsedTime.count() / 1000 << endl
       << "Global cost: " << bestGlobalCost << endl
       << "Number of optization loops: " << optLoop << endl
       << "Number of satisfaction loops: " << satLoop << endl;

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

void Solver::decay_tabu_list( bool& freeVariables ) 
{
  for( int i = 0 ; i < tabuList.size() ; ++i )
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
}

vector< Variable> Solver::compute_worst_variables( bool freeVariables, const vector<double>& costVariables )
{
  // Here, we look at neighbor configurations with the lowest cost.
  vector< Variable> worstVariableList;
  double worstVariableCost = 0.;

  for( int i = 0; i < _vecVariables.size(); ++i )
  {
    if( !freeVariables || tabuList[i] == 0 )
    {
      if( worstVariableCost < costVariables[i] )
      {
	worstVariableCost = costVariables[i];
	worstVariableList.clear();
	worstVariableList.push_back( i );
      }
      else 
	if( worstVariableCost == costVariables[i] )
	  worstVariableList.push_back( i );	  
    }
  }

  return worstVariableList;
}

double Solver::compute_constraints_costs( vector<double>& costConstraints )
{
  double globalCost = 0.;
  
  for( int i = 0 ; i < _vecConstraints.size() ; ++i )
  {
    costConstraints[i] = _vecConstraints[i]->cost();
    globalCost += costConstraints[i];
  }

  return globalCost;
}

void Solver::compute_variables_costs( const vector<double>& costConstraints, vector<double>& costVariables )
{
  for( int v = 0 ; v < _vecVariables.size() ; ++v )
    for( int c = 0 ; c < _vecConstraints.size() ; ++c )
      if( _vecConstraints[c]->hasVariable( _vecVariables[v] ) )
	costVariables[v] += costConstraints[c];
}
  
void Solver::local_move( Variable *variable )
{
  auto domainOfWV = worstVariable->possible_values();

  // Here, we look at values in the variable domain
  // leading to the lowest global cost.
  vector< Variable> bestValuesList;
  int bestValue = std::numeric_limits<int>::max();

  ////////////////////////////////////////////////////
  ////////////////////////////////////////////////////
  ////////////////////////////////////////////////////
  // must compute global cost while value changed here
  ////////////////////////////////////////////////////
  ////////////////////////////////////////////////////
  ////////////////////////////////////////////////////
  
  for( int i = 0; i < domainOfWV.size(); ++i )
  {
    if( bestValue > costVariables[i] )
    {
      worstVariableCost = costVariables[i];
      worstVariableList.clear();
      worstVariableList.push_back( i );
    }
    else 
      if( worstVariableCost == costVariables[i] )
	worstVariableList.push_back( i );	  
  }
}

void Solver::permutation_move( Variable *variable )
{
  auto domainOfWV = worstVariable->possible_values();

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
