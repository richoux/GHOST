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

#include <algorithm>
#include <iterator>
#include <limits>

#include "constraint.hpp"

using namespace ghost;

unsigned int Constraint::NBER_CTR = 0;

Constraint::Constraint( const std::vector<Variable>& variables )
	: _current_error( std::numeric_limits<double>::max() ),
	  _is_expert_delta_error_defined( true )
{
	if( NBER_CTR < std::numeric_limits<unsigned int>::max() )
		_id = NBER_CTR++;
	else
		_id = NBER_CTR = 0;

	std::transform( variables.begin(),
	                variables.end(),
	                std::back_inserter( _ptr_variables ),
	                [&](const Variable &v){ return &v; } );
}

// void Constraint::update_variable( unsigned int variable_index, int new_value )
// {
// 	_ptr_variables[ _id_mapping[ variable_index ] ].set_value( new_value );
// }

void Constraint::make_variable_id_mapping( unsigned int new_id, unsigned int original_id )
{
	auto iterator = std::find_if( _ptr_variables.begin(), _ptr_variables.end(), [&](auto& v){ return v.get_id() == original_id; } );
	if( iterator == _ptr_variables.end() )
		throw variableOutOfTheScope( original_id, _id );

	_id_mapping[ new_id ] = static_cast<int>( iterator - _ptr_variables.begin() );
}

double Constraint::error() const
{
	double value = required_error( _ptr_variables );
	if( std::isnan( value ) )
		throw nanException( _ptr_variables );
	return value;
}

double Constraint::delta_error( const std::vector<unsigned int>& variables_index, const std::vector<int>& new_values ) const
{
	std::vector<unsigned int> converted_indexes( variables_index.size() );
	std::transform( variables_index.begin(), variables_index.end(), converted_indexes.begin(), [&](unsigned int index){ return static_cast<unsigned int>( _id_mapping.at( index ) ); } );
	
	double value = expert_delta_error( _ptr_variables, converted_indexes, new_values );
	if( std::isnan( value ) )
	{
		auto changed_ptr_variables = _ptr_variables;
		for( int i = 0 ; i < static_cast<int>( new_values.size() ) ; ++i )
			changed_ptr_variables[ _id_mapping.at( variables_index[ i ] ) ]->set_value( new_values[ i ] );
		throw nanException( changed_ptr_variables );
	}
	return value;
}

double Constraint::simulate_delta( const std::vector<unsigned int>& variables_index, const std::vector<int>& new_values )
{
	if( _is_expert_delta_error_defined ) [[likely]]
	{
		return delta_error( variables_index, new_values );
	}
	else
	{
		std::vector<int> backup_values( new_values.size() );
		std::transform( variables_index.begin(), variables_index.end(), backup_values.begin(), [&](auto var_index){ return _ptr_variables[ _id_mapping[var_index] ]->get_value(); } );
		
		for( int i = 0 ; i < static_cast<int>( new_values.size() ) ; ++i )
			_ptr_variables[ _id_mapping[ variables_index[ i ] ] ]->set_value( new_values[ i ] );
		
		auto error = this->error();

		for( int i = 0 ; i < static_cast<int>( new_values.size() ) ; ++i )
			_ptr_variables[ _id_mapping[ variables_index[ i ] ] ]->set_value( backup_values[ i ] );

		return error - _current_error;
	}
}

bool Constraint::has_variable_unshifted( unsigned int var_id ) const
{
	return std::find_if( _ptr_variables.cbegin(),
	                     _ptr_variables.cend(),
	                     [&]( auto& v ){ return v->get_id() == var_id; } ) != _ptr_variables.cend();
}  

std::vector<unsigned int> Constraint::get_variable_ids() const
{
	std::vector<unsigned int> ids( _ptr_variables.size() );
	std::transform( _id_mapping.begin(), _id_mapping.end(), ids.begin(), [&](auto& pair){ return pair.first; } );
	return ids;	
}

double Constraint::expert_delta_error( const std::vector<Variable*>& variables, const std::vector<unsigned int>& variable_indexes, const std::vector<int>& candidate_values ) const
{
	_is_expert_delta_error_defined = false;
	throw deltaErrorNotDefinedException();
}
