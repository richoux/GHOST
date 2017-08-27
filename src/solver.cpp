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
#include <random>
#include <algorithm>
#include <chrono>

#include "solver.hpp"

using namespace std;
using namespace ghost;

template <typename TypeVariable>
Solver<TypeVariable>::Solver( const vector< TypeVariable >& vecVariables, 
			      const vector< Constraint<TypeVariable> >& vecConstraints,
			      Objective<TypeVariable> objective,
			      bool permutationProblem )
  : _vecVariables	( vecVariables ), 
    _vecConstraints	( vecConstraints ),
    _objective		( objective ),
    _weakTabuList	( vecVariables.size() ),
    _isOptimization	( true ),
    _permutationProblem	( permutationProblem )
{
  for( auto& var : vecVariables )
    for( auto& ctr : vecConstraints )
      if( ctr->has_variable( var ) )
	_mapVarCtr[ var ].push_back( ctr );
  // _mapVarCtr[ var ].push_back( make_pair( ctr, ctr->get_variable_iterator( var ) ) );
}

template <typename TypeVariable>
Solver<TypeVariable>::Solver( const vector< TypeVariable >& vecVariables, 
			      const vector< Constraint<TypeVariable> >& vecConstraints,
			      bool permutationProblem )
  : Solver( vecVariables, vecConstraints, NullObjective<TypeVariable>(), permutationProblem )
{
  _isOptimization = false;
}

