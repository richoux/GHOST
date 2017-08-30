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


#pragma once

#include <limits>
#include <random>
#include <algorithm>
#include <vector>
#include <map>
#include <chrono>
#include <memory>

#include "variable.hpp"
#include "constraint.hpp"
#include "domain.hpp"
#include "misc/random.hpp"
#include "objective.hpp"

using namespace std;

namespace ghost
{
  //! Solver is the class coding the solver itself.
  /*! 
   * You just need to instanciate one Solver object, then run its 'solve'
   * function.
   *
   * The Solver class is a template class, waiting for both the type
   * of variable and the type of constraint. Thus, you must instanciate 
   * a solver by specifying the class of your variable objects and the 
   * class of your constraint objects, like for instance Solver<MyCustomVariable, 
   * MyCustomConstraint> where MyCustomVariable must inherits 
   * from ghost::Variable and MyCustomConstraint must inherits 
   * from ghost::Constraint.
   *
   * Solver's constructor also need a shared pointer of an Objective
   * object. The reason why Objective is not a template parameter of 
   * Solver but a pointer is to allow a dynamic modification of the 
   * objective function.
   *
   * \sa Variable, Constraint, Objective
   */  
  template <typename TypeVariable, typename TypeConstraint>
  class Solver
  {
    vector<TypeVariable>		*_vecVariables;		// Pointer to the vector of variables.
    vector<TypeConstraint>		*_vecConstraints;	// Pointer to the vector of constraints.
    shared_ptr<Objective<TypeVariable>>	_objective;		// Shared pointer of the objective function.

    vector<int>	_weakTabuList;		// The weak tabu list, frozing used variables for tabuTime iterations. 
    Random	_random;		// The random generator used by the solver.
    double	_bestSatCost;		// The satisfaction cost of the best solution.
    double	_bestSatCostTour;	// The satisfaction cost of the best solution in the current optimization loop.
    double	_bestOptCost;		// The optimization cost of the best solution.
    bool	_isOptimization;	// A boolean to know if it is a satisfaction or optimization run.
    bool	_permutationProblem;	// A boolean to know if it is a permutation problem or not.

    int		_varOffset;		// Offset to shift variables id, sich that the first would be shifted to 0.
    int		_ctrOffset;		// Same for constraints.
    
    struct VarComp
    {
      bool operator()( const TypeVariable& lhs, const TypeVariable& rhs ) const
      {
	return lhs.get_id() < rhs.get_id();
      }
    };
    
    mutable map< TypeVariable,
		 vector<TypeConstraint>,
		 VarComp > _mapVarCtr;	// Map to know in which constraints are each variable.

    // Solver's regular constructor
    /*
     * \param vecVariables A pointer to the vector of Variables.
     * \param vecConstraints A pointer to the vector of Constraints.
     * \param objective A shared pointer to an Objective.
     * \param permutationProblem A boolean indicating if we work on a permutation problem. False by default.
     */
    Solver( vector<TypeVariable>		*vecVariables, 
	    vector<TypeConstraint>		*vecConstraints,
	    shared_ptr<Objective<TypeVariable>>	objective,
	    bool				permutationProblem = false );

    // Set the initial configuration by calling monte_carlo_sampling() 'samplings' times.
    /*
     * After calling calling monte_carlo_sampling() 'samplings' times, the function keeps 
     * the configuration wth the lowest satisfaction cost. If some of them reach 0, it keeps 
     * the configuration with the best optimization cost. 
     */
    void set_initial_configuration( int samplings = 1 );

    // Sample an configuration
    void monte_carlo_sampling();
    
    // Decreasing values in tabuList
    // freeVariables is set to true if there is at least one free variable, ie, untabu.
    void decay_weak_tabu_list( bool& freeVariables );

    // To factorize code like if (best > current) then best=current and update configuration
    void update_better_configuration( double& best, const double current, vector<int>& configuration );

    // Compute the cost of each constraints and fill up the vector costConstraints
    double compute_constraints_costs( vector<double>& costConstraints ) const;

    // Compute the variable cost of each variables and fill up vectors costVariables and costNonTabuVariables 
    void compute_variables_costs( const vector<double>& costConstraints,
				  vector<double>&	costVariables,
				  vector<double>&	costNonTabuVariables,
				  const double currentSatCost ) const;

