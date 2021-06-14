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
 * Copyright (C) 2014-2021 Florian Richoux
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
#include <map>
#include <cmath> // for isnan
#include <exception>

#include "variable.hpp"
#include "thirdparty/randutils.hpp"

namespace ghost
{
	//! This class encodes objective function of your model, as well as the special class NullObjective for CSP.
	/*! 
	 * In GHOST, many different objective objects can be instanciate.
	 *
	 * You cannot directly use this class Objective to encode your objective functions since this is an abstract class. 
	 * Thus, you must write your own objective class inheriting from ghost::Objective. You can write different objective 
	 * classes to model your problem, and switch from an objective to another between two Solver::solve calls.
	 *
	 * In this class, each virtual method follows the Non-Virtual Interface Idiom (see http://www.gotw.ca/publications/mill18.htm).
	 * The only pure virtual method is required_cost. All other virtual methods have a default behavior implemented and are 
	 * prefixed by 'expert_'. It is highly recommended to override these methods only if you know what you are doing.
	 *
	 * \sa Variable
	 */
	class Objective
	{
		template<typename FactoryModelType> friend class Solver;
		friend class SearchUnit;
		friend class FactoryModel;

		std::string _name; //!< Name of the objective object.
		std::vector<Variable*> _variables; //!<Vector of variables of the model.
		std::vector<int> _variables_index; // to know where are the constraint's variables in the global variable vector
		std::map<int,int> _variables_position; // to know where are global variables in the constraint's variables vector 
		bool _is_optimization;
		
		// std::map<int,int> _id_mapping; // Mapping between the variable's id in the solver (new_id) and its position in the vector of variables within the objective function.

		struct nanException : std::exception
		{
			std::vector<Variable*> variables;
			std::string message;

			nanException( const std::vector<Variable*>& variables ) : variables(variables)
			{
				message = "Objective required_cost returned a NaN value on variables (";
				for( int i = 0; i < static_cast<int>( variables.size() ) - 1; ++i )
					message += std::to_string( variables[i]->get_value() ) + ", ";
				message += std::to_string( variables[ static_cast<int>( variables.size() ) - 1 ]->get_value() ) + ")\n";
			}
			const char* what() const noexcept { return message.c_str(); }
		};

		struct variableOutOfTheScope : std::exception
		{
			std::string message;

			variableOutOfTheScope( int var_id, std::string name )
			{
				message = "Variable ID " + std::to_string( var_id ) + " is not in the scope of the Objective function " + name + ".\n";
			}
			const char* what() const noexcept { return message.c_str(); }
		};

		// Update a variable assignment.
		// void update_variable( int variable_id, int new_value );

		// Making the mapping between the variable's id in the solver (new_id) and its position in the vector of variables within the objective function. 
		// void make_variable_id_mapping( int new_id, int original_id );

		// Call required_cost() on Objective::_ptr_variables after getting sure the cost does give a nan, rise an exception otherwise.
		double cost() const;

		// // To simulate the cost between the current configuration and the candidate configuration.
		// // Call cost().
		// double simulate_cost( const std::vector<int>& variable_ids, const std::vector<int>& new_values );

		// Call expert_heuristic_value on Objective::_ptr_variables.
		inline int heuristic_value( int variable_index, const std::vector<int>& possible_values ) const
		{ return expert_heuristic_value( _variables, _variables_position.at( variable_index ), possible_values ); }

		// Call expert_heuristic_value_permutation on Objective::_ptr_variables.
		inline int heuristic_value_permutation( int variable_index, const std::vector<int>& bad_ptr_variables ) const
		{ return expert_heuristic_value_permutation( _variables, _variables_position.at( variable_index ), bad_ptr_variables ); }

		// Call expert_postprocess_satisfaction on Objective::_ptr_variables.
		inline void postprocess_satisfaction( double& best_error, std::vector<int>& solution ) const
		{ expert_postprocess_satisfaction( _variables, best_error, solution ); }
			
		// Call expert_postprocess_optimization on Objective::_ptr_variables.
		inline void postprocess_optimization( double& best_cost, std::vector<int>& solution ) const
		{ expert_postprocess_optimization( _variables, best_cost, solution ); }
			
	protected:
		mutable randutils::mt19937_rng rng; //!< A neat random generator implemented in thirdparty/randutils.hpp, see https://www.pcg-random.org/posts/ease-of-use-without-loss-of-power.html

		//! Pure virtual method to compute the value of the objective function on the current assignment in Objective::_ptr_variables .
		/*! 
		 * Like Constraint::required_error, this method is fundamental: it evalutes the performance of the current values of the variables.
		 * GHOST will search for variable values that will minimize the output of this method. If you are modeling a maximization problem, ie, 
		 * a problem where its natural objective function f(x) = z is to try to find the highest possible z, you can simplify write this method 
		 * such that it outputs -z. Values of variables minimizing -z will also maximize z.
		 *
		 * \param variables A const reference of the vector of variables in the scope of the objective function. The solver is calling this method with
		 * Objective::_ptr_variables as input. Giving a vector of variables as input here allow us to make cost simulations without modifying
		 * the value of Objective::_ptr_variables.
		 * \return A double corresponding to the value of the objective function on the current configuration. 
		 * Unlike Constraint::required_error, this output may be negative.
		 */
		virtual double required_cost( const std::vector<Variable*>& variables ) const = 0;