template <typename TypeVariable>
bool Solver<TypeVariable>::solve( double& finalCost, vector<int>& finalSolution, double satTimeout, double optTimeout )
{
  satTimeout *= 1000; // timeouts in microseconds
  if( optTimeout == 0 )
    optTimeout = satTimeout * 10;
  else
    optTimeout *= 1000;

  // The only parameter of Solver<TypeVariable>::solve outside timeouts
  int tabuTime = _vecVariables.size() - 1;

  chrono::duration<double,micro> elapsedTime(0);
  chrono::duration<double,micro> elapsedTimeOptLoop(0);
  chrono::time_point<chrono::steady_clock> start;
  chrono::time_point<chrono::steady_clock> startOptLoop;
  chrono::time_point<chrono::steady_clock> startPostprocess;
  start = chrono::steady_clock::now();

  chrono::duration<double,micro> timerPostProcessSat(0);
  chrono::duration<double,micro> timerPostProcessOpt(0);

  random_device	rd;
  mt19937	rng( rd() );
      
  int optLoop = 0;
  int satLoop = 0;

  double costBeforePostProc = numeric_limits<double>::max();
  
  vector< TypeVariable > worstVariableList;
  TypeVariable worstVariable;
  double currentSatCost;
  double currentOptCost;
  vector< double > costConstraints( _vecConstraints.size(), 0. );
  vector< double > costVariables( _vecVariables.size(), 0. );
  vector< double > costNonTabuVariables( _vecVariables.size(), 0. );

  // In case finalSolution is not a vector of the correct size,
  // ie, equals to the number of variables.
  finalSolution.resize( _vecVariables.size() );
  
  _bestSatCost = numeric_limits<double>::max();
  _bestOptCost = numeric_limits<double>::max();
  
  do // optimization loop
  {
    startOptLoop = chrono::steady_clock::now();
    ++optLoop;

    // start from a random configuration
    set_initial_configuration( 100 );
    
    // Reset weak tabu list
    fill( _weakTabuList.begin(), _weakTabuList.end(), 0 );

    // Reset the best satisfaction cost
    _bestSatCostTour = numeric_limits<double>::max();

    do // satisfaction loop 
    {
      ++satLoop;

      // Reset variables and constraints costs
      fill( costConstraints.begin(), costConstraints.end(), 0. );
      fill( costVariables.begin(), costVariables.end(), 0. );

      currentSatCost = compute_constraints_costs( costConstraints );
      compute_variables_costs( costConstraints, costVariables, costNonTabuVariables );
      
      bool freeVariables = false;
      decay_weak_tabu_list( freeVariables );

      worstVariableList = compute_worst_variables( freeVariables, costVariables );

      // If several variables share the same worst variable cost,
      // call Objective::heuristic_variable has a tie-break.
      // By default, Objective::heuristic_variable returns a random variable
      // among the vector of TypeVariables given in argument.
      if( worstVariableList.size() > 1 )
      	worstVariable = _objective.heuristic_variable( worstVariableList );
      else
      	worstVariable = worstVariableList[0];

      // if( freeVariables )
      // {
      // 	discrete_distribution<int> distribution { costNonTabuVariables.begin(), costNonTabuVariables.end() };
      // 	worstVariable = _vecVariables[ distribution( rng ) ];
      // }
      // else
      // {
      // 	discrete_distribution<int> distribution { costVariables.begin(), costVariables.end() };
      // 	worstVariable = _vecVariables[ distribution( rng ) ];
      // }
      
      if( _permutationProblem )
	permutation_move( worstVariable, costConstraints, costVariables, costNonTabuVariables, currentSatCost );
      else
	local_move( worstVariable, costConstraints, costVariables, costNonTabuVariables, currentSatCost );

      if( _bestSatCostTour > currentSatCost )
      {
	_bestSatCostTour = currentSatCost;

	if( _bestSatCost >= _bestSatCostTour )
	{
	  _bestSatCost = _bestSatCostTour;
	  // see the right <algorithm> function
	  cout << "Change SAT (" << _bestSatCost << ")\n";
	  for( auto& v : _vecVariables )
	  {
	    finalSolution[ v.get_id() ] = v.get_value();
	    cout << v.get_name() << ": " << v.get_value() << "\n";
	  }
	  cout << "\n";
	  // fill( finalSolution.begin(), finalSolution.end(), [](){})
	}
      }
      else // local minima
	// Mark worstVariable as weak tabu for tabuTime iterations.
	_weakTabuList[ worstVariable.get_id() ] = tabuTime;
      
      elapsedTimeOptLoop = chrono::steady_clock::now() - startOptLoop;
      elapsedTime = chrono::steady_clock::now() - start;
    } // satisfaction loop
    while( _bestSatCostTour > 0. && elapsedTimeOptLoop.count() < satTimeout && elapsedTime.count() < optTimeout );

    if( _bestSatCostTour == 0. )
    {
      currentOptCost = _objective.cost( _vecVariables );
      if( _bestOptCost > currentOptCost )
      {
	_bestOptCost = currentOptCost;
	cout << "Change OPT (" << _bestOptCost << ")\n";
	for( auto& v : _vecVariables )
	{
	  finalSolution[ v.get_id() ] = v.get_value();
	  cout << v.get_name() << ": " << v.get_value() << "\n";
	}
	cout << "\n";
	
	startPostprocess = chrono::steady_clock::now();
	_objective.postprocess_satisfaction( _vecVariables, _bestOptCost, finalSolution );
	timerPostProcessSat = chrono::steady_clock::now() - startPostprocess;
      }
    }
    
    elapsedTime = chrono::steady_clock::now() - start;
  } // optimization loop
  while( elapsedTime.count() < optTimeout && _isOptimization );

  if( _bestSatCost == 0. && _isOptimization )
  {
    costBeforePostProc = _bestOptCost;

    startPostprocess = chrono::steady_clock::now();
    _objective.postprocess_optimization( _vecVariables, _bestOptCost, finalSolution );
    timerPostProcessOpt = chrono::steady_clock::now() - startPostprocess;							     
  }

  if( _isOptimization )
  {
    if( _bestOptCost < 0 )
    {
      _bestOptCost = -_bestOptCost;
      costBeforePostProc = -costBeforePostProc;
    }
    
    finalCost = _bestOptCost;
  }
  else
    finalCost = _bestSatCost;

  // Set the variables to the best solution values.
  // Useful if the user prefer to directly use the vector of TypeVariables
  // to manipulate and exploit the solution.
  for( auto& v : _vecVariables )
    v.set_value( finalSolution[ v.get_id() ] );

#ifndef NDEBUG
  cout << "############" << endl;
      
  if( !_isOptimization )
    cout << "SATISFACTION run" << endl;
  else
    cout << "OPTIMIZATION run with objective " << _objective.get_name() << endl;

  cout << "Elapsed time: " << elapsedTime.count() / 1000 << endl
       << "Global cost: " << _bestSatCost << endl
       << "Number of optization loops: " << optLoop << endl
       << "Number of satisfaction loops: " << satLoop << endl;

  if( _isOptimization )
    cout << "Optimization cost: " << _bestOptCost << endl
	 << "Opt Cost BEFORE post-processing: " << costBeforePostProc << endl;
  
  if( timerPostProcessSat.count() > 0 )
    cout << "Satisfaction post-processing time: " << timerPostProcessSat.count() / 1000 << endl; 

  if( timerPostProcessOpt.count() > 0 )
    cout << "Optimization post-processing time: " << timerPostProcessOpt.count() / 1000 << endl; 

  cout << endl;
#endif
          
  return _bestSatCost == 0.;
}