    // Compute incrementally the now satisfaction cost IF we change the value of 'variable' by 'value' with a local move.
    double simulate_local_move_cost( TypeVariable		*variable,
				     double			value,
				     const vector<double>&	costConstraints,
				     double			currentSatCost ) const;

    // Compute incrementally the now satisfaction cost IF we swap values of 'variable' with another variable.
    double simulate_permutation_cost( TypeVariable		*worstVariable,
				      TypeVariable&		otherVariable,
				      const vector<double>&	costConstraints,
				      double			currentSatCost ) const;

    // Function to make a local move, ie, to assign a given
    void local_move( TypeVariable	*variable,
		     vector<double>&	costConstraints,
		     vector<double>&	costVariables,
		     vector<double>&	costNonTabuVariables,
		     double&		currentSatCost );

    // Function to make a permutation move, ie, to assign a given
    void permutation_move( TypeVariable		*variable,
			   vector<double>&	costConstraints,
			   vector<double>&	costVariables,
			   vector<double>&	costNonTabuVariables,
			   double&		currentSatCost );


  public:
    //! Solver's regular constructor
    /*!
     * \param vecVariables A pointer to the vector of Variables.
     * \param vecConstraints A reference to the vector of Constraints.
     * \param obj A shared pointer to the Objective.
     * \param permutationProblem A boolean indicating if we work on a permutation problem. False by default.
     */
    Solver( vector<TypeVariable>&		vecVariables, 
	    vector<TypeConstraint>&		vecConstraints,
	    shared_ptr<Objective<TypeVariable>>	objecive,
	    bool				permutationProblem = false );

    //! Second Solver's constructor
    /*!
     * \param vecVariables A reference to the vector of Variables.
     * \param vecConstraints A reference to the vector of Constraints.
     * \param permutationProblem A boolean indicating if we work on a permutation problem. False by default.
     */
    Solver( vector< TypeVariable >&	vecVariables, 
	    vector< TypeConstraint >&	vecConstraints,
	    bool			permutationProblem = false );
    
    //! Solver's main function, to solve the given CSP/COP.
    /*!
     * \param finalCost A reference to the double of the sum of constraints cost for satisfaction problems, or the value of the objective function for optimization problems. For satisfaction problems, a cost of zero means a solution has been found.
     * \param finalSolution The configuration of the best solution found, ie, a reference to the vector of assignements of each variable.
     * \param sat_timeout The satisfaction timeout in milliseconds.
     * \param opt_timeout The optimization timeout in milliseconds (optionnal, equals to 10 times sat_timeout is not set).
     * \return True iff a solution has been found.
     */
    bool solve( double& finalCost, vector<int>& finalSolution, double sat_timeout, double opt_timeout = 0. );
  };

  ////////////////////
  // Implementation //
  ////////////////////

  template <typename TypeVariable, typename TypeConstraint>
  Solver<TypeVariable, TypeConstraint>::Solver( vector<TypeVariable>			*vecVariables, 
						vector<TypeConstraint>			*vecConstraints,
						shared_ptr<Objective<TypeVariable>>	objective,
						bool					permutationProblem )
    : _vecVariables		( vecVariables ), 
      _vecConstraints		( vecConstraints ),
      _objective		( objective ),
      _weakTabuList		( vecVariables->size() ),
      _isOptimization		( objective == nullptr ? false : true ),
      _permutationProblem	( permutationProblem )
  {
    for( auto& var : *vecVariables )
      for( auto& ctr : *vecConstraints )
	if( ctr.has_variable( var ) )
	  _mapVarCtr[ var ].push_back( ctr );
  }

  template <typename TypeVariable, typename TypeConstraint>
  Solver<TypeVariable, TypeConstraint>::Solver( vector<TypeVariable>&			vecVariables, 
						vector<TypeConstraint>&			vecConstraints,
						shared_ptr<Objective<TypeVariable>>	objective,
						bool					permutationProblem )
    : Solver( &vecVariables, &vecConstraints, objective, permutationProblem )
  { }

