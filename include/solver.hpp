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

#include "variables/variable.hpp"
#include "constraints/constraint.hpp"
#include "domains/domain.hpp"
#include "misc/random.hpp"
#include "misc/constants.hpp"
#include "objectives/objective.hpp"

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
	bestSolution(vecVariables->size())
    { 
      reset();
    }

    //! Solver's main function, to solve the given CSP/COP.
    /*!
     * \param timeout The satisfaction run timeout in milliseconds
     * \return The satisfaction or optimization cost of the best
     * solution, respectively is the Solver object has been
     * instanciate with a null Objective (pure satisfaction run) or an
     * non-null Objective (optimization run).
     */
    double solve( double timeout )
    {
      chrono::duration<double,milli> elapsedTime(0);
      chrono::duration<double,milli> elapsedTimeTour(0);
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

      // double timerPostProcessSat = 0;
      double timerPostProcessOpt = 0;
      
      int sizeDomain = domain->getSize();
      vector< vector< double > >	vecConstraintsCosts( vecConstraints.size() );
      vector< double >			vecGlobalCosts( sizeDomain );
      vector< vector< double > >	vecVarSimCosts( sizeDomain );

      bool objOriginalNull = false;
      if( objective == nullptr )
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
      int    bestPosition = 0;

      vector<int> worstVariables;
      double worstVariableCost;
      int worstVariableId;

      TypeVariable *oldVariable;
      vector<int> possiblePositions;
      vector<double> varSimCost( vecVariables->size() );
      vector<double> bestSimCost( vecVariables->size() );

      int tour = 0;
      int iterations = 0;

      do // optimization loop
      {
	startTour = chrono::system_clock::now();
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
	    currentCost = 0.;

	    for( auto &c : vecConstraints )
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
	  oldVariable = &vecVariables->at( worstVariableId );
      
	  // get possible positions for oldVariable.
	  possiblePositions = domain->possibleValues( *oldVariable );

	  // time simulateCost
	  startSimCost = chrono::system_clock::now();

	  // variable simulated costs
	  fill( bestSimCost.begin(), bestSimCost.end(), 0. );

#ifndef NDEBUG
	  soverlap = chrono::system_clock::now();
	  vecConstraintsCosts[0] = vecConstraints[0]->simulateCost( *oldVariable, possiblePositions, vecVarSimCosts );
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
	  for( int i = 0; i < vecConstraints.size(); ++i )
	    vecConstraintsCosts[i] = vecConstraints[i]->simulateCost( *oldVariable, possiblePositions, vecVarSimCosts );
#endif

	  fill( vecGlobalCosts.begin(), vecGlobalCosts.end(), 0. );

	  // sum all numbers in the vector vecConstraintsCosts[i] and put it into vecGlobalCosts[i] 
	  for( auto &v : vecConstraintsCosts )
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
	  objective->updateHelper( *oldVariable, possiblePositions, vecVariables, domain );
	  int b = objective->heuristicValue( vecGlobalCosts, bestEstimatedCost, bestPosition );
	  bestSimCost = vecVarSimCosts[ b ];

	  timeSimCost += chrono::system_clock::now() - startSimCost;

	  currentCost = bestEstimatedCost;

	  if( bestEstimatedCost < globalCost )
	  {
	    globalCost = bestEstimatedCost;

	    if( globalCost < bestGlobalCost )
	    {
	      bestGlobalCost = globalCost;
	      for( int i = 0; i < vecVariables->size(); ++i )
		bestSolution[i] = vecVariables->at(i).getValue();
	    }
	    
	    variableCost = bestSimCost;
	    move( oldVariable, bestPosition );
	  }
	  else // local minima
	    tabuList[ worstVariableId ] = TABU;

	  elapsedTimeTour = chrono::system_clock::now() - startTour;
	  elapsedTime = chrono::system_clock::now() - start;
	} while( globalCost != 0. && elapsedTimeTour.count() < timeout && elapsedTime.count() < OPT_TIME );

	// remove useless buildings
	if( globalCost == 0 )
	  objective->postprocessSatisfaction( vecVariables, domain, bestCost, bestSolution );

	reset();
	elapsedTime = chrono::system_clock::now() - start;
      }
      while( ( ( !objOriginalNull || loops == 0 )  && ( elapsedTime.count() < OPT_TIME ) )
	     || ( objOriginalNull && elapsedTime.count() < timeout * loops ) );

      for( auto &b : *vecVariables )
	domain->clear( b );

      for( int i = 0; i < vecVariables->size(); ++i )
	vecVariables->at(i).setValue( bestSolution[i] );
    
      for( auto &b : *vecVariables )
	domain->add( b );

      beforePostProc = bestCost;
      
      if( bestGlobalCost == 0 )
	timerPostProcessOpt = objective->postprocessOptimization( vecVariables, domain, bestCost );
    
      cout << "Domains:" << *domain << endl;

      if( objOriginalNull )
	cout << "SATISFACTION run: try to find a sound wall only!" << endl;
      else
	cout << "OPTIMIZATION run with objective " << objective->getName() << endl;
      
      cout << "Elapsed time: " << elapsedTime.count() << endl
	   << "Global cost: " << bestGlobalCost << endl
	   << "Number of tours: " << tour << endl
	   << "Number of iterations: " << iterations << endl;

      if( objOriginalNull )
      {
        if( bestGlobalCost == 0 )
        {
          shared_ptr<WallinObjective> bObj = make_shared<BuildingObj>();
          shared_ptr<WallinObjective> gObj = make_shared<GapObj>();
          shared_ptr<WallinObjective> tObj = make_shared<TechTreeObj>();
	  
          cout << "Opt Cost if the objective was building: " << bObj->cost( vecVariables, domain ) << endl
	       << "Opt Cost if the objective was gap: \t" << gObj->cost( vecVariables, domain ) << endl
	       << "Opt Cost if the objective was techtree: " << tObj->cost( vecVariables, domain ) << endl;
        }
      }
      else
      {
	cout << "Optimization cost: " << bestCost << endl
	     << "Opt Cost BEFORE post-processing: " << beforePostProc << endl;
      }
      
      if( timerPostProcessOpt != 0 )
	cout << "Post-processing time: " << timerPostProcessOpt << endl; 