		//! Virtual method to apply the value heuristic used by the solver for non permutation problems.
		/*! 
		 * While dealing with non permutation problems, the solver calls this method to apply an eventual
		 * user-defined heuristic to choose a new domain value for a variable selected by the solver. 
		 *
		 * The default implementation outputs the value leading to the
		 * lowest objective cost. If two or more values lead to configurations 
		 * with the same lowest cost, one of them is randomly returned.
		 *
		 * Like all methods prefixed by 'expert_', you should override this method only if you 
		 * know what you are doing.
		 *
		 * \param variables A const reference of the vector of variables in the scope of the objective function. The solver is calling this method with
		 * Objective::_ptr_variables as input.
		 * \param variable_index The index of the variable to change in the vector Objective::_ptr_variables.
		 * \param possible_values A const reference to the vector of possible values of the variable to change. 
		 * \return The selected value according to the heuristic.
		 */
		virtual int expert_heuristic_value( const std::vector<Variable*>& variables,
		                                    int variable_index,
		                                    const std::vector<int>& possible_values ) const;

		//! Virtual method to apply the value heuristic used by the solver for permutation problems.
		/*! 
		 * While dealing with permutation problems, the solver calls this method to apply an eventual
		 * user-defined heuristic to choose a variable to swap the value with.
		 *
		 * By default, it returns a random variable from the bad_ptr_variables vector in input.
		 *
		 * Like all methods prefixed by 'expert_', you should override this method only if you 
		 * know what you are doing.
		 *
		 * \param variables A const reference of the vector of variables in the scope of the objective function. The solver is calling this method with
		 * Objective::_ptr_variables as input.
		 * \param variable_index The index of the variable to change in the vector Objective::_ptr_variables.
		 * \param bad_ptr_variables A const reference to the vector of candidate variables the solver may swap the value with.
		 * \return The index of the selected variable to swap with, according to the heuristic.
		 */
		virtual int expert_heuristic_value_permutation( const std::vector<Variable*>& variables,
		                                                int variable_index,
		                                                const std::vector<int>& bad_ptr_variables ) const;

		//! Virtual method to perform satisfaction post-processing.
		/*! 
		 * This method is called by the solver after a satisfaction run, if the solver was able to find a solution, to apply
		 * human-knowledge optimization in order to "clean-up" the proposed solution.
		 *
		 * It does nothing by default. You need to override it to have a satisfaction postprocess.
		 *
		 * Like all methods prefixed by 'expert_', you should override this method only if you 
		 * know what you are doing.
		 * 
		 * \param variables A const reference of the vector of variables in the scope of the objective function. The solver is calling this method with
		 * Objective::_ptr_variables as input.
		 * \param best_error A reference to the double representing the best satisfaction error found by the solver so far. Its value may be updated, justifying a non const reference.
		 * \param solution A reference to the vector of variables of the solution found by the solver. This vector may be updated, justifying a non const reference
		 * \sa postprocess_satisfaction
		 */
		virtual void expert_postprocess_satisfaction( const std::vector<Variable*>& variables,
		                                              double&	best_error,
		                                              std::vector<int>& solution ) const;

		//! Virtual method to perform optimization post-processing.
		/*! 
		 * This method is called by the solver after all optimization runs to apply human-knowledge optimization, allowing to improve
		 * the optimization cost.
		 *
		 * It does nothing by default. You need to override it to have an optimization postprocess.
		 *
		 * Like all methods prefixed by 'expert_', you should override this method only if you 
		 * know what you are doing.
		 *
		 * \warning The computation spantime of this method is not taken into account by timeouts given to the solver. 
		 * If you override this method, be sure its computation time is neglictable compare to the optimization timeout
		 * you give to Solver::solve.  
		 * 
		 * \param variables A const reference of the vector of variables in the scope of the objective function. The solver is calling this method with
		 * Objective::_ptr_variables as input.
		 * \param best_cost A reference to the double representing the best optimization cost found by the solver so far. Its value may be updated, justifying a non const reference.
		 * \param solution A reference to the vector of variables of the solution found by the solver. This vector may be updated, justifying a non const reference
		 * \sa postprocess_optimization
		 */
		virtual void expert_postprocess_optimization( const std::vector<Variable*>& variables,
		                                              double&	best_cost,
		                                              std::vector<int>&	solution ) const;

		inline void is_not_optimization() { _is_optimization = false; }

	public:
		//! Constructor taking variable indexes
		/*!
		 * \param name A const reference to a string to give the Objective object a specific name.
		 */
		Objective( std::string name, const std::vector<int>& variables_index );

		//! Constructor taking variables
		/*!
		 * \param name A const reference to a string to give the Objective object a specific name.
		 */
		Objective( std::string name, const std::vector<Variable>& variables );

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
		
		//! Inline accessor to get the name of the objective object.
		inline std::string get_name() const { return _name; }

		inline bool is_optimization() const { return _is_optimization; }
		
		// To have a nicer stream of Objective.
		friend std::ostream& operator<<( std::ostream& os, const Objective& o )
		{
			return os << "Objective name: " <<  o._name
			          << "\n********";
		}
	};

	/*******************/
	/** NullObjective **/
	/*******************/
	//! NullObjective is used when no objective functions have been given to the solver (ie, for pure satisfaction runs). 
	class NullObjective : public Objective
	{
	public:
		NullObjective() : Objective( "nullObjective", std::vector<int>{0} )
		{
			this->is_not_optimization();
		}
		
	private:
		double required_cost( const std::vector<Variable*>& variables ) const override { return 0.0; }
	};
}