  template <typename TypeVariable, typename TypeConstraint>
  Solver<TypeVariable, TypeConstraint>::Solver( vector<TypeVariable>&	vecVariables, 
						vector<TypeConstraint>& vecConstraints,
						bool			permutationProblem )
    : Solver( &vecVariables, &vecConstraints, nullptr, permutationProblem )
  { }
  

  template <typename TypeVariable, typename TypeConstraint>
  bool Solver<TypeVariable, TypeConstraint>::solve( double&	finalCost,
						    vector<int>& finalSolution,
						    double	satTimeout,
						    double	optTimeout )
  {
    satTimeout *= 1000; // timeouts in microseconds
    if( optTimeout == 0 )
      optTimeout = satTimeout * 10;
    else
      optTimeout *= 1000;

    // The only parameter of Solver<TypeVariable, TypeConstraint>::solve outside timeouts
    int tabuTime = _vecVariables->size() - 1;

    _varOffset = (*_vecVariables)[0].get_id();
    for( auto& v : (*_vecVariables) )
      if( v.get_id() < _varOffset )
	_varOffset = v.get_id();
    
    _ctrOffset = (*_vecConstraints)[0].get_id();
    for( auto& c : (*_vecConstraints) )
      if( c.get_id() < _ctrOffset )
	_ctrOffset = c.get_id();
    
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
      _objective = make_shared< NullObjective<TypeVariable> >();
    
    int optLoop = 0;
    int satLoop = 0;

    double costBeforePostProc = numeric_limits<double>::max();
  
    TypeVariable* worstVariable;
    double currentSatCost;
    double currentOptCost;
    vector< double > costConstraints( _vecConstraints->size(), 0. );
    vector< double > costVariables( _vecVariables->size(), 0. );
    vector< double > costNonTabuVariables( _vecVariables->size(), 0. );

    // In case finalSolution is not a vector of the correct size,
    // ie, equals to the number of variables.
    finalSolution.resize( _vecVariables->size() );
  
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
	compute_variables_costs( costConstraints, costVariables, costNonTabuVariables, currentSatCost );
      
	bool freeVariables = false;
	decay_weak_tabu_list( freeVariables );

	if( freeVariables )
	{
	  discrete_distribution<int> distribution { costNonTabuVariables.begin(), costNonTabuVariables.end() };
	  worstVariable = &(*_vecVariables)[ distribution( rng ) ];
	}
	else
	{
	  discrete_distribution<int> distribution { costVariables.begin(), costVariables.end() };
	  worstVariable = &(*_vecVariables)[ distribution( rng ) ];
	}

	if( _permutationProblem )
	  permutation_move( worstVariable, costConstraints, costVariables, costNonTabuVariables, currentSatCost );
	else
	  local_move( worstVariable, costConstraints, costVariables, costNonTabuVariables, currentSatCost );

	if( _bestSatCostTour > currentSatCost )
	{
	  _bestSatCostTour = currentSatCost;

	  if( _bestSatCost >= _bestSatCostTour )
	    _bestSatCost = _bestSatCostTour;

	  // freeze the variable a bit
	  _weakTabuList[ worstVariable->get_id() - _varOffset ] = (int)(tabuTime / 4);
	}
	else // local minima
	  // Mark worstVariable as weak tabu for tabuTime iterations.
	  _weakTabuList[ worstVariable->get_id() - _varOffset ] = tabuTime;
      
	elapsedTimeOptLoop = chrono::steady_clock::now() - startOptLoop;
	elapsedTime = chrono::steady_clock::now() - start;
      } // satisfaction loop
      while( _bestSatCostTour > 0. && elapsedTimeOptLoop.count() < satTimeout && elapsedTime.count() < optTimeout );

      if( _bestSatCostTour == 0. )
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
    // Useful if the user prefer to directly use the vector of TypeVariables
    // to manipulate and exploit the solution.
    for( auto& v : *_vecVariables )
      v.set_value( finalSolution[ v.get_id() - _varOffset ] );
    
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

  template <typename TypeVariable, typename TypeConstraint>
  double Solver<TypeVariable, TypeConstraint>::compute_constraints_costs( vector<double>& costConstraints ) const
  {
    double satisfactionCost = 0.;
    double cost;
  
    for( auto& c : *_vecConstraints )
    {
      cost = c.cost();
      costConstraints[ c.get_id() - _ctrOffset ] = cost;
      satisfactionCost += cost;    
    }

    return satisfactionCost;
  }

