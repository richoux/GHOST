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


#pragma once

#include <vector>
#include <set>
#include <memory>
#include <cmath>
#include <chrono>
#include <ctime>
#include <limits>
#include <algorithm>
#include <functional>
#include <cassert>
#include <typeinfo>

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
   * You just need to instanciate one Solver object.
   *
   * The Solver class is a template class, waiting for both the type
   * of variable, the type of domain and the type of constraint. Thus,
   * you must instanciate a solver by specifying the class of your
   * variable objects, the class of your domain object and the class
   * of your constraint objects, like for instance Solver<Variable,
   * Domain, Constraint> or Solver<MyCustomVariable, MyCustomDomain,
   * MyCustomConstraint>, if MyCustomVariable inherits from the
   * ghost::Variable class, MyCustomDomain inherits from the
   * ghost::Domain class and MyCustomConstraint inherits from the
   * ghost::Constraint class.
   *
   * Solver's constructor also need a shared pointer of an Objective
   * object (nullptr by default). The reason why Objective is not a
   * template parameter of Solver but a pointer is to allow a dynamic
   * modification of the objective function.
   *
   * \sa Variable, Domain, Constraint, Objective
   */
  template <typename TypeVariable, typename TypeDomain, typename TypeConstraint>
  class Solver
  {
  public:
    //! Solver's regular constructor
    /*!
     * The solver is calling Solver(vecVariables, domain, vecConstraints, obj, 0)
     *
     * \param vecVariables A pointer to the vector of variable objects of the CSP/COP.
     * \param domain A pointer to the domain object of the CSP/COP.
     * \param vecConstraints A constant reference to the vector of shared pointers of Constraint
     * \param obj A reference to the shared pointer of an Objective object. Default value is nullptr.
     */
    Solver( vector< TypeVariable > *vecVariables, 
	    TypeDomain *domain,
	    const vector< shared_ptr<TypeConstraint> > &vecConstraints,
	    const shared_ptr< Objective<TypeVariable, TypeDomain> > &obj = nullptr )
      : Solver(vecVariables, domain, vecConstraints, obj, 0){  }

    //! Solver's constructor mostly used for tests.
    /*!
     * Like the regular constructor, but take also a loops parameter
     * to repeat loops times to satisfaction loop inside
     * Solver::solve. This is mostly used for tests and runtime
     * performance measures.
     *
     * \param vecVariables A pointer to the vector of variable objects of the CSP/COP.
     * \param domain A pointer to the domain object of the CSP/COP.
     * \param vecConstraints A constant reference to the vector of shared pointers of Constraint
     * \param obj A reference to the shared pointer of an Objective object. Default value is nullptr.
     * \param loops The number of times we want to repeat the satisaction loop inside Solver::solve. 
     */
    Solver( vector< TypeVariable > *vecVariables, 
	    TypeDomain *domain,
	    const vector< shared_ptr<TypeConstraint> > &vecConstraints,
	    const shared_ptr< Objective<TypeVariable, TypeDomain> > &obj,
	    const int loops )
      : vecVariables(vecVariables), 
	domain(domain),
	vecConstraints(vecConstraints),
	objective(obj),
	variableCost(vecVariables->size()),
	loops(loops),
	tabuList(vecVariables->size()),
	bestSolution(vecVariables->size()),
	objOriginalNull(false)
    { 
      domain->restart( vecVariables );
    }

    //! Solver's main function, to solve the given CSP/COP.
    /*!
     * \param timeout The satisfaction run timeout in milliseconds
     * \return The satisfaction or optimization cost of the best
     * solution, respectively is the Solver object has been
     * instanciate with a null Objective (pure satisfaction run) or an
     * non-null Objective (optimization run).
     */
    double solve( double sat_timeout, double opt_timeout = 0 )
    {
      sat_timeout *= 1000; // timeouts in microseconds
      if( opt_timeout == 0 )
	opt_timeout = sat_timeout * 10;
      else
	opt_timeout *= 1000;

      int tabu_length = vecVariables->size() - 1;

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
      
      int sizeDomain = domain->getSize();
      vector< vector< double > >	vecConstraintsCosts( vecConstraints.size() );
      vector< double >			vecGlobalCosts( sizeDomain );
      vector< vector< double > >	vecVarSimCosts( sizeDomain );

      if( objective == nullptr && !objOriginalNull )
      {
	objective = make_shared< NullObjective<TypeVariable, TypeDomain> >();
	objOriginalNull = true;
      }
      
      objective->initHelper( sizeDomain );

      bestCost = numeric_limits<int>::max();
      double beforePostProc = bestCost;
      double bestGlobalCost = numeric_limits<int>::max();
      double globalCost;
      double currentCost;
      double bestEstimatedCost;
      int    bestValue = 0;

      vector<int> worstVariables;
      double worstVariableCost;
      int worstVariableId;

      TypeVariable *oldVariable;
      vector<int> possibleValues;
      vector<double> varSimCost( vecVariables->size() );
      vector<double> bestSimCost( vecVariables->size() );

      int tour = 0;
      int iterations = 0;

      do // optimization loop
      {
	startTour = chrono::high_resolution_clock::now();
	++tour;
	globalCost = numeric_limits<int>::max();
	bestEstimatedCost = numeric_limits<int>::max();
	std::fill( varSimCost.begin(), varSimCost.end(), 0. );
	std::fill( bestSimCost.begin(), bestSimCost.end(), 0. );
	std::fill( vecConstraintsCosts.begin(), vecConstraintsCosts.end(), vector<double>( vecVariables->size(), 0. ) );
	std::fill( vecVarSimCosts.begin(), vecVarSimCosts.end(), vector<double>( vecVariables->size(), 0. ) );
	std::fill( variableCost.begin(), variableCost.end(), 0. );
	std::fill( tabuList.begin(), tabuList.end(), 0 );

	do // solving loop 
	{
	  ++iterations;
	  
	  if( globalCost == numeric_limits<int>::max() )
	  {
	    domain->restart( vecVariables );
	    currentCost = 0.;

	    for( const auto &c : vecConstraints )
	      currentCost += c->cost( variableCost );

	    if( currentCost < globalCost )
	      globalCost = currentCost;
	    else
	    {
	      domain->restart( vecVariables );
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
	  oldVariable = &vecVariables->at( worstVariableId );
      
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

	  elapsedTimeTour = chrono::high_resolution_clock::now() - startTour;
	  elapsedTime = chrono::high_resolution_clock::now() - start;
	} while( globalCost != 0. && elapsedTimeTour.count() < sat_timeout && elapsedTime.count() < opt_timeout );

	// remove useless buildings
	if( globalCost == 0 )
	  objective->postprocessSatisfaction( vecVariables, domain, bestCost, bestSolution, sat_timeout );

	elapsedTime = chrono::high_resolution_clock::now() - start;
      }
      while( ( ( !objOriginalNull || loops == 0 )  && ( elapsedTime.count() < opt_timeout ) )
	     || ( objOriginalNull && elapsedTime.count() < sat_timeout * loops ) );

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
	   << "Number of tours: " << tour << endl
	   << "Number of iterations: " << iterations << endl;

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
    
  private:
    //! Solver's function to make a local move, ie, to assign a given
    //! value to a given variable
    inline void move( TypeVariable *building, int newValue )
    {
      domain->clear( *building );
      building->setValue( newValue );
      domain->add( *building );
    }

    //! Solver's function to make a permutation move, ie, to assign a given
    //! variable to a new position
    inline void permut( TypeVariable *building, int newValue )
    {
      int backup = building->getValue();

      if( backup == newValue )
	return;
      
      if( backup > newValue )
      {
	for( int i = backup ; i > newValue ; --i )
	{
	  std::swap( vecVariables->at(i-1), vecVariables->at(i) );
	  vecVariables->at(i).shiftValue();
	}

	vecVariables->at(newValue).setValue( newValue );	
      }
      else
      {
	for( int i = backup ; i < newValue ; ++i )
	{
	  std::swap( vecVariables->at(i), vecVariables->at(i+1) );
	  vecVariables->at(i).unshiftValue();
	}

	vecVariables->at(newValue).setValue( newValue );	
      }
    }

    vector< TypeVariable >				*vecVariables;	//!< A pointer to the vector of variable objects of the CSP/COP.
    TypeDomain						*domain;	//!< A pointer to the domain object of the CSP/COP.
    vector< shared_ptr<TypeConstraint> >		vecConstraints; //!< The vector of (shared pointers of) constraints of the CSP/COP.
    shared_ptr< Objective<TypeVariable, TypeDomain> >	objective;	//!< The shared pointer of the objective function.

    vector<double>				variableCost;		//!< The vector of projected costs on each variable.
    int						loops;			//!< The number of times we reiterate the satisfaction loop inside Solver::solve 
    vector<int>					tabuList;		//!< The tabu list, frozing each used variable for tabu_length iterations 
    Random					randomVar;		//!< The random generator used by the solver.
    double					bestCost;		//!< The (satisfaction or optimization) cost of the best solution.
    vector< TypeVariable >			bestSolution;		//!< The best solution found by the solver.
    bool					objOriginalNull;	//!< A boolean to know if it is a satisfaction or optimization run.
  };
}
