/*
 * GHOST (General meta-Heuristic Optimization Solving Tool) is a C++ library 
 * designed to help developers to model and implement optimization problem 
 * solving. It contains a meta-heuristic solver aiming to solve any kind of 
 * combinatorial and optimization real-time problems represented by a CSP/COP/CFN. 
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

#include <algorithm>
#include <limits>
#include <vector>
#include <cmath> // for isnan
#include <exception>

#include "variable.hpp"
#include "misc/randutils.hpp"

namespace ghost
{
	//! This class encodes objective functions of your COP/CFN, as well as the special class NullObjective for CSP.
	/*! 
	 * In GHOST, many different objective objects can be instanciate.
	 *
	 * You cannot directly use this class Objective to encode your objective functions since this is an abstract class. 
	 * Thus, you must write your own objective class inheriting from ghost::Objective. You can write different objective 
	 * classes to model your problem, and switch from an objective to another between two Solver::solve calls.
	 *
	 * In this class, each virtual function follows the Non-Virtual Interface Idiom (see http://www.gotw.ca/publications/mill18.htm).
	 * The only pure virtual function is required_cost. All other virtual functions have a default behavior implemented and are 
	 * prefixed by 'expert_'. It is highly recommended to override these functions only if you know what you are doing.
	 *
	 * \sa Variable
	 */
	class Objective
	{
		struct nanException : std::exception
		{
			const std::vector< Variable >&	variables;
			std::string message;

			nanException( const std::vector< Variable >&	variables ) : variables(variables)
			{
				message = "Objective required_cost returned a NaN value on variables (";
				for( int i = 0; i < (int)variables.size() - 1; ++i )
					message += std::to_string(variables[i].get_value()) + ", ";
				message += std::to_string(variables[(int)variables.size() - 1].get_value()) + ")\n";
			}
			const char* what() const noexcept { return message.c_str(); }
		};
	  
	protected:
    
		//Random random; //!< Random generator used by the function heuristicValue.
		mutable randutils::mt19937_rng rng; //!< A neat random generator from randutils.hpp.
		std::string name; //!< String for the name of the objective object.

		//! Pure virtual function to compute the value of the objective function on the current configuration.
		/*! 
		 * Like Constraint::required_cost, this function is fundamental: it evalutes the performance of the current values of the variables.
		 * GHOST will search for variable values that will minimize the output of this function. If you are modeling a maximization problem, ie, 
		 * a problem where its natural objective function f(x) = z is to try to find the highest possible z, you can simplify write this function 
		 * such that it outputs -z. Values of variables minimizing -z will also maximize z.
		 *
		 * \param variables A const reference to the vector of variable of the CSP/COP/CFN.
		 * \return A double corresponding to the value of the objective function on the current configuration. 
		 * Unlike Constraint::required_cost, this output may be negative.
		 * \sa cost
		 */
		virtual double required_cost( const std::vector< Variable >& variables ) const = 0;

		//! Virtual function to apply the value heuristic used by the solver for non permutation problems.
		/*! 
		 * While dealing with non permutation problems, the solver calls this function to apply an eventual
		 * user-defined heuristic to choose a new domain value for a variable selected by the solver. 
		 *
		 * The default implementation outputs the value leading to the
		 * lowest objective cost. If two or more values lead to configurations 
		 * with the same lowest cost, one of them is randomly returned.
		 *
		 * Like all functions prefixed by 'expert_', you should override this function only if you 
		 * know what you are doing.
		 *
		 * \param variables A const reference to the vector containing all variables.
		 * \param var A reference to the variable to change.
		 * \param possible_values A const reference to the vector of possible values of var. 
		 * \return The selected value according to the heuristic.
		 * \sa heuristic_value
		 */
		virtual int	expert_heuristic_value( const std::vector< Variable >&	variables,
		                                    Variable& var,
		                                    const std::vector< int >& possible_values ) const;

		//! Virtual function to apply the value heuristic used by the solver for permutation problems.
		/*! 
		 * While dealing with permutation problems, the solver calls this function to apply an eventual
		 * user-defined heuristic to choose a variable to swap the value with.
		 *
		 * By default, it returns a random variable among the vector in input.
		 *
		 * Like all functions prefixed by 'expert_', you should override this function only if you 
		 * know what you are doing.
		 *
		 * \param bad_variables A const reference to the vector of pointers to candidate variables
		 * the solver may swap the value with another variable it had chosen.
		 * \return The address of the selected variable to swap with, according to the heuristic.
		 * \sa heuristic_value
		 */
		virtual Variable* expert_heuristic_value( const std::vector< Variable* >& bad_variables ) const;

		//! Virtual function to perform satisfaction post-processing.
		/*! 
		 * This function is called by the solver after a satisfaction run, if the solver was able to find a solution, to apply
		 * human-knowledge optimization in order to "clean-up" the proposed solution.
		 *
		 * It does nothing by default. You need to override it to have a satisfaction postprocess.
		 *
		 * Like all functions prefixed by 'expert_', you should override this function only if you 
		 * know what you are doing.
		 * 
		 * \param variables A reference to the vector of variables of the CSP/COP/CFN.
		 * \param bestCost A reference to the double representing the best satisfaction cost found by the solver so far. Its value may be updated, justifying a non const reference.
		 * \param solution A reference to the vector of variables of the solution found by the solver. This vector may be updated, justifying a non const reference
		 * \sa postprocess_satisfaction
		 */
		virtual void expert_postprocess_satisfaction( std::vector< Variable >&	variables,
		                                              double&	bestCost,
		                                              std::vector< int >& solution ) const;

		//! Virtual function to perform optimization post-processing.
		/*! 
		 * This function is called by the solver after all optimization runs to apply human-knowledge optimization, allowing to improve
		 * the optimization cost.
		 *
		 * It does nothing by default. You need to override it to have an optimization postprocess.
		 *
		 * Like all functions prefixed by 'expert_', you should override this function only if you 
		 * know what you are doing.
		 *
		 * \warning The computation spantime of this function is not taken into account by timeouts given to the solver. 
		 * If you override this function, be sure its computation time is neglictable compare to the optimization timeout
		 * you give to Solver::solve.  
		 * 
		 * \param variables A reference to the vector of variables of the CSP/COP/CFN.
		 * \param bestCost A reference to the double representing the best optimization cost found by the solver so far. Its value may be updated, justifying a non const reference.
		 * \param solution A reference to the vector of variables of the solution found by the solver. This vector may be updated, justifying a non const reference
		 * \sa postprocess_optimization
		 */
		virtual void expert_postprocess_optimization( std::vector< Variable >&	variables,
		                                              double&		bestCost,
		                                              std::vector< int >&	solution ) const;

	public:
		//! Unique constructor
		/*!
		 * \param name A const reference to a string to give the Objective object a specific name.
		 */
		Objective( const std::string& name );

		//! Default copy contructor.
		Objective( const Objective& other ) = default;
		//! Default move contructor.
		Objective( Objective&& other ) = default;

		//! Copy assignment operator disabled.
		Objective& operator=( const Objective& other ) = delete;
		//! Move assignment operator disabled.
		Objective& operator=( Objective&& other ) = delete;

		// Default virtual destructor.
		virtual ~Objective() = default;
        
		//! Inline function following the NVI idiom. Calling v_cost.
		/*! 
		 * @throw nanException
		 * \sa required_cost
		 */
		inline double cost( const std::vector< Variable >& variables ) const
		{
			double value = required_cost( variables );
			if( std::isnan( value ) )
				throw nanException( variables );
			return value;
		}
	  
		//! Inline function following the NVI idiom. Calling expert_heuristic_value.
		/*! 
		 * \sa expert_heuristic_value
		 */
		inline int heuristic_value( const std::vector< Variable >&	variables,
		                            Variable&	var,
		                            const std::vector< int >& possible_values ) const
		{ return expert_heuristic_value( variables, var, possible_values ); }

		//! Inline function following the NVI idiom. Calling expert_heuristic_value.
		/*! 
		 * \sa expert_heuristic_value
		 */
		inline Variable* heuristic_value( const std::vector< Variable* >& bad_variables ) const
		{ return expert_heuristic_value( bad_variables ); }

		//! Inline function following the NVI idiom. Calling expert_postprocess_satisfaction.
		/*! 
		 * \sa expert_postprocess_satisfaction
		 */
		inline void postprocess_satisfaction( std::vector< Variable >&	variables,
		                                      double&	bestCost,
		                                      std::vector< int >& solution ) const
		{ expert_postprocess_satisfaction( variables, bestCost, solution ); }

		//! Inline function following the NVI idiom. Calling expert_postprocess_optimization.
		/*! 
		 * \sa expert_postprocess_optimization
		 */
		inline void postprocess_optimization( std::vector< Variable >&	variables,
		                                      double&	bestCost,
		                                      std::vector< int >& solution ) const
		{ expert_postprocess_optimization( variables, bestCost, solution ); }

		//! Inline accessor to get the name of the objective object.
		inline std::string get_name() const { return name; }
	};
}
