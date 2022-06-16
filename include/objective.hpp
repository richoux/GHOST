/*
 * GHOST (General meta-Heuristic Optimization Solving Tool) is a C++ framework
 * designed to help developers to model and implement optimization problem
 * solving. It contains a meta-heuristic solver aiming to solve any kind of
 * combinatorial and optimization real-time problems represented by a CSP/COP/EFSP/EFOP. 
 *
 * First developped to solve game-related optimization problems, GHOST can be used for
 * any kind of applications where solving combinatorial and optimization problems. In
 * particular, it had been designed to be able to solve not-too-complex problem instances
 * within some milliseconds, making it very suitable for highly reactive or embedded systems.
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
	/*!
	 * This is the base class containing the logic of objective functions. However, users
	 * would not derive their own Objective class directly from ghost::Objective, but
	 * from one of the two derived class ghost::Minimize and ghost::Maximize.
	 *
	 * This class contains some methods prefixed by 'expert_'. It is highly recommended that
	 * users override such methods only if they know what they are doing.
	 *
	 * \sa Variable
	 */
	class Objective
	{
		template<typename ModelBuilderType> friend class Solver;
		friend class SearchUnit;
		friend class ModelBuilder;

		friend class NullObjective;
		friend class Minimize;
		friend class Maximize;

		friend class AdaptiveSearchValueHeuristic;
		friend class AntidoteSearchValueHeuristic;
		
		std::vector<Variable*> _variables; // Vector of raw pointers to variables needed to compute the objective function.
		std::vector<int> _variables_index; // To know where are the constraint's variables in the global variable vector.
		std::map<int,int> _variables_position; // To know where are global variables in the constraint's variables vector. 
		bool _is_optimization;
		bool _is_maximization;
		std::string _name; // Name of the objective object.

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

		Objective( const std::vector<int>& variables_index, bool is_maximization, const std::string& name );
		Objective( const std::vector<Variable>& variables, bool is_maximization, const std::string& name );

		inline void update( int index, int new_value ) { conditional_update_data_structures( _variables, _variables_position[ index ], new_value ); }

		// Call required_cost() on Objective::_variables after making sure the cost does not give a nan, rise an exception otherwise.
		double cost() const;

		// Call expert_heuristic_value on Objective::_variables.
		inline int heuristic_value( int variable_index, const std::vector<int>& possible_values ) const
		{ return expert_heuristic_value( _variables, _variables_position.at( variable_index ), possible_values ); }

		// Call expert_heuristic_value_permutation on Objective::_variables.
		inline int heuristic_value_permutation( int variable_index, const std::vector<int>& bad_variables ) const
		{ return expert_heuristic_value_permutation( _variables, _variables_position.at( variable_index ), bad_variables ); }

		// Call expert_postprocess on Objective::_variables.
		inline double postprocess( double best_cost ) const
		{ return expert_postprocess( _variables, best_cost ); }

	protected:
		mutable randutils::mt19937_rng rng; //!< A neat random generator implemented in thirdparty/randutils.hpp, see https://www.pcg-random.org/posts/ease-of-use-without-loss-of-power.html

		/*!
		 * Pure virtual method to compute the value of the objective function regarding the values of
		 * variables given as input.
		 *
		 * Like Constraint::required_error, this method is fundamental: it evalutes the performance
		 * of the current values of the variables. GHOST will search for variable values that will
		 * minimize or maximize the output of this method.
		 *
		 * Like any methods prefixed by 'required_', overriding this method is mandatory.
		 *
		 * \param variables a const reference of the vector of raw pointers to variables in the
		 * scope of the constraint. The solver is actually calling this method with the vector 
		 * of variables that has been given to the constructor.
		 * \return A double corresponding to the value of the objective function on the current
		 * configuration. Unlike Constraint::required_error, this output may be negative.
		 * \exception Throws an exception if the computed value is NaN.
		 */
		virtual double required_cost( const std::vector<Variable*>& variables ) const = 0;

		/*!
		 * Update user-defined data structures in the objective function.
		 *
		 * Like any methods prefixed by 'conditional_', this method must be overriden under
		 * some conditions: if some inner data structures are defined in derived objective classes
		 * and need to be updated while variable values change (i.e., when the solver asssign
		 * 'new_value' to variables[index]), this method must be implemented to define how data
		 * structures  must be updated.
		 *
		 * \param variables a const reference of the vector of raw pointers to variables of the 
		 * objective function.
		 * \param index an integer to get the variable 'variables[index]' that has been updated by
		 * the solver.
		 * \param new_value an integer to know what is the new value of 'variables[index]'.
		 */
		virtual void conditional_update_data_structures( const std::vector<Variable*>& variables, int index, int new_value );

		/*!
		 * Virtual method to apply the value heuristic used by the solver for non permutation
		 * problems.
		 *
		 * While dealing with non permutation problems, the solver calls this method to apply
		 * an eventual user-defined heuristic to choose a new domain value for a variable selected
		 * by the solver.
		 *
		 * The default implementation outputs the value leading to the lowest objective cost.
		 * If two or more values lead to configurations with the same lowest cost, one of them
		 * is randomly returned.
		 *
		 * Like any methods prefixed by 'expert_', users should override this method only if they
		 * know what they are doing.
		 *
		 * \param variables a const reference of the vector of raw pointers of variables in the scope
		 * of the objective function. The solver is calling this method with the vector of variables
		 * that has been given to the constructor.
		 * \param variable_index the index of the variable to change in the vector
		 * Objective::_variables.
		 * \param possible_values a const reference to the vector of possible values of the variable
		 * to change.
		 * \return The selected value according to the heuristic.
		 */
		virtual int expert_heuristic_value( const std::vector<Variable*>& variables,
		                                    int variable_index,
		                                    const std::vector<int>& possible_values ) const;

		/*!
		 * Virtual method to apply the value heuristic used by the solver for permutation problems.
		 *
		 * While dealing with permutation problems, the solver calls this method to apply an eventual
		 * user-defined heuristic to choose a variable to swap the value with.
		 *
		 * By default, it returns a random variable from the bad_variables vector given as input.
		 *
		 * Like any methods prefixed by 'expert_', users should override this method only if
		 * they know what they are doing.
		 *
		 * \param variables a const reference of the vector of raw pointers of variables in the
		 * scope of the objective function. The solver is calling this method with the vector 
		 * of variables that has been given to the constructor.
		 * \param variable_index the index of the variable to change in the vector Objective::_variables.
		 * \param bad_variables a const reference to the vector of candidate variables the solver
		 * may swap the value with.
		 * \return The index of the selected variable to swap with, according to the heuristic.
		 */
		virtual int expert_heuristic_value_permutation( const std::vector<Variable*>& variables,
		                                                int variable_index,
		                                                const std::vector<int>& bad_variables ) const;

		/*!
		 * Virtual method to perform post-processing optimization.
		 *
		 * This method is called by the solver once it has found a solution. Its purpose is to apply
		 * human-knowledge optimization.
		 *
		 * By default, it simply returns best_cost given as input, without modifying the variables.
		 * Users need to override it to have their own post-processing optimization.
		 *
		 * Like any methods prefixed by 'expert_', users should override this method only if
		 * they know what they are doing.
		 *
		 * \warning The computation spantime of this method is not taken into account by timeouts
		 * given to the solver. If users override this method, they must ensure its computation time
		 * is neglictable compare to the timeout giving as input to Solver::solve.
		 *
		 * \param variables a const reference of the vector of raw pointers of variables in the
		 * scope of the objective function. The solver is calling this method with the vector 
		 * of variables that has been given to the constructor. If users must change some variables 
		 * values, they must do it on variables from this vector, otherwise the modified solution
		 * won't be taken into account by the solver. 
		 * \param best_cost a double representing the best optimization cost found by the solver
		 * so far. This helps users be sure that their post-processing leads to actual improvements.
		 * \return The new error after post-processing.
		 */
		virtual double expert_postprocess( const std::vector<Variable*>& variables,
		                                   double best_cost ) const;


		// No documentation on purpose.
		inline void is_not_optimization() { _is_optimization = false; }

	public:
		//! Default copy contructor.
		Objective( const Objective& other ) = default;
		//! Default move contructor.
		Objective( Objective&& other ) = default;

		//! Copy assignment operator disabled.
		Objective& operator=( const Objective& other ) = delete;
		//! Move assignment operator disabled.
		Objective& operator=( Objective&& other ) = delete;

		//! Default virtual destructor.
		virtual ~Objective() = default;

		//! Inline accessor to get the name of the objective object.
		inline std::string get_name() const { return _name; }

		//! Inline method returning if a user-defined objective function has been declared.
		inline bool is_optimization() const { return _is_optimization; }

		/*!
		 * Inline method if the user-defined objective function has to be maximized (true)
		 * or minimize (false).
		 */
		inline bool is_maximization() const { return _is_maximization; }

		//! To have a nicer stream of Objective.
		friend std::ostream& operator<<( std::ostream& os, const Objective& o )
		{
			return os << "Objective name: " <<  o._name
			          << "\n********";
		}
	};

	/*******************/
	/** NullObjective **/
	/*******************/
	// NullObjective is used when no objective functions have been given to the solver (ie, for pure satisfaction runs). 
	class NullObjective : public Objective
	{
	public:
		NullObjective() : Objective( std::vector<int>{0}, false, "nullObjective" )
		{
			this->is_not_optimization();
		}

	private:
		double required_cost( const std::vector<Variable*>& variables ) const override { return 0.0; }
	};

	/**************/
	/** Minimize **/
	/**************/
	/*!
	 * This is the base class to define minimization objective functions.
	 *
	 * ghost::Minimize cannot be directly used to encode user-defined objective function, since
	 * this is an abstract class. To declare a problem with GHOST, users have to make their
	 * own derived objective class.
	 *
	 * \sa Objective
	 */
	class Minimize : public Objective
	{
	public:
		/*!
		 * Constructor with a vector of variable IDs. This vector is internally used by Objective
		 * to know what variables from the global variable vector it is handling. The name of 
		 * the objective function will be set to the string 'Minimize'.
		 * \param variables_index a const reference to a vector of IDs of variables composing the 
		 * objective function.
		 */
		Minimize( const std::vector<int>& variables_index )
			: Objective( variables_index, false, std::string( "Minimize" ) )
		{	}

		/*!
		 * Constructor building a vector of variable IDs by calling v->get_id() from all variables v.
		 * The name of the objective function will be set to the string 'Minimize'.
		 *
		 * \param variables a const reference to a vector of variable composing the objective function.
		 */
		Minimize( const std::vector<Variable>& variables )
			: Objective( variables, false, std::string( "Minimize" ) )
		{	}

		/*!
		 * Constructor with a vector of variable IDs. This vector is internally used by Objective
		 * to know what variables from the global variable vector it is handling.
		 * \param variables_index a const reference to a vector of IDs of variables composing the 
		 * objective function.
		 * \param name a const reference ot a string to give a name to the objective function.
		 */
		Minimize( const std::vector<int>& variables_index, const std::string& name )
			: Objective( variables_index, false, name )
		{	}

		/*!
		 * Constructor building a vector of variable IDs by calling v->get_id() from all variables v.
		 *
		 * \param variables a const reference to a vector of variable composing the objective function.
		 * \param name a const reference ot a string to give a name to the objective function.
		 */
		Minimize( const std::vector<Variable>& variables, const std::string& name )
			: Objective( variables, false, name )
		{	}

		/*!
		 * Constructor with a vector of variable IDs. This vector is internally used by Objective
		 * to know what variables from the global variable vector it is handling.
		 * \param variables_index a const reference to a vector of IDs of variables composing the 
		 * objective function.
		 * \param name a const char* to give a name to the objective function.
		 */
		Minimize( const std::vector<int>& variables_index, const char* name )
			: Objective( variables_index, false, std::string( name ) )
		{	}

		/*!
		 * Constructor building a vector of variable IDs by calling v->get_id() from all variables v.
		 *
		 * \param variables a const reference to a vector of variable composing the objective function.
		 * \param name a const char* to give a name to the objective function.
		 */
		Minimize( const std::vector<Variable>& variables, const char* name )
			: Objective( variables, false, std::string( name ) )
		{	}
	};

	/**************/
	/** Maximize **/
	/**************/
	/*!
	 * This is the base class to define maximization objective functions.
	 *
	 * ghost::Maximize cannot be directly used to encode user-defined objective function, since
	 * this is an abstract class. To declare a problem with GHOST, users have to make their
	 * own derived objective class.
	 *
	 * \sa Objective
	 */
	class Maximize : public Objective
	{
	public:
		/*!
		 * Constructor with a vector of variable IDs. This vector is internally used by Objective
		 * to know what variables from the global variable vector it is handling. The name of 
		 * the objective function will be set to the string 'Maximize'.
		 * \param variables_index a const reference to a vector of IDs of variables composing the 
		 * objective function.
		 */
		Maximize( const std::vector<int>& variables_index )
			: Objective( variables_index, true, std::string( "Maximize" ) )
		{	}

		/*!
		 * Constructor building a vector of variable IDs by calling v->get_id() from all variables v.
		 * The name of the objective function will be set to the string 'Maximize'.
		 *
		 * \param variables a const reference to a vector of variable composing the objective function.
		 */
		Maximize( const std::vector<Variable>& variables )
			: Objective( variables, true, std::string( "Maximize" ) )
		{	}

		/*!
		 * Constructor with a vector of variable IDs. This vector is internally used by Objective
		 * to know what variables from the global variable vector it is handling.
		 * \param variables_index a const reference to a vector of IDs of variables composing the 
		 * objective function.
		 * \param name a const reference ot a string to give a name to the objective function.
		 */
		Maximize( const std::vector<int>& variables_index, const std::string& name )
			: Objective( variables_index, true, name )
		{	}

		/*!
		 * Constructor building a vector of variable IDs by calling v->get_id() from all variables v.
		 *
		 * \param variables a const reference to a vector of variable composing the objective function.
		 * \param name a const reference ot a string to give a name to the objective function.
		 */
		Maximize( const std::vector<Variable>& variables, const std::string& name )
			: Objective( variables, true, name )
		{	}

		/*!
		 * Constructor with a vector of variable IDs. This vector is internally used by Objective
		 * to know what variables from the global variable vector it is handling.
		 * \param variables_index a const reference to a vector of IDs of variables composing the 
		 * objective function.
		 * \param name a const char* to give a name to the objective function.
		 */
		Maximize( const std::vector<int>& variables_index, const char* name )
			: Objective( variables_index, true, std::string( name ) )
		{	}

		/*!
		 * Constructor building a vector of variable IDs by calling v->get_id() from all variables v.
		 *
		 * \param variables a const reference to a vector of variable composing the objective function.
		 * \param name a const char* to give a name to the objective function.
		 */
		Maximize( const std::vector<Variable>& variables, const char* name )
			: Objective( variables, true, std::string( name ) )
		{	}
	};
}