template <typename TypeVariable>
double Solver<TypeVariable>::compute_constraints_costs( vector<double>& costConstraints ) const
{
  double globalCost = 0.;
  double cost;
  
  for( auto& c : _vecConstraints )
  {
    cost = c.cost();
    costConstraints[ c.get_id() ] = cost;
    globalCost += cost;    
  }

  return globalCost;
}

template <typename TypeVariable>
void Solver<TypeVariable>::compute_variables_costs( const vector<double>& costConstraints,
						    vector<double>& costVariables,
						    vector<double>& costNonTabuVariables ) const
{
  int id;

  fill( costNonTabuVariables.begin(), costNonTabuVariables.end(), 0. );

  for( auto& v : _vecVariables )
  {
    id = v.get_id();
    
    for( auto& c : _mapVarCtr[ v ] )
      costVariables[ id ] += costConstraints[ c.get_id() ];

    if( _isOptimization )
    {
      
    }
    
    if( _weakTabuList[ id ] == 0 )
      costNonTabuVariables[ id ] = costVariables[ id ];
  }
}

template <typename TypeVariable>
void Solver<TypeVariable>::set_initial_configuration( int samplings )
{
  if( samplings == 1 )
  {
    monte_carlo_sampling();
  }
  else
  {
    // To avoid weird samplings numbers like 0 or -1
    samplings = std::max( 2, samplings );
    
    double bestSatCost = numeric_limits<double>::max();
    double bestObjCost = numeric_limits<double>::max();
    
    double currentSatCost;
    double currentObjCost;

    vector<int> bestValues( _vecVariables.size(), 0 );
    
    for( int i = 0 ; i < samplings ; ++i )
    {
      monte_carlo_sampling();
      currentSatCost = 0.;
      for( auto& c : _vecConstraints )
	currentSatCost += c.cost();

      // cout << "currentSatCost: " << currentSatCost << "\n";
      
      if( bestSatCost > currentSatCost )
      {
	bestSatCost = currentSatCost;
	// cout << "bestSatCost: " << bestSatCost << "\n";
	for( auto& v : _vecVariables )
	  bestValues[ v.get_id() ] = v.get_value();
      }
      else
	if( currentSatCost == 0 )
	  if( _isOptimization )
	  {
	    currentObjCost = _objective.cost( _vecVariables );
	    // cout << "currentObjCost: " << currentObjCost << "\n";
	    if( bestObjCost > currentObjCost )
	    {
	      bestObjCost = currentObjCost;
	      // cout << "bestObjCost: " << bestObjCost << "\n";
	      for( auto& v : _vecVariables )
		bestValues[ v.get_id() ] = v.get_value();
	    }	    
	  }
    }

    // cout << "Best values: ";
    for( auto& v : _vecVariables )
    {
      // cout << bestValues[ v->get_id() ] << " ";
      v.set_value( bestValues[ v.get_id() ] );
    }
    // cout << "\n\n";
  }
}

template <typename TypeVariable>
void Solver<TypeVariable>::monte_carlo_sampling()
{
  for( auto& v : _vecVariables )
    v.do_random_initialization();
}