#ifndef NDEBUG
      cout << endl << "Elapsed time to simulate cost: " << timeSimCost.count() << endl
	   << "Overlap: " << toverlap.count() << endl
	   << "Buildable: " << tbuildable.count() << endl
	   << "NoGaps: " << tnogaps.count() << endl
	   << "STT: " << tstt.count() << endl;

      // print cost for each constraint
      for( auto &c : vecConstraints )
      {
	fill( varSimCost.begin(), varSimCost.end(), 0. );
	cout << "Cost of " << typeid(*c).name() << ": " << c->cost( varSimCost ) << " [";

	for( auto &v : varSimCost )
	  cout << " " << v;

	cout << " ]" << endl;
      }      

      cout << endl << "Buildings:" << endl;
      for(const auto &v : *vecVariables)
	cout << v << endl;
      
      cout << endl;
#endif

      if( objOriginalNull )
	return bestGlobalCost;
      else
	return bestCost;
    }
    
  private:
    //! Solver's function to perform a reset, ie, to restart the
    //! search process from a fresh and randomly generated
    //! configuration.
    void reset()
    {
      for( auto &v : *vecVariables )
	domain->clear( v );
	
      for( auto &v : *vecVariables )
      {
	// 1 chance over 3 to be placed on the domain
	if( randomVar.getRandNum(3) == 0)
	{
	  v.setValue( domain->randomValue( v ) );
	  domain->add( v );
	}
	else
	  v.setValue( -1 );
      }
    }

    //! Solver's function to make a local move, ie, to assign a given
    //! value to a given variable
    inline void move( TypeVariable *building, int newPosition )
    {
      domain->clear( *building );
      building->setValue( newPosition );
      domain->add( *building );
    }

    vector< TypeVariable >				*vecVariables;	//!< A pointer to the vector of variable objects of the CSP/COP.
    TypeDomain						*domain;	//!< A pointer to the domain object of the CSP/COP.
    vector< shared_ptr<TypeConstraint> >		vecConstraints; //!< The vector of (shared pointers of) constraints of the CSP/COP.
    shared_ptr< Objective<TypeVariable, TypeDomain> >	objective;	//!< The shared pointer of the objective function.

    vector<double>				variableCost;		//!< The vector of projected costs on each varaible.
    int						loops;			//!< The number of times we reiterate the satisfaction loop inside Solver::solve 
    vector<int>					tabuList;		//!< The tabu list, frozing each used variable for TABU iterations 
    Random					randomVar;		//!< The random generator used by the solver.
    double					bestCost;		//!< The (satisfaction or optimization) cost of the best solution.
    vector<int>					bestSolution;		//!< The best solution found by the solver.
  };
}