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
    _weakTabuList(vecVariables.size()),
    _isOptimization(objective == nullptr ? false : true)
{
  for( auto& var : vecVariables )
    for( auto& ctr : vecConstraints )
      if( ctr->hasVariable( var ) )
	_mapVarCtr[ var ].push_back( ctr );
  // _mapVarCtr[ var ].push_back( make_pair( ctr, ctr->get_variable_iterator( var ) ) );
}

Solver::Solver( vector< Variable >& vecVariables, 
		vector< shared_ptr<Constraint> > vecConstraints,
		bool permutationProblem )
  : Solver( vecVariables, vecConstraints, nullptr, permutationProblem )
{ }

bool Solver::solve( double& finalCost, vector<int>& finalSolution, double satTimeout, double optTimeout )
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
  chrono::time_point<chrono::steady_clock> startPostprocess;
  start = chrono::steady_clock::now();

  // To time cost functions
  chrono::duration<double,micro> timeSimCost(0);
  chrono::time_point<chrono::steady_clock> startSimCost; 

  chrono::duration<double,micro> timerPostProcessSat(0);
  chrono::duration<double,micro> timerPostProcessOpt(0);
      
  if( _objective == nullptr )
    _objective = make_shared< NullObjective >();
      
  int optLoop = 0;
  int satLoop = 0;

  vector< Variable > worstVariableList;
  Variable* worstVariable;
  double currentSatCost;
  double currentOptCost;
  vector< double > costConstraints( _vecConstraints.size(), 0. );
  vector< double > costVariables( _vecVariables.size(), 0. );

  // In case finalSolution is not a vector of the correct size,
  // ie, equals to the number of variables.
  finalSolution.clear();
  finalSolution.reshape( _vecVariables.size() );
  
  _bestOptCost = std::numeric_limits<double>::max();
  
  do // optimization loop
  {
    startOptLoop = chrono::steady_clock::now();
    ++optLoop;

    // Reset weak tabu list
    std::fill( _weakTabuList.begin(), _weakTabuList.end(), 0 );

    // Reset the best satisfaction cost
    _bestSatCost = std::numeric_limits<double>::max();
    
    do // satisfaction loop 
    {
      ++satLoop;

      // Reset variables and constraints costs
      std::fill( costConstraints.begin(), costConstraints.end(), 0. );
      std::fill( costVariables.begin(), costVariables.end(), 0. );

      currentSatCost = compute_constraints_costs( costConstraints );
      compute_variables_costs( costConstraints, costVariables );
      
      bool freeVariables = false;
      decay_weak_tabu_list( freeVariables );
      worstVariableList = compute_worst_variables( freeVariables );

      // If several variables share the same worst variable cost,
      // call Objective::heuristic_variable has a tie-break.
      // By default, Objective::heuristic_variable returns a random variable
      // among the vector of Variables given in argument.
      if( worstVariableList.size() > 1 )
	worstVariable = _objective->heuristic_variable( worstVariableList );
      else
	worstVariable = &worstVariableList[0];

      if( _permutationProblem )
	permutation_move( worstVariable, costConstraints, costVariables, currentSatCost );
      else
	local_move( worstVariable, costConstraints, costVariables, currentSatCost );

      if( _bestSatCost > currentSatCost )
      {
	_bestSatCost = currentSatCost;
	// see the right <algorithm> function
	for( auto& v : _vecVariables )
	  finalSolution[ v.get_id() ] = v.get_value();
	// std::fill( finalSolution.begin(), finalSolution.end(), [](){})
      }
      else // local minima
	// Mark worstVariable as weak tabu for tabuTime iterations.
	_weakTabuList[ worstVariable->get_id() ] = tabuTime;
      
      timeSimCost += chrono::steady_clock::now() - startSimCost;

      elapsedTimeOptLoop = chrono::steady_clock::now() - startOptLoop;
      elapsedTime = chrono::steady_clock::now() - start;
    } // satisfaction loop
    while( _bestSatCost > 0. && elapsedTimeOptLoop.count() < satTimeout && elapsedTime.count() < optTimeout );

    // remove useless variables
    if( _bestSatCost == 0. )
    {
      startPostprocess = chrono::steady_clock::now();
      _objective->postprocess_satisfaction( _vecVariables, _bestOptCost, finalSolution );
      timerPostProcessSat = chrono::steady_clock::now() - startPostprocess;							     
    }
    
    elapsedTime = chrono::steady_clock::now() - start;
  } // optimization loop
  while( elapsedTime.count() < optTimeout && _isOptimization );

  if( _bestSatCost == 0. && _isOptimization )
  {
    startPostprocess = chrono::steady_clock::now();
    _objective->postprocess_optimization( _vecVariables, _bestOptCost, finalSolution );
    timerPostProcessOpt = chrono::steady_clock::now() - startPostprocess;							     
  }
  