template <typename TypeVariable>
void Solver<TypeVariable>::decay_weak_tabu_list( bool& freeVariables ) 
{
  for( auto& tabu : _weakTabuList )
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

template <typename TypeVariable>
vector< TypeVariable > Solver<TypeVariable>::compute_worst_variables( bool freeVariables, const vector<double>& costVariables ) const
{
  // Here, we look at neighbor configurations with the lowest cost.
  vector< TypeVariable > worstVariableList;
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
template <typename TypeVariable>
double Solver<TypeVariable>::simulate_local_move_cost( TypeVariable& variable,
						       double value,
						       vector<double>& costConstraints,
						       double currentSatCost ) const
{
  double newCurrentSatCost = currentSatCost;

  variable.set_value( value );
  for( auto& c : _mapVarCtr[ variable ] )
    newCurrentSatCost += ( c.cost() - costConstraints[ c.get_id() ] );

  return newCurrentSatCost;
}

template <typename TypeVariable>
double Solver<TypeVariable>::simulate_permutation_cost( TypeVariable& worstVariable,
							TypeVariable& otherVariable,
							vector<double>& costConstraints,
							double currentSatCost ) const
{
  double newCurrentSatCost = currentSatCost;
  int tmp = worstVariable.get_value();
  worstVariable.set_value( otherVariable.get_value() );
  otherVariable.set_value( tmp );

  vector<bool> compted( costConstraints.size(), false );
  
  for( auto& c : _mapVarCtr[ worstVariable ] )
  {
    newCurrentSatCost += ( c.cost() - costConstraints[ c.get_id() ] );
    compted[ c.get_id() ] = true;
  }
  
  for( auto& c : _mapVarCtr[ otherVariable ] )
    if( !compted[ c.get_id() ] )
      newCurrentSatCost += ( c.cost() - costConstraints[ c.get_id() ] );

  // We must roll back to the previous state before returning the new cost value. 
  tmp = worstVariable.get_value();
  worstVariable.set_value( otherVariable->get_value() );
  otherVariable.set_value( tmp );

  return newCurrentSatCost;
}

template <typename TypeVariable>
void Solver<TypeVariable>::local_move( TypeVariable& variable,
				       vector<double>& costConstraints,
				       vector<double>& costVariables,
				       vector<double>& costNonTabuVariables,
				       double& currentSatCost )
{
  // Here, we look at values in the variable domain
  // leading to the lowest global cost.
  double newCurrentSatCost;
  vector< int > bestValuesList;
  int bestValue;
  double bestCost = numeric_limits<double>::max();
  
  for( auto& val : variable.possible_values() )
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
    bestValue = _objective.heuristic_value( _vecVariables, variable, bestValuesList );
  else
    bestValue = bestValuesList[0];

  variable.set_value( bestValue );
  currentSatCost = bestCost;
  for( auto& c : _mapVarCtr[ variable ] )
    costConstraints[ c.get_id() ] = c.cost();

  compute_variables_costs( costConstraints, costVariables, costNonTabuVariables );
}

template <typename TypeVariable>
void Solver<TypeVariable>::permutation_move( TypeVariable& variable,
					     vector<double>& costConstraints,
					     vector<double>& costVariables,
					     vector<double>& costNonTabuVariables,
					     double& currentSatCost )
{
  // Here, we look at values in the variable domain
  // leading to the lowest global cost.
  double newCurrentSatCost;
  vector< TypeVariable > bestVarToSwapList;
  TypeVariable bestVarToSwap;
  double bestCost = numeric_limits<double>::max();
  
  for( auto& otherVariable : _vecVariables )
  {
    if( otherVariable.get_id() == variable.get_id() )
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
    bestVarToSwap = _objective.heuristic_value( bestVarToSwapList );
  else
    bestVarToSwap = bestVarToSwapList[0];

  int tmp = variable.get_value();
  variable.set_value( bestVarToSwap.get_value() );
  bestVarToSwap.set_value( tmp );

  currentSatCost = bestCost;
  vector<bool> compted( costConstraints.size(), false );
  
  for( auto& c : _mapVarCtr[ variable ] )
  {
    newCurrentSatCost += ( c.cost() - costConstraints[ c.get_id() ] );
    compted[ c.get_id() ] = true;
  }
  
  for( auto& c : _mapVarCtr[ bestVarToSwap ] )
    if( !compted[ c.get_id() ] )
      newCurrentSatCost += ( c.cost() - costConstraints[ c.get_id() ] );

  compute_variables_costs( costConstraints, costVariables, costNonTabuVariables );
}
