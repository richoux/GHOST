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

#include <numeric>
#include <sstream>

#include "model_builder.hpp"

using ghost::ModelBuilder;
using ghost::Model;

ModelBuilder::ModelBuilder( bool permutation_problem )
	: permutation_problem( permutation_problem )
{ }

Model ModelBuilder::build_model()
{
	// Necessary reinitializations, if build_model() is called more than once during the program execution.
	_number_variables_created = 0;
	variables.clear();
	domains.clear();
	_created_domains.clear();
	_domain_of_variable.clear();
	constraints.clear();
	objective = nullptr;

	declare_variables();

	// Set the id of each variable object to be their index in the _variables vector
	for( int variable_id = 0 ; variable_id < static_cast<int>( variables.size() ) ; ++variable_id )
		if( variables[ variable_id ]._name.empty() )
			variables[ variable_id ]._name = "v" + std::to_string( variable_id );

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

	return Model( std::move( variables ),
	              std::move( domains ),
	              std::move( _domain_of_variable ),
	              constraints,
	              objective,
	              auxiliary_data,
	              permutation_problem );
}

int ModelBuilder::fake_building_and_count_variables()
{
	_number_variables_created = 0;
	variables.clear();
	domains.clear();
	_created_domains.clear();
	_domain_of_variable.clear();

	declare_variables();

	int nb_vars = _number_variables_created;
	_number_variables_created = 0;
	variables.clear();
	domains.clear();
	_created_domains.clear();
	_domain_of_variable.clear();

	return nb_vars;
}

std::string ModelBuilder::make_key( const std::vector<int>& vec)
{
	std::stringstream stream;
	for( const auto& element : vec )
		stream << element << "_";
	return stream.str();
}

void ModelBuilder::create_variable( std::vector< int > domain, const std::string& name, int index )
{
	int var_id = static_cast<int>( variables.size() );

	std::string key = make_key( domain );
	if( _created_domains.contains( key ) )
		_domain_of_variable[ var_id ] = _created_domains[ key ] ;
	else
	{
		domains.emplace_back( domain );
		int dom_index = static_cast<int>( domains.size() ) - 1;
		_domain_of_variable[ var_id ] = dom_index;
		_created_domains[ key ] = dom_index;
	}

	variables.push_back( ghost::Variable( _number_variables_created++,
	                                      &(domains[ _domain_of_variable[ var_id ] ]),
	                                      index,
	                                      name ) );
}

void ModelBuilder::create_variable( int starting_value, std::size_t size, const std::string& name, int index )
{
	std::vector<int> dom( size );
	std::iota( dom.begin(), dom.end(), starting_value );

	create_variable( dom, name, index );
}

void ModelBuilder::create_n_variables( int number, const std::vector<int>& domain, int index )
{
	for( int i = 0 ; i < number ; ++i )
		create_variable( domain, std::string(), index );
}

void ModelBuilder::create_n_variables( int number, int starting_value, std::size_t size, int index )
{
	for( int i = 0 ; i < number ; ++i )
		create_variable( starting_value, size, std::string(), index );
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
