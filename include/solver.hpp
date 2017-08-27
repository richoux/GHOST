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

#include <vector>
#include <map>

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
  template <typename TypeVariable>
  class Solver
  {
    vector< TypeVariable >		_vecVariables;	//!< Vector of (shared pointers of) variable of the CSP/COP.
    vector< Constraint<TypeVariable> >	_vecConstraints; //!< The vector of (shared pointers of) constraints of the CSP/COP.
    Objective<TypeVariable>		_objective;	//!< The shared pointer of the objective function.

    vector<int>	_weakTabuList;		//!< The weak tabu list, frozing used variables for tabuTime iterations. 
    Random	_randomVar;		//!< The random generator used by the solver.
    double	_bestSatCost;		//!< The satisfaction cost of the best solution.
    double	_bestSatCostTour;	//!< The satisfaction cost of the best solution in the current optimization loop.
    double	_bestOptCost;		//!< The optimization cost of the best solution.
    bool	_isOptimization;	//!< A boolean to know if it is a satisfaction or optimization run.
    bool	_permutationProblem;	//!< A boolean to know if it is a permutation problem or not.

    /////////////////////////
    // Bonne idée de mettre mutable ici ? On peut s'en débarasser ?
    mutable map< TypeVariable, vector< Constraint<TypeVariable> > > _mapVarCtr;	//!< Map to know in which constraints are each variable.
    // map<Variable, vector< pair< shared_ptr< Constraint >, vector< Variable* >::iterator> >
    // _mapVarCtr;	//!< Map to know in which constraints are each variable.

    //! Set the initial configuration by calling monte_carlo_sampling() 'samplings' times.
    /*!
     * After calling calling monte_carlo_sampling() 'samplings' times, the function keeps 
     * the configuration wth the lowest global cost. If some of them reach 0, it keeps 
     * the configuration with the best objective cost. 
     * 
     * \param samplings The number of Monte Carlo samplings. Equals to 1 by default.
     */
    void set_initial_configuration( int samplings = 1 );

    //! Sample an configuration
    void monte_carlo_sampling();
    
    //! Decreasing values in tabuList
    //! \param freeVariables A boolean set to true if there is at least one free variable, ie, untabu.
    //! \sa _weakTabuList
    void decay_weak_tabu_list( bool& freeVariables );

    //! Compute and return the vector containing worst variables,
    //! ie, variables with the highest variable cost.
    //! \return A vector of worst variables
    vector< TypeVariable > compute_worst_variables( bool freeVariables, const vector<double>& costVariables ) const;

    //! Compute the cost of each constraints
    //! \param costConstraints The vector to be filled by this function.
    //! \return The sum of constraints costs, ie, the global cost of the current configuration.
    double compute_constraints_costs( vector<double>& costConstraints ) const;

    //! Compute the variable cost of each variables
    //! \param costConstraints The vector containing the cost of each constraint.
    //! \param costVariables The vector of the cost of each variable, filled by this function.
    //! \param costNonTabuVariables The vector of the cost of non-tabu variables only, filled by this function.
    void compute_variables_costs( const vector<double>& costConstraints,
				  vector<double>& costVariables,
				  vector<double>& costNonTabuVariables ) const;

    // Compute incrementally the now global cost IF we change the value of 'variable' by 'value' with a local move.
    double simulate_local_move_cost( TypeVariable& variable,
				     double value,
				     vector<double>& costConstraints,
				     double currentSatCost ) const;

    // Compute incrementally the now global cost IF we swap values of 'variable' with another variable.
    double simulate_permutation_cost( TypeVariable& worstVariable,
				      TypeVariable& otherVariable,
				      vector<double>& costConstraints,
				      double currentSatCost ) const;

    //! Function to make a local move, ie, to assign a given
    //! value to a given variable
    void local_move( TypeVariable& variable,
		     vector<double>& costConstraints,
		     vector<double>& costVariables,
		     vector<double>& costNonTabuVariables,
		     double& currentSatCost );

    //! Function to make a permutation move, ie, to assign a given
    //! variable to a new position
    void permutation_move( TypeVariable& variable,
			   vector<double>& costConstraints,
			   vector<double>& costVariables,
			   vector<double>& costNonTabuVariables,
			   double& currentSatCost );


  public:
    //! Solver's regular constructor
    /*!
     * \param vecVariables A pointer to the vector of variable objects of the CSP/COP.
     * \param vecConstraints A constant reference to the vector of shared pointers of Constraint
     * \param obj A reference to the shared pointer of an Objective object. Default value is nullptr.
     * \param permutationProblem A boolean indicating if we work on a permutation problem. False by default.
     */
    Solver( const vector< TypeVariable >& vecVariables, 
	    const vector< Constraint<TypeVariable> >& vecConstraints,
	    Objective<TypeVariable> obj,
	    bool permutationProblem = false );

    //! Second Solver's constructor
    /*!
     * The solver is calling Solver(vecVariables, vecConstraints, nullptr, permutationProblem)
     *
     * \param vecVariables A pointer to the vector of variable objects of the CSP/COP.
     * \param vecConstraints A constant reference to the vector of shared pointers of Constraint
     * \param permutationProblem A boolean indicating if we work on a permutation problem. False by default.
     */
    Solver( const vector< TypeVariable >& vecVariables, 
	    const vector< Constraint<TypeVariable> >& vecConstraints,
	    bool permutationProblem = false );

    //! Solver's main function, to solve the given CSP/COP.
    /*!
     * \param finalCost The double of the sum of constraints cost for satisfaction problems, or the value of the objective function for optimization problems. For satisfaction problems, a cost of zero means a solution has been found.
     * \param finalSolution The configuration of the best solution found, ie, the vector of assignements of each variable.
     * \param sat_timeout The satisfaction timeout in milliseconds.
     * \param opt_timeout The optimization timeout in milliseconds (optionnal, equals to 10 times sat_timeout is not set).
     * \return True iff a solution has been found.
     */
    bool solve( double& finalCost, vector<int>& finalSolution, double sat_timeout, double opt_timeout = 0. );
  };
}
