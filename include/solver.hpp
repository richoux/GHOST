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
 * Copyright (C) 2014-2020 Florian Richoux
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

#include <cassert>
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
#include "misc/randutils.hpp"
#include "objective.hpp"

using namespace std;

namespace ghost
{
	//! Solver is the class coding the solver itself.
	/*! 
	 * To solve a problem instance, you must instanciate a Solver object, then run Solver::solve.
	 *
	 * Solver constructors need a vector of Variable, a vector of shared pointers on Constraint objects, an optional 
	 * shared pointer on an Objective object (the solver will create a special empty Objective object is none is given), 
	 * and finally an optionnal boolean to indicate if the problem has been modeled as a permutation problem (false by default).
	 *
	 * A permutation problem is a problem where all variables start with different values, and only swapping values is allowed.
	 * This is typically the case for scheduling problems, for instance: you want to do A first, then B second, C third, and so on. 
	 * The solution of the problem must assign a unique value for each variable. Try as much as possible to model your problems as 
	 * permutation problems, since it should greatly speed-up the search of solutions.
	 *
	 * \sa Variable, Constraint, Objective
	 */  
	class Solver final
	{
		vector<Variable>&			_vecVariables;		//!< Reference to the vector of variables.
		vector<shared_ptr<Constraint>>&	_vecConstraints;	//!< Reference to the vector of shared pointer constraints.
		shared_ptr<Objective>		_objective;		//!< Shared pointer of the objective function.

		vector<int>	_weakTabuList;		//!< The weak tabu list, frozing used variables for tabuTime iterations. 
		//Random	_random;		//!< The random generator used by the solver.
		mutable randutils::mt19937_rng _rng; 	//!< A neat random generator from randutils.hpp.
		double	_bestSatCost;		//!< The satisfaction cost of the best solution.
		double	_bestSatCostOptLoop;	//!< The satisfaction cost of the best solution in the current optimization loop.
		double	_bestOptCost;		//!< The optimization cost of the best solution.
		bool	_isOptimization;	//!< A boolean to know if it is a satisfaction or optimization run.
		bool	_permutationProblem;	//!< A boolean to know if it is a permutation problem or not.
		
		int		_varOffset;		//!< Offset to shift variables id, such that the first would be shifted to 0.
		int		_ctrOffset;		//!< Same for constraints.
		int		_number_variables;	//!< Size of the vector of variables.
    
		//! NullObjective is used when no objective functions have been given to the solver (ie, for pure satisfaction runs). 
		class NullObjective : public Objective
		{
			using Objective::rng;
      
		public:
			NullObjective() : Objective("nullObjective") { }
      
		private:
			double required_cost( const vector< Variable >& variables ) const override { return 0.; }
      
			int expert_heuristic_value( const vector< Variable >&	variables,
			                            Variable&			var,
			                            const vector< int >&		valuesList ) const override
			{
				return rng.pick( valuesList );
			}
		};
    
		struct VarComp
		{
			bool operator()( const Variable& lhs, const Variable& rhs ) const
			{
				return lhs.get_id() < rhs.get_id();
			}
		};
    
		mutable map< Variable, vector< shared_ptr<Constraint> >, VarComp > _mapVarCtr; //!< Map to know in which constraints are each variable.

		//! Set the initial configuration by calling monte_carlo_sampling() 'samplings' times.
		/*!
		 * After calling calling monte_carlo_sampling() 'samplings' times, the function keeps 
		 * the configuration wth the lowest satisfaction cost. If some of them reach 0, it keeps 
		 * the configuration with the best optimization cost. 
		 */
		void set_initial_configuration( int samplings = 1 );

		//! Sample an configuration
		void monte_carlo_sampling();

		//! Sample an configuration for permutation problems
		void random_permutations();

		//! Decreasing values in tabuList
		/*!
		 * \param freeVariables is set to true if there is at least one free variable, ie, untabu. 
		 */
		void decay_weak_tabu_list( bool& freeVariables );

		//! To factorize code like if (best > current) then best=current and update configuration
		void update_better_configuration( double& best, const double current, vector<int>& configuration );

#if defined(ADAPTIVE_SEARCH)
		//! To compute the vector of variables which are principal culprits for not satisfying the problem
		vector< Variable* > compute_worst_variables( bool freeVariables, const vector<double>& costVariables );
#endif
    
		//! Compute the cost of each constraints and fill up the vector costConstraints
		double compute_constraints_costs( vector<double>& costConstraints ) const;

		//! Compute the variable cost of each variables and fill up vectors costVariables and costNonTabuVariables 
		void compute_variables_costs( const vector<double>& costConstraints,
		                              vector<double>&	costVariables,
		                              vector<double>&	costNonTabuVariables,
		                              const double		currentSatCost ) const;

		//! Compute incrementally the now satisfaction cost IF we change the value of 'variable' by 'value' with a local move.
		double simulate_local_move_cost( Variable			*variable,
		                                 double			value,
		                                 const vector<double>&	costConstraints,
		                                 double			currentSatCost ) const;