  template <typename TypeVariable, typename TypeConstraint>
  void Solver<TypeVariable, TypeConstraint>::compute_variables_costs( const vector<double>& costConstraints,
								      vector<double>& costVariables,
								      vector<double>& costNonTabuVariables,
								      const double currentSatCost ) const
  {
    int id;
    
    fill( costNonTabuVariables.begin(), costNonTabuVariables.end(), 0. );

    for( auto& v : *_vecVariables )
    {
      id = v.get_id() - _varOffset;
      int ratio = 1;//std::max( 5, (int)v.get_domain_size()/100 );
    
      for( auto& c : _mapVarCtr[ v ] )
	costVariables[ id ] += costConstraints[ c.get_id() - _ctrOffset ];

      // i is initialized just not to be warned by compiler
      int i = 1;
      double sum = 0.;
      
      if( _permutationProblem )
      {
	TypeVariable *otherVariable;
	
	for( i = 0 ; i < ratio; ++i )
	{
	  otherVariable = &(*_vecVariables)[ _random.get_random_number( _vecVariables->size() ) ];
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
      costVariables[ id ] = fabs( costVariables[ id ] - ( sum / ratio ) );
      
      if( _weakTabuList[ id ] == 0 )
	costNonTabuVariables[ id ] = costVariables[ id ];
    }
  }

  template <typename TypeVariable, typename TypeConstraint>
  void Solver<TypeVariable, TypeConstraint>::set_initial_configuration( int samplings )
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
      double bestOptCost = numeric_limits<double>::max();
    
      double currentSatCost;
      double currentOptCost;

      vector<int> bestValues( _vecVariables->size(), 0 );
    
      for( int i = 0 ; i < samplings ; ++i )
      {
	monte_carlo_sampling();
	currentSatCost = 0.;
	for( auto& c : *_vecConstraints )
	  currentSatCost += c.cost();
      
	if( bestSatCost > currentSatCost )
	  update_better_configuration( bestSatCost, currentSatCost, bestValues );
	else
	  if( currentSatCost == 0 )
	    if( _isOptimization )
	    {
	      currentOptCost = _objective->cost( _vecVariables );
	      if( bestOptCost > currentOptCost )
		update_better_configuration( bestOptCost, currentOptCost, bestValues );
	    }
      }

      for( auto& v : *_vecVariables )
	v.set_value( bestValues[ v.get_id() - _varOffset ] );
    }
  }

  template <typename TypeVariable, typename TypeConstraint>
  void Solver<TypeVariable, TypeConstraint>::monte_carlo_sampling()
  {
    for( auto& v : *_vecVariables )
      v.do_random_initialization();
  }

  template <typename TypeVariable, typename TypeConstraint>
  void Solver<TypeVariable, TypeConstraint>::decay_weak_tabu_list( bool& freeVariables ) 
  {
    for( int i = 0 ; i < (int)_weakTabuList.size() ; ++i )
    {
      if( _weakTabuList[i] <= 1 )
      {
	_weakTabuList[i] = 0;
	freeVariables = true;      
      }
      else
	--_weakTabuList[i];
    }
  }

  template <typename TypeVariable, typename TypeConstraint>
  void Solver<TypeVariable, TypeConstraint>::update_better_configuration( double& best,
									  const double current,
									  vector<int>& configuration )
  {
    best = current;

    for( auto& v : *_vecVariables )
      configuration[ v.get_id() - _varOffset ] = v.get_value();
  }
  
  // NO VALUE BACKED-UP!
  template <typename TypeVariable, typename TypeConstraint>
  double Solver<TypeVariable, TypeConstraint>::simulate_local_move_cost( TypeVariable* variable,
									 double value,
									 const vector<double>& costConstraints,
									 double currentSatCost ) const
  {
    double newCurrentSatCost = currentSatCost;

    variable->set_value( value );
    for( auto& c : _mapVarCtr[ *variable ] )
      newCurrentSatCost += ( c.cost() - costConstraints[ c.get_id() - _ctrOffset ] );

    return newCurrentSatCost;
  }

