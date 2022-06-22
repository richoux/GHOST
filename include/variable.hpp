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
 * Copyright (C) 2014-2022 Florian Richoux
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
		friend class SearchUnit;
		friend class ModelBuilder;

		std::vector<int> _domain; // The domain, i.e., the vector of values the variable can take.
		int _id; // Unique ID integer
		std::string _name;	// String to give a name to the variable, helpful to debug/trace.

		int	_current_value;	// Current value assigned to the variable.
		int _min_value; // minimal value in the domain
		int _max_value; // maximal value in the domain
		randutils::mt19937_rng _rng; // Neat random generator implemented in thirdparty/randutils.hpp,
		                             // see https://www.pcg-random.org/posts/ease-of-use-without-loss-of-power.html

		struct valueException : std::exception
		{
			int value;
			int min;
			int max;
			valueException( int value, int min, int max ) : value( value ), min( min ), max( max ) {}
			std::string message = "Wrong value " + std::to_string( value ) + " passed to Variable::set_value. The given value does not belong to the domain and/or is not be between "
				+ std::to_string( min ) + " (included) and "
				+ std::to_string( max ) + " (included).\n";
			const char* what() const noexcept { return message.c_str(); }
		};

		// Assign to the variable a random values from its domain.
		inline void pick_random_value()	{	_current_value = _rng.pick( _domain ); }

	public:
		//! Default constructor
		Variable() = default;

		/*!
		 * Constructor with the domain as input.
		 *
		 * This is the only constructor able to give a non-contiguous domain to a variable, i.e.,
		 * a domain with "holes" like [1, 2, 5, 7, 8], where 3, 4 and 6 are missing.
		 *
		 * \param domain a const reference to the vector of integers composing the domain
		 * to create.
		 * \param index the position in the domain corresponding to the variable initial value.
		 * For instance with the domain [1, 2, 5, 7, 8], if index equals to 3 then the variable 
		 * initial value is 7. By default, the index is zero.
		 * \param name a const reference of a string to give a name to the variable. If no names are
		 * given, GHOST will automatically rename variables by "vx", with x the variable ID.
		 */
		Variable( const std::vector<int>& domain,
		          int	index = 0,
		          const std::string& name = std::string() );

		/*!
		 * Constructor building a contiguous domain with all intergers from starting_value to 
		 * starting_value + size-1.
		 *
		 * \param starting_value an integer representing the first value of the domain.
		 * The creating domain will then be the interval [starting_value, starting_value + size - 1].
		 * \param size a size_t corresponding to the size of the domain to create.
		 * \param index the position in the domain corresponding to the variable initial value.
		 * For instance with the domain [1, 2, 5, 7, 8], if index equals to 3 then the variable 
		 * initial value is 7. By default, the index is zero.
		 * \param name a const reference of a string to give a name to the variable. If no names are
		 * given, GHOST will automatically rename variables by "vx", with x the variable ID.
		 */
		Variable( int starting_value,
		          std::size_t size,
		          int	index = 0,
		          const std::string& name = std::string() );

		/*!
		 * Equivalent to the constructor Variable(domain, index, name).
		 *
		 * It simply calls Variable(domain, 0, name).
		 *
		 * \param domain a const reference to the vector of integers composing the domain
		 * to create.
		 * \param name a const reference of a string to give a name to the variable.
		 */
		Variable( const std::vector<int>& domain,
		          const std::string& name );

		/*!
		 * Equivalent to the constructor Variable(starting_value, size, index, name).
		 *
		 * It simply calls Variable(starting_value, size, 0, name).
		 *
		 * \param starting_value an integer representing the first value of the domain.
		 * The creating domain will then be the interval [starting_value, starting_value + size - 1].
		 * \param size a size_t corresponding to the size of the domain to create.
		 * \param name a const reference of a string to give a name to the variable.
		 */
		Variable( int starting_value,
		          std::size_t size,
		          const std::string& name );

		/*!
		 * Inline method returning the domain.
		 *
		 * \return The vector of integers composing the domain.
		 */
		inline std::vector<int> get_full_domain() const { return _domain; }

		/*!
		 * Method returning the range of values
		 * [current_value - range/2 [mod domain_size], current_value + range/2 [mod domain_size]]
		 * from the domain.
		 *
		 * \return A vector containing these integers.
		 */
		std::vector<int> get_partial_domain( int range ) const;

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
			if( std::find( _domain.cbegin(), _domain.cend(), value ) == _domain.cend() )
				throw valueException( value, get_domain_min_value(), get_domain_max_value() );

			_current_value = value;
		}

		/*!
		 * Inline method returning the size of the domain of the variable.
		 *
		 * \return A size_t equals to size of the domain of the variable.
		 */
		inline std::size_t get_domain_size() const { return _domain.size(); }

		/*!
		 * Inline method returning the minimal value in the variable's domain.
		 *
		 * \return The minimal value in the domain, in constant time.
		 */
		inline int get_domain_min_value() const { return _min_value; }

		/*!
		 * Inline method returning the maximal value in the variable's domain.
		 *
		 * \return The maximal value in the domain, in constant time.
		 */
		inline int get_domain_max_value() const { return _max_value; }

		//! Inline accessor to get the variable name.
		inline std::string get_name() const { return _name; }

		//! Inline method to get the unique id of the Variable object.
		inline int get_id() const { return _id; }

		//! To have a nicer stream of Variable.
		friend std::ostream& operator<<( std::ostream& os, const Variable& v )
		{
			std::string domain = "";
			for( auto value : v.get_full_domain() )
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