		//! Compute incrementally the now satisfaction cost IF we swap values of 'variable' with another variable.
		double simulate_permutation_cost( Variable			*worstVariable,
		                                  Variable&			otherVariable,
		                                  const vector<double>&	costConstraints,
		                                  double			currentSatCost ) const;

		//! Function to make a local move, ie, to assign a given
		void local_move( Variable		*variable,
		                 vector<double>&	costConstraints,
		                 vector<double>&	costVariables,
		                 vector<double>&	costNonTabuVariables,
		                 double&		currentSatCost );

		//! Function to make a permutation move, ie, to assign a given
		void permutation_move( Variable		*variable,
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
		Solver( vector<Variable>&			vecVariables, 
		        vector<shared_ptr<Constraint>>&	vecConstraints,
		        shared_ptr<Objective>		objective,
		        bool				permutationProblem = false );

		//! Second Solver's constructor, without Objective
		/*!
		 * \param vecVariables A reference to the vector of Variables.
		 * \param vecConstraints A reference to the vector of Constraints.
		 * \param permutationProblem A boolean indicating if we work on a permutation problem. False by default.
		 */
		Solver( vector<Variable>&			vecVariables, 
		        vector<shared_ptr<Constraint>>&	vecConstraints,
		        bool				permutationProblem = false );
    
		//! Solver's main function, to solve the given CSP/COP.
		/*!
		 * This function is the heart of GHOST's solver: it will try to find a solution within a limit time. If it finds wuch a solution, 
		 * the function outputs the value true.\n
		 * Here how it works: if at least one solution is found, at the end of the computation, it will write in the two first
		 * parameters finalCost and finalSolution the cost of the best solution found and the value of each variable.\n
		 * For a satisfaction problem (without any objective functions), the cost of a solution is the sum of the cost of each
		 * problem constraint (computated by Constraint::required_cost). For an optimization problem, the cost is the value outputed
		 * by Objective::required_cost.\n
		 * For both, the lower value the better: A satisfaction cost of 0 means we have a solution to a satisfaction problem (ie, 
		 * all constraints are satisfied). An optimization cost should be as low as possible: GHOST is handling minimization problems 
		 * only. If you have a maximization problem (you are looking to the highest possible value of your objective function), look 
		 * at the Objective documentation to see how to easily convert your problem into a minimization problem.
		 *
		 * The two last parameters sat_timeout and opt_timeout are fundamental: sat_timeout is mandatory, opt_timeout is optional: 
		 * if not given, its value will be fixed to sat_timeout * 10.\n
		 * sat_timeout is the timeout in milliseconds you give to GHOST to find a solution to the problem, ie, finding a value for 
		 * each variable such that each constraint of the problem is satisfied. For a satisfaction problem, this is the timeout within
		 * GHOST must output a solution.\n
		 * opt_timeout is only useful for optimization problems. Once GHOST finds a solution within sat_timeout, it saves it and try to find 
		 * other solutions leading to better (ie, smaller) values of the objective function. Then it restart a fresh satisfaction search, 
		 * with again sat_timeout as a timeout to find a solution. It will repeat this operation until opt_timeout is reached.
		 *
		 * Thus for instance, if you set sat_timeout to 20ms and opt_timeout to 60ms (or bit more like 61 or 62ms, see why below), you let GHOST 
		 * the time to run 3 satisfaction runs within a global runtime of 60ms (or 61, 62ms), like illustrated below.
		 *
		 * \image html architecture.png "x and y milliseconds correspond respectively to sat_timeout and opt_timeout"
		 * \image latex architecture.png "x and y milliseconds correspond respectively to sat_timeout and opt_timeout"
		 *
		 * It is possible it returns no solutions after timeout; in that case Solver::solve returns false. If it is often the case, this is a 
		 * strong evidence the satisfaction timeout is too low, and the solver does not have time to find at least one solution. Thus, this is 
		 * the only parameter you may have to tweak in GHOST.
		 *
		 * The illustration above shows satisfaction and optimization post-processes. The first one is triggered each time the solver found a solution. 
		 * If the user overloads Objective::expert_postprocess_satisfaction, he or she must be sure that his or her function runs very quickly, otherwise
		 * it may slow down the whole optimization process and may limit the number of solutions found by the solver. Optimization post-process runtime 
		 * is not taken into account within opt_timeout, so the real GHOST runtime for optimization problems will be roughly equals to opt_timeout + 
		 * optimization post-process runtime.
		 *
		 * \param finalCost A reference to the double of the sum of constraints cost for satisfaction problems, 
		 * or the value of the objective function for optimization problems. For satisfaction problems, a cost of zero means a solution has been found.
		 * \param finalSolution The configuration of the best solution found, ie, a reference to the vector of assignements of each variable.
		 * \param sat_timeout The satisfaction timeout in milliseconds.
		 * \param opt_timeout The optimization timeout in milliseconds (optionnal, equals to 10 times sat_timeout is not set).
		 * \param no_random_starting_point A Boolean to indicate if the solver should not start from a random starting point. This is necessary in particular to use the resume feature.
		 * \return True iff a solution has been found.
		 */
		bool solve( double& finalCost,
		            vector<int>& finalSolution,
		            double sat_timeout,
		            double opt_timeout = 0.,
		            bool no_random_starting_point = false );
	};
}
