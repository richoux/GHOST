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
		template <typename ... ConstraintType> friend class Solver;

		static unsigned int NBER_CTR; //!< Static counter that increases each time one instantiates a Constraint object.
		unsigned int _id;	//!< Unique ID integer
		std::vector<Variable> _variables;	//!< Vector of variable composing the model.
		std::map<unsigned int,int> _id_mapping; // Mapping between the variable's id in the solver (new_id) and its position in the vector of variables within the constraint.
		mutable bool _is_expert_delta_error_defined;
		double _current_error;		
		
		struct nanException : std::exception
		{
			std::vector<Variable> variables;
			std::string message;

			nanException( const std::vector<Variable>& variables ) : variables(variables)
			{
				message = "Constraint required_error returned a NaN value on variables (";
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

			variableOutOfTheScope( unsigned int var_id, unsigned int ctr_id )
			{
				message = "Variable ID " + std::to_string( var_id ) + " is not in the scope of Constraint ID " + std::to_string( ctr_id ) + ".\n";
			}
			const char* what() const noexcept { return message.c_str(); }
		};

		// Update a variable assignment.
		inline void update_variable( unsigned int variable_id, int new_value ) { _variables[ _id_mapping[ variable_id ] ].set_value( new_value ); }

		// Making the mapping between the variable's id in the solver (new_id) and its position in the vector of variables within the constraint. 
		void make_variable_id_mapping( unsigned int new_id, unsigned int original_id );
		
		// To simulate the error delta between the previous and the new error.
		double simulate( const std::vector<std::pair<unsigned int, int>>& changes );		 
		
	protected:
		//! Pure virtual method to compute the current error of the constraint.
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
		 * If you are modeling a CFN problem, then it still must outputs 0 for satisfying values of variables, 
		 * but must outputs a value strictly higher than 0 otherwise, such that the higher this value, the further
		 * current values of variables are from satisfying your constraint.
		 *
		 * \warning Do not implement any side effect in this method. It is called by the solver 
		 * to compute the constraint error but also for some inner mechanisms (such as error simulations).
		 *
		 * \param The vector of variables representing the current assignment.
		 * \return A positive double corresponding to the error of the constraint with current variable values. 
		 * Outputing 0 means current values are satisfying this constraint.
		 * \sa error
		 */
		virtual double required_error( const std::vector<Variable>& variables ) const = 0;

		virtual double expert_delta_error( const std::vector<std::pair<unsigned int, int>>& changes ) const;
	public:
		//! Unique constructor
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
    
		//! Calling required_error (indirectly).
		/*!
		 * @throw nanException
		 * \sa required_error, error
		 */
		inline double error() const
		{
			return error( _variables );
		}

		//! Method to compute the error of a given assignment, calling required_error.
		/*!
		 * @throw nanException
		 * \sa required_error, error
		 */
		double error( const std::vector<Variable>& variables ) const
		{
			double value = required_error( variables );
			if( std::isnan( value ) )
				throw nanException( variables );
			return value;
		}

		//! Method to determine if the constraint contains a given variable. 
		/*!
		 * Given a variable, returns if it composes the constraint.
		 *
		 * \param var A variable.
		 * \return True iff the constraint contains var.
		 */ 
		bool has_variable( const Variable& var ) const;

		//! Inline method to get the unique id of the Constraint object.
		inline int get_id() const { return _id; }

		// To have a nicer stream of Constraint.
		friend std::ostream& operator<<( std::ostream& os, const Constraint& c )
		{
			return os << "Constraint type: " <<  typeid(c).name() << "\n";
		}

		// // To let the access of Constraint::simulate() to Solver::solve
		// friend bool Solver::solve( double& finalCost,
		//                            std::vector<int>& finalSolution,
		//                            double sat_timeout,
		//                            double opt_timeout,
		//                            bool no_random_starting_point );
	};
}
