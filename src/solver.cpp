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

#include "solver.hpp"

using namespace ghost;

Solver::Solver( vector<Variable>&		vecVariables, 
		vector<shared_ptr<Constraint>>&	vecConstraints,
		shared_ptr<Objective>		objective,
		bool				permutationProblem )
  : _vecVariables	( vecVariables ), 
    _vecConstraints	( vecConstraints ),
    _objective		( objective ),
    _weakTabuList	( vecVariables.size() ),
    _isOptimization	( objective == nullptr ? false : true ),
    _permutationProblem	( permutationProblem ),
    _number_variables	( vecVariables.size() )
{
  for( auto& var : vecVariables )
    for( auto& ctr : vecConstraints )
      if( ctr->has_variable( var ) )
	_mapVarCtr[ var ].push_back( ctr );
}

Solver::Solver( vector<Variable>&		vecVariables, 
		vector<shared_ptr<Constraint>>&	vecConstraints,
		bool				permutationProblem )
  : Solver( vecVariables, vecConstraints, nullptr, permutationProblem )
{ }
  

bool Solver::solve( double&	finalCost,
		    vector<int>& finalSolution,
		    double	satTimeout,
		    double	optTimeout )
{
  //satTimeout *= 1000; // timeouts in microseconds
  if( optTimeout == 0 )
    optTimeout = satTimeout * 10;
  //else
  //optTimeout *= 1000;

  // The only parameter of Solver<Variable, Constraint>::solve outside timeouts
  int tabuTimeLocalMin = _number_variables - 1;
  int tabuTimeSelected = std::max( 1, tabuTimeLocalMin / 2);

  _varOffset = _vecVariables[0]._id;
  for( auto& v : _vecVariables )
    if( v._id < _varOffset )
      _varOffset = v._id;
    
  _ctrOffset = _vecConstraints[0]->get_id();
  for( auto& c : _vecConstraints )
    if( c->get_id() < _ctrOffset )
      _ctrOffset = c->get_id();
    
  chrono::duration<double,micro> elapsedTime(0);
  chrono::duration<double,micro> elapsedTimeOptLoop(0);
  chrono::time_point<chrono::steady_clock> start;
  chrono::time_point<chrono::steady_clock> startOptLoop;
  chrono::time_point<chrono::steady_clock> startPostprocess;
  start = chrono::steady_clock::now();

  chrono::duration<double,micro> timerPostProcessSat(0);
  chrono::duration<double,micro> timerPostProcessOpt(0);

  random_device	rd;
  mt19937		rng( rd() );

  if( _objective == nullptr )
    _objective = make_shared< NullObjective >();
    
  int optLoop = 0;
  int satLoop = 0;

  double costBeforePostProc = numeric_limits<double>::max();
  
  Variable* worstVariable;
  double currentSatCost;
  double currentOptCost;
  vector< double > costConstraints( _vecConstraints.size(), 0. );
  vector< double > costVariables( _number_variables, 0. );
  vector< double > costNonTabuVariables( _number_variables, 0. );

  // In case finalSolution is not a vector of the correct size,
  // ie, equals to the number of variables.
  finalSolution.resize( _number_variables );
  
  _bestSatCost = numeric_limits<double>::max();
  _bestOptCost = numeric_limits<double>::max();
  
  do // optimization loop
  {
    startOptLoop = chrono::steady_clock::now();
    ++optLoop;

    // start from a random configuration
    set_initial_configuration( 10 );
    
    // Reset weak tabu list
    fill( _weakTabuList.begin(), _weakTabuList.end(), 0 );

    // Reset the best satisfaction cost
    _bestSatCostOptLoop = numeric_limits<double>::max();

    do // satisfaction loop 
    {
      ++satLoop;

      // Reset variables and constraints costs
      fill( costConstraints.begin(), costConstraints.end(), 0. );
      fill( costVariables.begin(), costVariables.end(), 0. );

      currentSatCost = compute_constraints_costs( costConstraints );
      compute_variables_costs( costConstraints, costVariables, costNonTabuVariables, currentSatCost );
      
      bool freeVariables = false;
      decay_weak_tabu_list( freeVariables );

#if defined(ADAPTIVE_SEARCH)
      auto worstVariableList = compute_worst_variables( freeVariables, costVariables );
      if( worstVariableList.size() > 1 )
	worstVariable = worstVariableList[ _random.get_random_number( worstVariableList.size() ) ];
      else
	worstVariable = worstVariableList[0];      
#else
      if( freeVariables )
      {
	discrete_distribution<int> distribution { costNonTabuVariables.begin(), costNonTabuVariables.end() };
	worstVariable = &(_vecVariables[ distribution( rng ) ]);
      }
      else
      {
	discrete_distribution<int> distribution { costVariables.begin(), costVariables.end() };
	worstVariable = &(_vecVariables[ distribution( rng ) ]);
      }      
#endif
      
      if( _permutationProblem )
	permutation_move( worstVariable, costConstraints, costVariables, costNonTabuVariables, currentSatCost );
      else
	local_move( worstVariable, costConstraints, costVariables, costNonTabuVariables, currentSatCost );

      if( _bestSatCostOptLoop > currentSatCost )
      {
	_bestSatCostOptLoop = currentSatCost;

	if( _bestSatCost >= _bestSatCostOptLoop )
	  _bestSatCost = _bestSatCostOptLoop;

	// freeze the variable a bit
	_weakTabuList[ worstVariable->_id - _varOffset ] = tabuTimeSelected;
      }
      else // local minima
	// Mark worstVariable as weak tabu for tabuTimeLocalMin iterations.
	_weakTabuList[ worstVariable->_id - _varOffset ] = tabuTimeLocalMin;
      
      elapsedTimeOptLoop = chrono::steady_clock::now() - startOptLoop;
      elapsedTime = chrono::steady_clock::now() - start;
    } // satisfaction loop
    while( _bestSatCostOptLoop > 0. && elapsedTimeOptLoop.count() < satTimeout && elapsedTime.count() < optTimeout );

    if( _bestSatCostOptLoop == 0. )
    {
      currentOptCost = _objective->cost( _vecVariables );
      if( _bestOptCost > currentOptCost )
      {
	update_better_configuration( _bestOptCost, currentOptCost, finalSolution );

	startPostprocess = chrono::steady_clock::now();
	_objective->postprocess_satisfaction( _vecVariables, _bestOptCost, finalSolution );
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
    _objective->postprocess_optimization( _vecVariables, _bestOptCost, finalSolution );
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
  // Useful if the user prefer to directly use the vector of Variables
  // to manipulate and exploit the solution.
  for( auto& v : _vecVariables )
    v.set_value( finalSolution[ v._id - _varOffset ] );
    
#if defined(DEBUG) || defined(BENCH)
  cout << "############" << "\n";
      
  if( !_isOptimization )
    cout << "SATISFACTION run" << "\n";
  else
    cout << "OPTIMIZATION run with objective " << _objective->get_name() << "\n";

  cout << "Elapsed time: " << elapsedTime.count() / 1000 << "\n"
       << "Satisfaction cost: " << _bestSatCost << "\n"
       << "Number of optization loops: " << optLoop << "\n"
       << "Number of satisfaction loops: " << satLoop << "\n";

  if( _isOptimization )
    cout << "Optimization cost: " << _bestOptCost << "\n"
	 << "Opt Cost BEFORE post-processing: " << costBeforePostProc << "\n";
  
  if( timerPostProcessSat.count() > 0 )
    cout << "Satisfaction post-processing time: " << timerPostProcessSat.count() / 1000 << "\n"; 

  if( timerPostProcessOpt.count() > 0 )
    cout << "Optimization post-processing time: " << timerPostProcessOpt.count() / 1000 << "\n"; 

  cout << "\n";
#endif
          
  return _bestSatCost == 0.;
}

double Solver::compute_constraints_costs( vector<double>& costConstraints ) const
{
  double satisfactionCost = 0.;
  double cost;
  
  for( auto& c : _vecConstraints )
  {
    cost = c->cost();
    costConstraints[ c->get_id() - _ctrOffset ] = cost;
    satisfactionCost += cost;    
  }

  return satisfactionCost;
}

void Solver::compute_variables_costs( const vector<double>&	costConstraints,
				      vector<double>&		costVariables,
				      vector<double>&		costNonTabuVariables,
				      const double		currentSatCost ) const
{
  int id;
    
  fill( costNonTabuVariables.begin(), costNonTabuVariables.end(), 0. );

  for( auto& v : _vecVariables )
  {
    id = v._id - _varOffset;
    int ratio = std::max( 5, (int)v.get_domain_size()/100 );
    
    for( auto& c : _mapVarCtr[ v ] )
      costVariables[ id ] += costConstraints[ c->get_id() - _ctrOffset ];

    // i is initialized just not to be warned by compiler
    int i = 1;
    double sum = 0.;
      
    if( _permutationProblem )
    {
      Variable *otherVariable;
	
      for( i = 0 ; i < ratio; ++i )
      {
	otherVariable = &(_vecVariables[ _random.get_random_number( _number_variables ) ]);
	sum += simulate_permutation_cost( &v, *otherVariable, costConstraints, currentSatCost );
      }
    }
    else
    {
      int backup = v.get_value();
      int value;
      auto domain = v.possible_values();
	
      for( i = 0 ; i < ratio; ++i )
      {
	value = domain[ _random.get_random_number( domain.size() ) ];
	sum += simulate_local_move_cost( &v, value, costConstraints, currentSatCost );
      }
	
      v.set_value( backup );
    }
      
    // sum / i is the mean
    // costVariables[ id ] = fabs( costVariables[ id ] - ( sum / ratio ) );
    costVariables[ id ] = std::max( 0., costVariables[ id ] * ( ( sum / ratio ) / currentSatCost ) );
            
    if( _weakTabuList[ id ] == 0 )
      costNonTabuVariables[ id ] = costVariables[ id ];
  }
}

void Solver::set_initial_configuration( int samplings )
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
    double currentSatCost;

    vector<int> bestValues( _number_variables, 0 );
    
    for( int i = 0 ; i < samplings ; ++i )
    {
      monte_carlo_sampling();
      currentSatCost = 0.;
      for( auto& c : _vecConstraints )
	currentSatCost += c->cost();
      
      if( bestSatCost > currentSatCost )
	update_better_configuration( bestSatCost, currentSatCost, bestValues );

      if( currentSatCost == 0. )
	break;
    }

    for( int i = 0; i < _number_variables; ++i )
      _vecVariables[ i ].set_value( bestValues[ i ] );
    
    // for( auto& v : _vecVariables )
    //   v.set_value( bestValues[ v._id - _varOffset ] );
  }
}

void Solver::monte_carlo_sampling()
{
  for( auto& v : _vecVariables )
    v.do_random_initialization();
}

void Solver::decay_weak_tabu_list( bool& freeVariables ) 
{
  for( int i = 0 ; i < (int)_weakTabuList.size() ; ++i )
  {
    if( _weakTabuList[i] == 0 )
      freeVariables = true;
    else
      --_weakTabuList[i];

    assert( _weakTabuList[i] >= 0 );
  }
}

void Solver::update_better_configuration( double&	best,
					  const double	current,
					  vector<int>&	configuration )
{
  best = current;

  for( int i = 0; i < _number_variables; ++i )
    configuration[ i ] = _vecVariables[ i ].get_value();
  // for( auto& v : _vecVariables )
  //   configuration[ v._id - _varOffset ] = v.get_value();
}

#if defined(ADAPTIVE_SEARCH)
vector< Variable* > Solver::compute_worst_variables( bool freeVariables,
						     const vector<double>& costVariables )
{
  // Here, we look at neighbor configurations with the lowest cost.
  vector< Variable* > worstVariableList;
  double worstVariableCost = 0.;
  int id;
  
  for( auto& v : _vecVariables )
  {
    id = v._id - _varOffset;
    if( !freeVariables || _weakTabuList[ id ] == 0 )
    {
      if( worstVariableCost < costVariables[ id ] )
      {
	worstVariableCost = costVariables[ id ];
	worstVariableList.clear();
	worstVariableList.push_back( &v );
      }
      else 
	if( worstVariableCost == costVariables[ id ] )
	  worstVariableList.push_back( &v );	  
    }
  }
  
  return worstVariableList;

#endif
  
// NO VALUE BACKED-UP!
double Solver::simulate_local_move_cost( Variable*		variable,
					 double			value,
					 const vector<double>&	costConstraints,
					 double			currentSatCost ) const
{
  double newCurrentSatCost = currentSatCost;

  variable->set_value( value );
  for( auto& c : _mapVarCtr[ *variable ] )
    newCurrentSatCost += ( c->cost() - costConstraints[ c->get_id() - _ctrOffset ] );

  return newCurrentSatCost;
}

double Solver::simulate_permutation_cost( Variable*		worstVariable,
					  Variable&		otherVariable,
					  const vector<double>&	costConstraints,
					  double		currentSatCost ) const
{
  double newCurrentSatCost = currentSatCost;
  // int tmp = worstVariable->get_value();
  // worstVariable->set_value( otherVariable.get_value() );
  // otherVariable.set_value( tmp );
  std::swap( worstVariable->_index, otherVariable._index );
  std::swap( worstVariable->_cache_value, otherVariable._cache_value );
  
  vector<bool> compted( costConstraints.size(), false );
  
  for( auto& c : _mapVarCtr[ *worstVariable ] )
  {
    newCurrentSatCost += ( c->cost() - costConstraints[ c->get_id() - _ctrOffset ] );
    compted[ c->get_id() - _ctrOffset ] = true;
  }
  
  for( auto& c : _mapVarCtr[ otherVariable ] )
    if( !compted[ c->get_id() - _ctrOffset ] )
      newCurrentSatCost += ( c->cost() - costConstraints[ c->get_id() - _ctrOffset ] );

  // We must roll back to the previous state before returning the new cost value. 
  // tmp = worstVariable->get_value();
  // worstVariable->set_value( otherVariable.get_value() );
  // otherVariable.set_value( tmp );
  std::swap( worstVariable->_index, otherVariable._index );
  std::swap( worstVariable->_cache_value, otherVariable._cache_value );

  return newCurrentSatCost;
}

void Solver::local_move( Variable*		variable,
			 vector<double>&	costConstraints,
			 vector<double>&	costVariables,
			 vector<double>&	costNonTabuVariables,
			 double&		currentSatCost )
{
  // Here, we look at values in the variable domain
  // leading to the lowest satisfaction cost.
  double newCurrentSatCost = 0.0;
  vector< int > bestValuesList;
  int bestValue;
  double bestCost = numeric_limits<double>::max();
  
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

  // If several values lead to the same best satisfaction cost,
  // call Objective::heuristic_value has a tie-break.
  // By default, Objective::heuristic_value returns the value
  // improving the most the optimization cost, or a random value
  // among values improving the most the optimization cost if there
  // are some ties.
  if( bestValuesList.size() > 1 )
    bestValue = _objective->heuristic_value( _vecVariables, *variable, bestValuesList );
  else
    bestValue = bestValuesList[0];

  variable->set_value( bestValue );
  currentSatCost = bestCost;
  // for( auto& c : _mapVarCtr[ *variable ] )
  //   costConstraints[ c->get_id() - _ctrOffset ] = c->cost();

  // compute_variables_costs( costConstraints, costVariables, costNonTabuVariables, currentSatCost );
}

void Solver::permutation_move( Variable*	variable,
			       vector<double>&	costConstraints,
			       vector<double>&	costVariables,
			       vector<double>&	costNonTabuVariables,
			       double&		currentSatCost )
{
  // Here, we look at values in the variable domain
  // leading to the lowest satisfaction cost.
  double newCurrentSatCost = 0.0;
  vector< Variable > bestVarToSwapList;
  Variable bestVarToSwap;
  double bestCost = numeric_limits<double>::max();
  
  for( auto& otherVariable : _vecVariables )
  {
    if( otherVariable._id == variable->_id )
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

  // If several values lead to the same best satisfaction cost,
  // call Objective::heuristic_value has a tie-break.
  // By default, Objective::heuristic_value returns the value
  // improving the most the optimization cost, or a random value
  // among values improving the most the optimization cost if there
  // are some ties.
  if( bestVarToSwapList.size() > 1 )
    bestVarToSwap = _objective->heuristic_value( bestVarToSwapList );
  else
    bestVarToSwap = bestVarToSwapList[0];

  // int tmp = variable->get_value();
  // variable->set_value( bestVarToSwap.get_value() );
  // bestVarToSwap.set_value( tmp );
  std::swap( variable->_index, bestVarToSwap._index );
  std::swap( variable->_cache_value, bestVarToSwap._cache_value );

  currentSatCost = bestCost;
  vector<bool> compted( costConstraints.size(), false );
  
  for( auto& c : _mapVarCtr[ *variable ] )
  {
    newCurrentSatCost += ( c->cost() - costConstraints[ c->get_id() - _ctrOffset ] );
    compted[ c->get_id() - _ctrOffset ] = true;
  }
  
  for( auto& c : _mapVarCtr[ bestVarToSwap ] )
    if( !compted[ c->get_id() - _ctrOffset ] )
      newCurrentSatCost += ( c->cost() - costConstraints[ c->get_id() - _ctrOffset ] );

  compute_variables_costs( costConstraints, costVariables, costNonTabuVariables, newCurrentSatCost );
}
