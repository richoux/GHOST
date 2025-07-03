/*
 * GHOST (General meta-Heuristic Optimization Solving Tool) is a C++ framework
 * designed to help developers to model and implement optimization problem
 * solving. It contains a meta-heuristic solver aiming to solve any kind of
 * combinatorial and optimization real-time problems represented by a CSP/COP/EF-CSP/EF-COP. 
 *
 * First developed to solve game-related optimization problems, GHOST can be used for
 * any kind of applications where solving combinatorial and optimization problems. In
 * particular, it had been designed to be able to solve not-too-complex problem instances
 * within some milliseconds, making it very suitable for highly reactive or embedded systems.
 * Please visit https://github.com/richoux/GHOST for further information.
 *
 * Copyright (C) 2014-2025 Florian Richoux
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
#include <utility>
#include <iostream>
#include <typeinfo>
#include <functional>
#include <cmath> // for isnan
#include <exception>
#include <string>

#include "variable.hpp"

namespace ghost
{
	namespace algorithms
	{
		class SearchUnit;
		class ErrorProjectionAdaptiveSearch;
		class ErrorProjectionCulpritSearch;
		class Regular;
		class SwitchOptimization;
	}

	/*!
	 * This is the base class from which users need to derive their Constraint classes. 
	 *
	 * ghost::Constraint cannot be directly used to encode user-defined constrains, since
	 * this is an abstract class. To declare a problem with GHOST, users have to make their
	 * own derived constraint classes.
	 *
	 * \sa Variable
	 */
	class Constraint
	{
		template<typename ModelBuilderType> friend class Solver;
		friend class ModelBuilder;
		friend class SearchUnit;
		friend class algorithms::ErrorProjectionAdaptiveSearch;
		friend class algorithms::ErrorProjectionCulpritSearch;
		friend class algorithms::Regular;
		friend class algorithms::SwitchOptimization;
		
		std::vector<Variable*> _variables;
		std::vector<int> _variables_index; // To know where are the constraint's variables in the global variable vector
		std::map<int,int> _variables_position; // To know where are global variables in the constraint's variables vector 

		double _current_error; // Current error of the constraint.

		int _id; // Unique ID integer
		mutable bool _is_optional_delta_error_defined; // Boolean telling if optional_delta_error() is overrided or not.

		struct nanException : std::exception
		{
			std::vector<Variable*> ptr_variables;
			std::vector<Variable> variables;
			std::string message;

			nanException( const std::vector<Variable*>& ptr_variables ) : ptr_variables(ptr_variables)
			{
				message = "Constraint required_error returned a NaN value on variables (";
				for( int i = 0; i < static_cast<int>( ptr_variables.size() ) - 1; ++i )
					message += std::to_string( ptr_variables[i]->get_value() ) + ", ";
				message += std::to_string( ptr_variables[ static_cast<int>( ptr_variables.size() ) - 1 ]->get_value() ) + ")\n";
			}

			nanException( const std::vector<Variable>& variables ) : variables(variables)
			{
				message = "Constraint optional_delta_error returned a NaN value on variables (";
				for( int i = 0; i < static_cast<int>( variables.size() ) - 1; ++i )
					message += std::to_string( variables[i].get_value() ) + ", ";
				message += std::to_string( variables[ static_cast<int>( variables.size() ) - 1 ].get_value() ) + ")\n";
			}

			const char* what() const noexcept { return message.c_str(); }
		};

		struct negativeException : std::exception
		{
			std::vector<Variable*> ptr_variables;
			std::vector<Variable> variables;
			std::string message;

			negativeException( const std::vector<Variable*>& ptr_variables ) : ptr_variables(ptr_variables)
			{
				message = "Constraint required_error returned a negative value on variables (";
				for( int i = 0; i < static_cast<int>( ptr_variables.size() ) - 1; ++i )
					message += std::to_string( ptr_variables[i]->get_value() ) + ", ";
				message += std::to_string( ptr_variables[ static_cast<int>( ptr_variables.size() ) - 1 ]->get_value() ) + ")\n";
			}

			const char* what() const noexcept { return message.c_str(); }
		};

		struct deltaErrorNotDefinedException : std::exception
		{
			std::string message;

			deltaErrorNotDefinedException()
			{
				message = "Constraint::optional_delta_error() has not been user-defined.\n";
			}
			const char* what() const noexcept { return message.c_str(); }
		};

		struct variableOutOfTheScope : std::exception
		{
			std::string message;

			variableOutOfTheScope( int var_id, int ctr_id )
			{
				message = "Variable* ID " + std::to_string( var_id ) + " is not in the scope of Constraint ID " + std::to_string( ctr_id ) + ".\n";
			}
			const char* what() const noexcept { return message.c_str(); }
		};

		inline bool is_optional_delta_error_defined() { return _is_optional_delta_error_defined; }

		// Call required_error() after getting sure the error does give a nan, rise an exception otherwise.
		double error() const;

		// Compute the delta error of the current assignment, giving a vector of variables index and their candidate values.
		// Calling optional_delta_error after making the conversion of variables index.
		// Getting sure the delta error does give a nan, rise an exception otherwise.
		double delta_error( const std::vector<int>& variables_index, const std::vector<int>& candidate_values ) const;

		// To simulate the error delta between the current configuration and the candidate configuration.
		// This calls delta_error() if the user overrided it, otherwise it makes the simulation 'by hand' and calls error()
		double simulate_delta( const std::vector<int>& variables_index, const std::vector<int>& candidate_values );

		// Return ids of variable objects in _variables.
		inline std::vector<int> get_variable_ids() const { return _variables_index; }

		inline void update( int index, int new_value ) { conditional_update_data_structures( _variables, _variables_position[ index ], new_value ); }

	protected:
		/*!
		 * Pure virtual method to compute the error of the constraint regarding the values of
		 * variables given as input.
		 *
		 * This method is fundamental: as a predicate, it evalutes if the given values of 
		 * variables violate this contraint, and as an error function, it evaluates how much.\n
		 * Let's consider the following example to understand error functions: consider the
		 * contraint (x = y).\n
		 * If x = 42 and y = 42, then these values satify the contraint. The error is then 0.\n
		 * If x = 42 and y = 40, the constraint is not satified, but intuitively, we are closer
		 * to have a solution than with x = 42 and y = 10,000. Thus the error when y = 40 must
		 * be strictly lower than the error when y = 10,000.\n
		 * Thus, a required_error candidate for the contraint (x = y) could be the function |x-y|.
		 *
		 * This method MUST returns a value greater than or equals to 0.
		 *
		 * Users have the choice: while modeling CSP or COP problems, required_error must implement
		 * the logic of a predicate and should outputs 0 if current values of variables
		 * satisfy the user-defined constraint, and something strictly higher than 0 otherwise,
		 * like 1 for instance.
		 *
		 * While modeling EF-CSP/EF-COP problems, required_error needs to express an error function.
		 * It still must outputs 0 for satisfying values of variables, but must
		 * outputs a value strictly higher than 0 otherwise, such that the higher this value,
		 * the further current values of variables are from satisfying the user-defined constraint.
		 *
		 * Like any methods prefixed by 'required_', overriding this method is mandatory.
		 *
		 * \warning DO NOT implement any side effect in this method. It is called by the solver 
		 * to compute the constraint error but also for some inner mechanisms (such as error 
		 * simulations).
		 *
		 * \param variables a const reference of the vector of raw pointers to variables in the
		 * scope of the constraint. The solver is actually calling this method with the vector 
		 * of variables that has been given to the constructor.
		 * \return A **positive** double corresponding to the error of the constraint.
		 * Outputing 0 means that given variable values satisfy the constraint.
		 * \exception Throws an exception if the computed value is negative or is NaN.
		 */
		virtual double required_error( const std::vector<Variable*>& variables ) const = 0;

		/*!
		 * Virtual method to compute the difference, or delta, between the current error and
		 * the error of a candidate assignment.
		 *
		 * The current assignment, as well as its error, are automatically stored in the class
		 * Constraint. Giving a vector of variable indexes and their respective candidate value,
		 * this methods ouputs the difference between the error of the current assignment in
		 * 'variables' given as input and the error we would get if we assign new candidate values.
		 *
		 * The ouput can be negative, positive, or equals to 0. If the candidate error is strictly
		 * lower (then better) than the current error, the ouput is negative. If errors are the
		 * same, the ouputs equals to 0. Finally, if the candidate error is strictly higher
		 * (then worst) than the current error, the ouput is positive.
		 *
		 * For EF-CSP/EF-COP models, this method can be VERY important to have faster computation. 
		 * Although optional (the solver still works without it), we strongly advise users to define
		 * it properly, unless the overridden required_error method is trivial to compute.
		 * Having this method may make a big difference for the solver to quickly find better
		 * solutions. However, if the problem is modeled as an CSP/COP, users can just skip
		 * the implementation of this method.
		 *
		 * Like any methods prefixed by 'optional_', overriding this method is not mandatory.
		 *
		 * \warning DO NOT implement any side effect in this method. It is called by the solver 
		 * to compute the constraint delta error but also for some inner mechanisms (such as error
		 * simulations).
		 *
		 * \param variables a const reference of the vector of raw pointers of variables in the scope
		 * of the constraint. The solver is actually calling this method with the vector of variables
		 * that has been given to the constructor.
		 * \param indexes the vector of indexes of variables that are reassigned.
		 * \param candidate_values the vector of their respective candidate values.
		 * \return A double corresponding to the difference between the current error of the
		 * constraint and the error one would get if the solver assigns candidate values to given
		 * variables.
		 * \exception Throws an exception if the computed value is NaN.
		 */
		virtual double optional_delta_error( const std::vector<Variable*>& variables, const std::vector<int>& indexes, const std::vector<int>& candidate_values ) const;

		/*!
		 * Update user-defined data structures in the constraint.
		 *
		 * Like any methods prefixed by 'conditional_', this method must be overriden under
		 * some conditions: if some inner data structures are defined in derived constraint
		 * classes and need to be updated while variable values change (i.e., when the solver
		 * asssign 'new_value' to variables[index]), this method must be implemented to define
		 * how data structures must be updated.
		 *
		 * \param variables a const reference of the vector of raw pointers to variables of the 
		 * constraint.
		 * \param index an integer to get the variable 'variables[index]' that has been updated by
		 * the solver.
		 * \param new_value an integer to know what is the new value of 'variables[index]'.
		 */
		virtual void conditional_update_data_structures( const std::vector<Variable*>& variables, int index, int new_value );

		/*!
		 * Inline method returning the current error of the constraint (automatically updated by the
		 * solver). This can be helpful for implementing optional_delta_error.
		 * \sa optional_delta_error
		 */
		inline double get_current_error() const { return _current_error; }

		//! Inline method to get the unique id of the Constraint object.
		inline int get_id() const { return _id; }

	public:
		/*!
		 * Constructor with a vector of variable IDs. This vector is internally used by Constraint
		 * to know what variables from the global variable vector it is handling.
		 * \param variables a const reference to a vector of IDs of variables composing the constraint.
		 */
		Constraint( const std::vector<int>& variables_index );

		/*!
		 * Constructor building a vector of variable IDs by calling v->get_id() from all variables v.
		 *
		 * \param variables a const reference to a vector of variable composing the constraint.
		 */
		Constraint( const std::vector<Variable>& variables );

		//! Default copy contructor.
		Constraint( const Constraint& other ) = default;
		//! Default move contructor.
		Constraint( Constraint&& other ) = default;

		//! Copy assignment operator disabled.
		Constraint& operator=( const Constraint& other ) = delete;
		//! Move assignment operator disabled.
		Constraint& operator=( Constraint&& other ) = delete;

		//! Default virtual destructor.
		virtual ~Constraint() = default;

		/*!
		 * Determine if the constraint contains a variable given its id.
		 *
		 * \param var_id an integer being a variable ID
		 */
		bool has_variable( int var_id ) const;

		//! To have a nicer stream of Constraint.
		friend std::ostream& operator<<( std::ostream& os, const Constraint& c )
		{
			return os << "Constraint type: " <<  typeid(c).name()
			          << "\nId: " <<  c._id
			          << "\n########";
		}
	};

	/**********************/
	/** PureOptimization **/
	/**********************/
	// PureOptimization is used when no constraints have been given to the solver (ie, for pure optimization runs). 
	class PureOptimization : public Constraint
	{
		double required_error( const std::vector<Variable*>& variables ) const
		{
			return 0.;
		}

		double optional_delta_error( const std::vector<Variable*>& variables, const std::vector<int>& indexes, const std::vector<int>& candidate_values ) const
		{
			return 0.;
		}

	public:
		PureOptimization( const std::vector<Variable>& variables )
			: Constraint( variables )
		{ }
	};

}
