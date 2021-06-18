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
	//! This class encodes constraints of your model.
	/*! 
	 * You cannot directly use this class Constraint to encode constrains of 
	 * your model, since this is an abstract class. To declare a problem with GHOST, 
	 * you have to make your own constraints by inheriting from this class. 
	 * The same problem can be composed of several subclasses of Constraint.
	 *
	 * The only pure virtual Constraint method is required_cost, following the 
	 * Non-Virtual Interface Idiom (see http://www.gotw.ca/publications/mill18.htm).
	 *
	 * \sa Variable
	 */
	class Constraint
	{
		friend class SearchUnit;
		friend class ModelBuilder;

		std::vector<Variable*> _variables;
		std::vector<int> _variables_index; // to know where are the constraint's variables in the global variable vector
		std::map<int,int> _variables_position; // to know where are global variables in the constraint's variables vector 
		
		double _current_error; //!< Current error of the constraint. 

		int _id;	// Unique ID integer
		mutable bool _is_expert_delta_error_defined; // Boolean telling if expert_delta_error() is overrided or not.

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
				message = "Constraint expert_delta_error returned a NaN value on variables (";
				for( int i = 0; i < static_cast<int>( variables.size() ) - 1; ++i )
					message += std::to_string( variables[i].get_value() ) + ", ";
				message += std::to_string( variables[ static_cast<int>( variables.size() ) - 1 ].get_value() ) + ")\n";
			}

			const char* what() const noexcept { return message.c_str(); }
		};

		struct deltaErrorNotDefinedException : std::exception
		{
			std::string message;

			deltaErrorNotDefinedException()
			{
				message = "Constraint::expert_delta_error() has not been user-defined.\n";
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

		// Update a variable assignment.
		// void update_variable( int variable_index, int new_value );
		
		// Making the mapping between the variable's id in the solver (new_id) and its position in the vector of variables within the constraint. 
		// void make_variable_id_mapping( int new_id, int original_id );
		
		inline bool is_expert_delta_error_defined() { return _is_expert_delta_error_defined; }

		// Call required_error() after getting sure the error does give a nan, rise an exception otherwise.
		double error() const;
			
		// Compute the delta error of the current assignment, giving a vector of variables index and their candidate values.
		// Calling expert_delta_error after making the conversion of variables index.
		// Getting sure the delta error does give a nan, rise an exception otherwise.
		double delta_error( const std::vector<int>& variables_index, const std::vector<int>& candidate_values ) const;

		// To simulate the error delta between the current configuration and the candidate configuration.
		// This calls delta_error() if the user overrided it, otherwise it makes the simulation 'by hand' and calls error()
		double simulate_delta( const std::vector<int>& variables_index, const std::vector<int>& candidate_values );

		// Determine if the constraint contains a variable given its id. 
		bool has_variable( int var_id ) const;

		// Return ids of variable objects in _ptr_variables.
		inline std::vector<int> get_variable_ids() const { return _variables_index; }

		inline void update( int index, int new_value ) { expert_update_if_delta_error_defined( _variables, _variables_position[ index ], new_value ); }
		
	protected:
		//! Pure virtual method to compute the error of the constraint with the current assignment in Constraint::_ptr_variables.
		/*!
		 * This method is fundamental: it evalutes how much the current values of variables violate this contraint.
		 * Let's consider the following example : consider the contraint (x = y).\n
		 * If x = 42 and y = 42, then these values satify the contraint. The error is then 0.\n
		 * If x = 42 and y = 40, the constraint is not satified, but intuitively, we are closer to have a solution than with
		 * x = 42 and y = 10,000. Thus the error when y = 40 must be strictly lower than the error when y = 10,000.\n
		 * Thus, a required_error candidate for the contraint (x = y) could be the function |x-y|.
		 * 
		 * This method MUST returns a value greater than or equals to 0.
		 *
		 * We have the choice: if your are modeling a CSP or COP problem, then required_error should outputs 0 if
		 * current values of variables satisfy your constraint, and something strictly higher than 0, like 1 
		 * for instance, otherwise.
		 * If you are modeling an EFSP/EFOP problem, then it still must outputs 0 for satisfying values of variables, 
		 * but must outputs a value strictly higher than 0 otherwise, such that the higher this value, the further
		 * current values of variables are from satisfying your constraint.
		 *
		 * \warning DO NOT implement any side effect in this method. It is called by the solver 
		 * to compute the constraint error but also for some inner mechanisms (such as error simulations).
		 *
		 * \param variables A const reference of the vector of variables in the scope of the constraint. The solver is calling this method with
		 * Constraint::_ptr_variables as input. This is mostly to have the same interface than Objective::required_cost.
		 * \return A positive double corresponding to the error of the constraint.
		 * Outputing 0 means that given variable values satisfy the constraint. 
		 */
		virtual double required_error( const std::vector<Variable*>& variables ) const = 0;

		//! Virtual method to compute the difference, or delta, between the current error and a the error of a candidate assignment.
		/*!
		 * The current assignment, as well as its error, are automatically stored in the class Constraint.
		 * Giving a vector of variable indexes and their respective candidate value, this methods ouputs the difference between the 
		 * error of the current assignment in Constraint::_ptr_variables and the error we would get if we assign new candidate values to variables given as input.
		 *
		 * The ouput can be negative, positive, or equals to 0. If the candidate error is strictly lower (then better) than the current error, 
		 * the ouput is negative. If errors are the same, the ouputs equals to 0. Finally, if the candidate error is strictly higher (then worth) 
		 * than the current error, the ouput is positive.
		 * 
		 * For instance, let's assume the constraint is #find a better example#
		 * 
		 * If your problem is modeled as an EFSP/EFOP, then this method is VERY important to have faster computation. Although optional (the solver still works without it),
		 * we strongly advise you to define it properly, unless your required_error method is trivial to compute. Having this method may make a big difference for the solver 
		 * to quickly find better solutions.
		 * However, if your problem is modeled as an CSP/COP, you can just skip implementing this method.
		 *
		 * \warning DO NOT implement any side effect in this method. It is called by the solver 
		 * to compute the constraint delta error but also for some inner mechanisms (such as error simulations).
		 *
		 * \param variables A const reference of the vector of variables in the scope of the constraint. The solver is calling this method with
		 * Constraint::_ptr_variables as input.
		 * \param The vector of variables indexes and the vector of their respective candidate values.
		 * \return A double corresponding to the difference between the current error of the constraint and the error you would get if you assign candidate 
		 * values to given variables.
		 */
		virtual double expert_delta_error( const std::vector<Variable*>& variables, const std::vector<int>& variable_indexes, const std::vector<int>& candidate_values ) const;

		// To allow users to update their inner constraint data structure after assigning to a variable a new value.
		virtual void expert_update_if_delta_error_defined( const std::vector<Variable*>& variables, int variable_index, int new_value );

		//! Inline method returning the current error of the constraint (automatically updated by the solver). This is useful to implement expert_delta_error.
		/*!
		 * \sa expert_delta_error
		 */
		inline double get_current_error() const { return _current_error; }

		// Inline method to get the unique id of the Constraint object.
		inline int get_id() const { return _id; }

	public:
		//! Constructor taking variable indexes
		/*!
		 * \param variables A const reference to a vector of variable composing the constraint.
		 */
		Constraint( const std::vector<int>& variables_index );

		//! Constructor taking variables
		/*!
		 * \param variables A const reference to a vector of variable composing the constraint.
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
		
		// To have a nicer stream of Constraint.
		friend std::ostream& operator<<( std::ostream& os, const Constraint& c )
		{
			return os << "Constraint type: " <<  typeid(c).name()
			          << "\nId: " <<  c._id
			          << "\n########";
		}
	};
}
