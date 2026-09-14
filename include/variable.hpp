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
 * Copyright (C) 2014-2026 Florian Richoux
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

#include <iostream>
#include <vector>
#include <string>
#include <algorithm>

#include "thirdparty/randutils.hpp"

namespace ghost
{
	/*!
	 * This class encodes variables of the model. Users cannot write classes inheriting 
	 * from ghost::Variable.
	 *
	 * In GHOST, all variables are discrete variables with a domain containing integers only 
	 * (positive, negative or both). Since no classes can inherits from ghost::Variable,
	 * if constraints or the objective function need specific details about variables
	 * (for instance, each variable models an agent with 2D coordinates), users need to either
	 * store these data into data structures in their own Constraints and/or their own Objective
	 * classes, or to place these data into their derived AuxiliaryData class, depending on their
	 * needs.
	 *
	 * \sa AuxiliaryData
	 */
	class Variable final
	{
		friend class VariableTest;
		friend class SearchUnit;
		friend class ModelBuilder;
		friend class Constraint;

		int _id; // Unique ID integer
		std::string _name;	// String to give a name to the variable, helpful to debug/trace.
		int	_current_value;	// Current value assigned to the variable.
		std::vector<int>* _domain; // Pointer to the domain of the variable (located in the model).
				
		struct valueException : std::exception
		{
			int value;
			valueException( int value ) : value( value ) {}
			std::string message = "Wrong value " + std::to_string( value ) + " passed to Variable::set_value. The given value does not belong to the domain.\n";
			const char* what() const noexcept { return message.c_str(); }
		};

		/*
		 * Constructor with the domain as input.
		 *
		 * \param id the ID of the variable.
		 * \param domain a pointer to the domain of the variable.
		 * \param index the position in the domain corresponding to the variable initial value. For instance with the domain [1, 2, 5, 7, 8], if index equals to 3 then the variable initial value is 7. By default, the index is zero. 
		 * \param name a const reference of a string to give a name to the variable. If no names are
		 * given, GHOST will automatically rename variables by "vx", with x the variable ID.
		 */
		Variable( int id,
		          std::vector<int>* domain,
		          int	index = 0,
		          const std::string& name = std::string() );

		// Default constructor (for instance, for vector initialization.)
		Variable() = default;

	public:
		//! Default copy contructor.
		Variable( const Variable& other ) = default;
		//! Default move contructor.
		Variable( Variable&& other ) = default;
		
		//! Copy assignment operator disabled.
		Variable& operator=( const Variable& other ) = default;
		//! Move assignment operator disabled.
		Variable& operator=( Variable&& other ) = default;
		
		//! Default virtual destructor.
		virtual ~Variable() = default;
				
		/*!
		 * Inline method to get the current value of the variable.
		 *
		 * \return An integer corresponding to the variable value.
		 */
		inline int get_value() const { return _current_value; }
		
		/*!
		 * Set the value of the variable.
		 *
		 * \param value an integer that must be a value in the domain to assign to the variable.
		 * \exception If the given value is not in the domain, raises a valueException.
		 */
		inline void	set_value( int value )
		{
			if( std::find( _domain->cbegin(), _domain->cend(), value ) == _domain->cend() )
				throw valueException( value );

			_current_value = value;
		}

		//! Inline accessor to get the variable name.
		inline std::string get_name() const { return _name; }

		//! Inline method to get the unique id of the Variable object.
		inline int get_id() const { return _id; }

		//! To have a nicer stream of Variable.
		friend std::ostream& operator<<( std::ostream& os, const Variable& v )
		{
			std::string domain = "";
			for( auto value : *(v._domain) )
				domain += std::to_string( value ) + std::string( ", " );

			return os
				<< "Variable name: " << v._name
				<< "\nId: " <<  v._id
				<< "\nValue: " <<  v._current_value
				<< "\nDomain: " << domain
				<< "\n--------";
		}
	};
}
