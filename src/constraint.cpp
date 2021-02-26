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

#include <algorithm>
#include <iterator>
#include <limits>

#include "constraint.hpp"

using namespace ghost;

unsigned int Constraint::NBER_CTR = 0;

Constraint::Constraint( const std::vector<Variable>& variables )
	: _variables( variables ),
	  _is_expert_delta_error_defined( true )
{
	if( NBER_CTR < std::numeric_limits<unsigned int>::max() )
		_id = NBER_CTR++;
	else
		_id = NBER_CTR = 0;
}

void Constraint::make_variable_id_mapping( unsigned int new_id, unsigned int original_id )
{
	auto iterator = std::find_if( _variables.begin(), _variables.end(), [&](auto& v){ return v.get_id() == original_id; } );
	if( iterator == _variables.end() )
		throw variableOutOfTheScope( original_id, _id );

	_id_mapping[ new_id ] = static_cast<int>( iterator - _variables.begin() );
}

double Constraint::error() const
{
	double value = required_error( _variables );
	if( std::isnan( value ) )
		throw nanException( _variables );
	return value;
}

double Constraint::delta_error( const std::vector<unsigned int>& variable_ids, const std::vector<int>& new_values ) const
{
	std::vector<unsigned int> converted_indexes( variable_ids.size() );
	std::transform( variable_ids.begin(), variable_ids.end(), converted_indexes.begin(), [&](unsigned int index){ return static_cast<unsigned int>( _id_mapping.at( index ) ); } );
	
	double value = expert_delta_error( converted_indexes, new_values );
	if( std::isnan( value ) )
	{
		auto changed_variables = _variables;
		for( int i = 0 ; i < static_cast<int>( new_values.size() ) ; ++i )
			changed_variables[ _id_mapping.at( variable_ids[ i ] ) ].set_value( new_values[ i ] );
		throw nanException( changed_variables );
	}
	return value;
}

double Constraint::simulate_delta( const std::vector<unsigned int>& variable_ids, const std::vector<int>& new_values )
{
	if( _is_expert_delta_error_defined ) [[likely]]
	{
		return delta_error( variable_ids, new_values );
	}
	else
	{
		std::vector<int> backup_values( new_values.size() );
		std::copy( new_values.begin(), new_values.end(), backup_values.begin() );
		
		for( int i = 0 ; i < static_cast<int>( new_values.size() ) ; ++i )
			_variables[ _id_mapping[ variable_ids[ i ] ] ].set_value( new_values[ i ] );
		
		auto error = this->error();

		for( int i = 0 ; i < static_cast<int>( new_values.size() ) ; ++i )
			_variables[ _id_mapping[ variable_ids[ i ] ] ].set_value( backup_values[ i ] );

		return error - current_error;
	}
}

bool Constraint::has_variable( unsigned int var_id ) const
{
	return std::find_if( _variables.cbegin(),
	                     _variables.cend(),
	                     [&]( auto& v ){ return v.get_id() == var_id; } ) != _variables.cend();
}  

std::vector<unsigned int> Constraint::get_variable_ids()
{
	std::vector<unsigned int> ids( _variables.size() );
	std::transform( _id_mapping.begin(), _id_mapping.end(), ids.begin(), [&](auto& pair){ return pair.first; } );
	return ids;	
}

double Constraint::expert_delta_error( const std::vector<unsigned int>& variable_indexes, const std::vector<int>& candidate_values ) const
{
	_is_expert_delta_error_defined = false;
	throw deltaErrorNotDefinedException();
}
