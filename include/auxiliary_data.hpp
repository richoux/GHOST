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

#include "variable.hpp"

namespace ghost
{
	/*******************/
	/** AuxiliaryData **/
	/*******************/
	class AuxiliaryData
	{
		friend class SearchUnit;
		friend class FactoryModel;

		std::vector<Variable*> _variables;
		std::vector<int> _variables_index; // to know where are the constraint's variables in the global variable vector
		std::map<int,int> _variables_position; // to know where are global variables in the constraint's variables vector 

		void update();		
		void update( int index, int new_value );		

	protected:
		virtual void update( const std::vector<Variable*>& variables, int index, int new_value ) = 0;
	
	public:
		AuxiliaryData( const std::vector<int>& variables_index );
		AuxiliaryData( const std::vector<Variable>& variables );
				
		//! Default copy contructor.
		AuxiliaryData( const AuxiliaryData& other ) = default;
		//! Default move contructor.
		AuxiliaryData( AuxiliaryData&& other ) = default;
    
		//! Copy assignment operator disabled.
		AuxiliaryData& operator=( const AuxiliaryData& other ) = delete;
		//! Move assignment operator disabled.
		AuxiliaryData& operator=( AuxiliaryData&& other ) = delete;

		virtual ~AuxiliaryData() = default;
	};
	
	/***********************/
	/** NullAuxiliaryData **/
	/***********************/
  //! NullAuxiliaryData is used when no auxiliary data are necessary in the model.
	class NullAuxiliaryData : public AuxiliaryData
	{
	public:
		NullAuxiliaryData()
			: AuxiliaryData( std::vector<int>{0} )
		{ }
		
		void update( const std::vector<Variable*>& variables, int index, int new_value ) override { }
	};
}
