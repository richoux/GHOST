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

#include "factory_model.hpp"

using namespace ghost;

Model FactoryModel::make_model()
{
	variables.clear();
	constraints.clear();

	declare_variables();
	// Set the id of each variable object to be their index in the _variables vector
	for( int variable_id = 0 ; variable_id < static_cast<int>( variables.size() ) ; ++variable_id )
		variables[ variable_id ]._id = variable_id;

	// Auxiliary data may be needed by the constraints and the objective function,
	// so it must be defined before them.
	declare_auxiliary_data();
	declare_constraints();
	declare_objective();

	// Internal data structure initialization
	// Set the id of each constraint object to be their index in the _constraints vector
	for( int constraint_id = 0 ; constraint_id < static_cast<int>( constraints.size() ) ; ++constraint_id )
	{
		constraints[ constraint_id ]->_id = constraint_id;
		// Set also constraints' variables and their internal data structures
		for( int index = 0 ; index < static_cast<int>( constraints[ constraint_id ]->_variables_index.size() ) ; ++index )
		{
			constraints[ constraint_id ]->_variables.push_back( &variables[ constraints[ constraint_id ]->_variables_index[ index ] ] );
			constraints[ constraint_id ]->_variables_position[ constraints[ constraint_id ]->_variables_index[ index ] ] = index;
		}
	}

	// Set auxiliary data's variables and its internal data structures
	for( int index = 0 ; index < static_cast<int>( auxiliary_data->_variables_index.size() ) ; ++index )
	{
		auxiliary_data->_variables.push_back( &variables[ auxiliary_data->_variables_index[ index ] ] );
		auxiliary_data->_variables_position[ auxiliary_data->_variables_index[ index ] ] = index;
	}

	// Set objective function's variables and its internal data structures
	for( int index = 0 ; index < static_cast<int>( objective->_variables_index.size() ) ; ++index )
	{
		objective->_variables.push_back( &variables[ objective->_variables_index[ index ] ] );
		objective->_variables_position[ objective->_variables_index[ index ] ] = index;
	}
	
	return Model( std::move( variables ), constraints, objective, auxiliary_data );
}

void FactoryModel::declare_objective()
{
	objective = std::make_shared<NullObjective>();
}

void FactoryModel::declare_auxiliary_data()
{
	auxiliary_data = std::make_shared<NullAuxiliaryData>();
}
