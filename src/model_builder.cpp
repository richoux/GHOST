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

#include "model_builder.hpp"

using ghost::ModelBuilder;
using ghost::Model;

ModelBuilder::ModelBuilder( bool permutation_problem )
	: permutation_problem( permutation_problem )
{ }

Model ModelBuilder::build_model()
{
	variables.clear();
	constraints.clear();

	declare_variables();
	// Set the id of each variable object to be their index in the _variables vector
	for( int variable_id = 0 ; variable_id < static_cast<int>( variables.size() ) ; ++variable_id )
	{
		variables[ variable_id ]._id = variable_id;
		if( variables[ variable_id ]._name.empty() )
			variables[ variable_id ]._name = "v" + std::to_string( variable_id );
	}

	// Auxiliary data may be needed by the constraints and the objective function,
	// so it must be defined before them.
	declare_auxiliary_data();		
	declare_constraints();
	declare_objective();

	if( constraints.empty() )
		constraints.emplace_back( std::make_shared<PureOptimization>( variables ) );
	
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

	return Model( std::move( variables ), constraints, objective, auxiliary_data, permutation_problem );
}

void ModelBuilder::create_n_variables( int number, const std::vector<int>& domain, int index )
{
	for( int i = 0 ; i < number ; ++i )
		variables.emplace_back( domain, index );
}

void ModelBuilder::create_n_variables( int number, int starting_value, std::size_t size, int index )
{
	for( int i = 0 ; i < number ; ++i )
		variables.emplace_back( starting_value, size, index );
}

void ModelBuilder::declare_constraints()
{ }

void ModelBuilder::declare_objective()
{
	objective = std::make_shared<NullObjective>();
}

void ModelBuilder::declare_auxiliary_data()
{
	auxiliary_data = std::make_shared<NullAuxiliaryData>();
}
