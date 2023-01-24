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
 * Copyright (C) 2014-2023 Florian Richoux
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

namespace ghost
{
	/*******************/
	/** AuxiliaryData **/
	/*******************/

	/*!
	 * ghost::AuxiliaryData is a class users eventually need to derive from if they need to keep
	 * track with some auxiliary data outside variable values.
	 *
	 * ghost::AuxiliaryData cannot be directly used to encode auxiliary data of a model, since
	 * this is an abstract class.
	 *
	 * Derived classes would contain all data outside variable values users need to keep updated.
	 * The only method to override is required_update(variables, index, new_value), defining how
	 * data should be updated when the solver assigns the value 'new_value' to the variable
	 * 'variables[index]'.
	 */
	class AuxiliaryData
	{
		friend class SearchUnit;
		friend class ModelBuilder;

		std::vector<Variable*> _variables;
		std::vector<int> _variables_index; // To know where are the constraint's variables in the global variable vector
		std::map<int,int> _variables_position; // To know where are global variables in the constraint's variables vector 

		void update();
		void update( int index, int new_value );

	protected:
		/*!
		 * Method to handle what should happen to the auxiliary data if variables[index] is updated
		 * with the value 'new_value'.
		 *
		 * Like any methods prefixed by 'required_', overriding this method is mandatory.
		 *
		 * \param variables a const reference of the vector of raw pointers to variables that are
		 * relevant to the auxiliary data.
		 * \param index an integer to get the variable 'variables[index]' that has been updated by
		 * the solver.
		 * \param new_value an integer to know what is the new value of 'variables[index]'.
		 */
		virtual void required_update( const std::vector<Variable*>& variables, int index, int new_value ) = 0;

	public:
		//! Constructor instanciating an empty vector of variable IDs
		AuxiliaryData();

		/*!
		 * Constructor with a vector of variable IDs. This vector is internally used by AuxiliaryData
		 * to know what variables from the global variable vector it is handling.
		 * \param variables a const reference to a vector of IDs of variables needed for the
		 * auxiliary data.
		 */
		AuxiliaryData( const std::vector<int>& variables_index );

		/*!
		 * Constructor building a vector of variable IDs by calling v->get_id() from all variables v.
		 * \param variables a const reference to a vector of variable composing the constraint.
		 */
		AuxiliaryData( const std::vector<Variable>& variables );

		//! Default copy contructor.
		AuxiliaryData( const AuxiliaryData& other ) = default;
		//! Default move contructor.
		AuxiliaryData( AuxiliaryData&& other ) = default;

		//! Copy assignment operator disabled.
		AuxiliaryData& operator=( const AuxiliaryData& other ) = delete;
		//! Move assignment operator disabled.
		AuxiliaryData& operator=( AuxiliaryData&& other ) = delete;

		//! Default virtual destructor.
		virtual ~AuxiliaryData() = default;
	};

	/***********************/
	/** NullAuxiliaryData **/
	/***********************/
	// NullAuxiliaryData is used when no auxiliary data are necessary in the model.
	class NullAuxiliaryData : public AuxiliaryData
	{
	public:
		NullAuxiliaryData()
			: AuxiliaryData()
		{ }

		void required_update( const std::vector<Variable*>& variables, int index, int new_value ) override { }
	};
}