  template <typename TypeVariable, typename TypeConstraint>
  double Solver<TypeVariable, TypeConstraint>::simulate_permutation_cost( TypeVariable* worstVariable,
									  TypeVariable& otherVariable,
									  const vector<double>& costConstraints,
									  double currentSatCost ) const
  {
    double newCurrentSatCost = currentSatCost;
    int tmp = worstVariable->get_value();
    worstVariable->set_value( otherVariable.get_value() );
    otherVariable.set_value( tmp );

    vector<bool> compted( costConstraints.size(), false );
  
    for( auto& c : _mapVarCtr[ *worstVariable ] )
    {
      newCurrentSatCost += ( c.cost() - costConstraints[ c.get_id() - _ctrOffset ] );
      compted[ c.get_id() - _ctrOffset ] = true;
    }
  
    for( auto& c : _mapVarCtr[ otherVariable ] )
      if( !compted[ c.get_id() - _ctrOffset ] )
	newCurrentSatCost += ( c.cost() - costConstraints[ c.get_id() - _ctrOffset ] );

    // We must roll back to the previous state before returning the new cost value. 
    tmp = worstVariable->get_value();
    worstVariable->set_value( otherVariable.get_value() );
    otherVariable.set_value( tmp );

    return newCurrentSatCost;
  }

  template <typename TypeVariable, typename TypeConstraint>
  void Solver<TypeVariable, TypeConstraint>::local_move( TypeVariable* variable,
							 vector<double>& costConstraints,
							 vector<double>& costVariables,
							 vector<double>& costNonTabuVariables,
							 double& currentSatCost )
  {
    // Here, we look at values in the variable domain
    // leading to the lowest satisfaction cost.
    double newCurrentSatCost;
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
      bestValue = _objective->heuristic_value( _vecVariables, variable, bestValuesList );
    else
      bestValue = bestValuesList[0];

    variable->set_value( bestValue );
    currentSatCost = bestCost;
    for( auto& c : _mapVarCtr[ *variable ] )
      costConstraints[ c.get_id() - _ctrOffset ] = c.cost();

    compute_variables_costs( costConstraints, costVariables, costNonTabuVariables, currentSatCost );
  }

  template <typename TypeVariable, typename TypeConstraint>
  void Solver<TypeVariable, TypeConstraint>::permutation_move( TypeVariable* variable,
							       vector<double>& costConstraints,
							       vector<double>& costVariables,
							       vector<double>& costNonTabuVariables,
							       double& currentSatCost )
  {
    // Here, we look at values in the variable domain
    // leading to the lowest satisfaction cost.
    double newCurrentSatCost;
    vector< TypeVariable* > bestVarToSwapList;
    TypeVariable* bestVarToSwap;
    double bestCost = numeric_limits<double>::max();
  
    for( auto& otherVariable : *_vecVariables )
    {
      if( otherVariable.get_id() == variable->get_id() )
	continue;
    
      newCurrentSatCost = simulate_permutation_cost( variable, otherVariable, costConstraints, currentSatCost );
      if( bestCost > newCurrentSatCost )
      {
	bestCost = newCurrentSatCost;
	bestVarToSwapList.clear();
	bestVarToSwapList.push_back( &otherVariable );
      }
      else 
	if( bestCost == newCurrentSatCost )
	  bestVarToSwapList.push_back( &otherVariable );	  
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

    int tmp = variable->get_value();
    variable->set_value( bestVarToSwap->get_value() );
    bestVarToSwap->set_value( tmp );

    currentSatCost = bestCost;
    vector<bool> compted( costConstraints.size(), false );
  
    for( auto& c : _mapVarCtr[ *variable ] )
    {
      newCurrentSatCost += ( c.cost() - costConstraints[ c.get_id() - _ctrOffset ] );
      compted[ c.get_id() - _ctrOffset ] = true;
    }
  
    for( auto& c : _mapVarCtr[ *bestVarToSwap ] )
      if( !compted[ c.get_id() - _ctrOffset ] )
	newCurrentSatCost += ( c.cost() - costConstraints[ c.get_id() - _ctrOffset ] );

    compute_variables_costs( costConstraints, costVariables, costNonTabuVariables, newCurrentSatCost );
  }
}