#ifndef NDEBUG
  cout << "############" << endl;
      
  if( !_isOptimization )
    cout << "SATISFACTION run" << endl;
  else
    cout << "OPTIMIZATION run with objective " << _objective->getName() << endl;
      
  cout << "Elapsed time: " << elapsedTime.count() / 1000 << endl
       << "Global cost: " << bestGlobalCost << endl
       << "Number of optization loops: " << optLoop << endl
       << "Number of satisfaction loops: " << satLoop << endl;

  if( _isOptimization )
  {
    cout << "Optimization cost: " << bestCost << endl
	 << "Opt Cost BEFORE post-processing: " << beforePostProc << endl;
  }
      
  if( timerPostProcessSat > 0 )
    cout << "Satisfaction post-processing time: " << timerPostProcessSat.cout() / 1000 << end; 

  if( timerPostProcessOpt > 0 )
    cout << "Optimization post-processing time: " << timerPostProcessOpt.cout() / 1000 << end; 

  cout << endl;
#endif
      
  if( _isOptimization )
    finalCost = _bestOptCost;
  else
    finalCost = _bestSatCost;

  // Set the variables to the best solution values.
  // Useful if the user prefer to directly use the vector of Variables
  // to manipulate and exploit the solution.
  for( auto& v : _vecVariables )
    v.set_value( finalSolution[ v.get_id() ] );
  
  return _bestSatCost == 0.;
}

double Solver::compute_constraints_costs( vector<double>& costConstraints ) const
{
  double globalCost = 0.;
  double cost;
  
  for( auto& c : _vecConstraints )
  {
    cost = c->cost();
    costConstraints[ c->get_id() ] = cost;
    globalCost += cost;    
  }

  return globalCost;
}

void Solver::compute_variables_costs( const vector<double>& costConstraints, vector<double>& costVariables ) const
{
  for( auto& v : _vecVariables )
    for( auto& c : _mapVarCtr[ v ] )
      costVariables[ v.get_id() ] += costConstraints[ c.get_id() ];
}

void Solver::decay_weak_tabu_list( bool& freeVariables ) 
{
  for( auto& tabu : weakTabuList )
  {
    if( tabu <= 1 )
    {
      tabu = 0;
      if( !freeVariables )
	freeVariables = true;      
    }
    else
      --tabu;
  }
}

vector< Variable> Solver::compute_worst_variables( bool freeVariables, const vector<double>& costVariables ) const
{
  // Here, we look at neighbor configurations with the lowest cost.
  vector< Variable> worstVariableList;
  double worstVariableCost = 0.;
  int id;
  
  for( auto& v : _vecVariables )
  {
    id = v.get_id();
    if( !freeVariables || _weakTabuList[ id ] == 0 )
    {
      if( worstVariableCost < costVariables[ id ] )
      {
	worstVariableCost = costVariables[ id ];
	worstVariableList.clear();
	worstVariableList.push_back( v );
      }
      else 
	if( worstVariableCost == costVariables[ id ] )
	  worstVariableList.push_back( v );	  
    }
  }

  return worstVariableList;
}

// NO VALUE BACKED-UP!
double Solver::simulate_local_move_cost( Variable *variable, double value, vector<double>& costConstraints, double& currentSatCost ) const
{
  double newCurrentSatCost = currentSatCost;

  variable->set_value( value );
  for( auto& c : _mapVarCtr[ *variable ] )
    newCurrentSatCost += ( c->cost() - costConstraints[ c->get_id() ] );

  return newCurrentSatCost;
}

double Solver::simulate_permutation_cost( Variable *worstVariable, Variable& otherVariable, vector<double>& costConstraints ) const
{
  double newCurrentSatCost = currentSatCost;
  int tmp = worstVariable->get_value();
  worstVariable->set_value( otherVariable.get_value() );
  otherVariable.set_value( tmp );

  vector<bool> compted( costConstraints.size(), false );
  
  for( auto& c : _mapVarCtr[ *worstVariable ] )
  {
    newCurrentSatCost += ( c->cost() - costConstraints[ c->get_id() ] );
    compted[ c->get_id() ] = true;
  }
  
  for( auto& c : _mapVarCtr[ otherVariable ] )
    if( !compted[ c->get_id() ] )
      newCurrentSatCost += ( c->cost() - costConstraints[ c->get_id() ] );

  // We must roll back to the previous state before returning the new cost value. 
  tmp = worstVariable->get_value();
  worstVariable->set_value( otherVariable.get_value() );
  otherVariable.set_value( tmp );

  return newCurrentSatCost;
}

void Solver::local_move( Variable *variable, vector<double>& costConstraints, vector<double>& costVariables, double& currentSatCost )
{
  // Here, we look at values in the variable domain
  // leading to the lowest global cost.
  vector< int > bestValuesList;
  int backupValue = variable->get_value();
  int bestValue;
  double bestCost = std::numeric_limits<double>::max();
  double currentCost;
  
  for( auto& val : variable->possible_values() )
  {
    newCurrentSatCost = simulate_local_move_cost( variable, val, costConstraints, currentSatCost );
    if( bestCost > newCurrentSatCost )
    {
      bestCost = newCurrentSatCost;
      bestValuesList.clear();
      bestValuesList.push_back( val );
    }
    else 
      if( bestCost == newCurrentSatCost )
	bestValuesList.push_back( val );	  
  }

  // If several values lead to the same best global cost,
  // call Objective::heuristic_value has a tie-break.
  // By default, Objective::heuristic_value returns the value
  // improving the most the objective function, or a random value
  // among values improving the most the objective function if there
  // are some ties.
  if( bestValuesList.size() > 1 )
    bestValue = _objective->heuristic_value( bestValuesList );
  else
    bestValue = &bestValuesList[0];

  variable->set_value( value );
  currentSatCost = bestCost;
  for( auto& c : _mapVarCtr[ *variable ] )
    costConstraints[ c->get_id() ] = c->cost();

  compute_variables_costs( costConstraints, costVariables );
}

void Solver::permutation_move( Variable *variable, vector<double>& costConstraints, vector<double>& costVariables, double& currentSatCost )
{
  // Here, we look at values in the variable domain
  // leading to the lowest global cost.
  vector< Variable > bestVarToSwapList;
  int backupValue = variable->get_value();
  Variable bestVarToSwap;
  double bestCost = std::numeric_limits<double>::max();
  double currentCost;
  
  for( auto& otherVariable : _vecVariables )
  {
    if( otherVariable.get_id() == variable->get_id() )
      continue;
    
    newCurrentSatCost = simulate_permutation_cost( variable, otherVariable, costConstraints, currentSatCost );
    if( bestCost > newCurrentSatCost )
    {
      bestCost = newCurrentSatCost;
      bestVarToSwapList.clear();
      bestVarToSwapList.push_back( otherVariable );
    }
    else 
      if( bestCost == newCurrentSatCost )
	bestVarToSwapList.push_back( otherVariable );	  
  }

  // If several values lead to the same best global cost,
  // call Objective::heuristic_value has a tie-break.
  // By default, Objective::heuristic_value returns the value
  // improving the most the objective function, or a random value
  // among values improving the most the objective function if there
  // are some ties.

  if( bestVarToSwapList.size() > 1 )
    bestVarToSwap = _objective->heuristic_value( bestVarToSwapList );
  else
    bestVarToSwap = &bestVarToSwapList[0];

  int tmp = variable->get_value();
  variable->set_value( bestVarToSwap.get_value() );
  bestVarToSwap.set_value( tmp );

  currentSatCost = bestCost;
  vector<bool> compted( costConstraints.size(), false );
  
  for( auto& c : _mapVarCtr[ *variable ] )
  {
    newCurrentSatCost += ( c->cost() - costConstraints[ c->get_id() ] );
    compted[ c->get_id() ] = true;
  }
  
  for( auto& c : _mapVarCtr[ bestVarToSwap ] )
    if( !compted[ c->get_id() ] )
      newCurrentSatCost += ( c->cost() - costConstraints[ c->get_id() ] );

  compute_variables_costs( costConstraints, costVariables );
}
