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

#include <algorithm>
#include <iterator>
#include <limits>

#include "constraint.hpp"

using namespace ghost;

Constraint::Constraint( const std::vector<int>& variables_index )
	: _variables_index( variables_index ),
	  _current_error( std::numeric_limits<double>::max() ),
	  _id( 0 ),
	  _is_optional_delta_error_defined( true )
{ }

Constraint::Constraint( const std::vector<Variable>& variables )
	: _variables_index( std::vector<int>( variables.size() ) ),
	  _current_error( std::numeric_limits<double>::max() ),
	  _id( 0 ),
	  _is_optional_delta_error_defined( true )
{
	std::transform( variables.begin(),
	                variables.end(),
	                _variables_index.begin(),
	                [&](const auto& v){ return v.get_id(); } );
}

double Constraint::error() const
{
	double value = required_error( _variables );
	if( std::isnan( value ) )
		throw nanException( _variables );
	if( value < 0 )
		throw negativeException( _variables );
	return value;
}

double Constraint::delta_error( const std::vector<int>& variables_index, const std::vector<int>& new_values ) const
{
	std::vector<int> variables_index_within_constraint( variables_index.size() );
	std::transform( variables_index.begin(),
	                variables_index.end(),
	                variables_index_within_constraint.begin(),
	                [&]( auto index ){ return _variables_position.at( index ); } );

	double value = optional_delta_error( _variables, variables_index_within_constraint, new_values );
	if( std::isnan( value ) )
	{
		std::vector<Variable> changed_variables( _variables.size() );
		std::transform( _variables.begin(),
		                _variables.end(),
		                changed_variables.begin(),
		                [&]( auto& var ){ return *var; } );

		for( int i = 0 ; i < static_cast<int>( new_values.size() ) ; ++i )
			changed_variables[ variables_index_within_constraint[i] ].set_value( new_values[i] );
		throw nanException( changed_variables );
	}
	return value;
}

double Constraint::simulate_delta( const std::vector<int>& variables_index, const std::vector<int>& new_values )
{
	if( _is_optional_delta_error_defined ) [[likely]]
	{
		return delta_error( variables_index, new_values );
	}
	else
	{
		std::vector<int> backup_values( new_values.size() );

		for( int i = 0 ; i < static_cast<int>( new_values.size() ) ; ++i )
		{
			backup_values[ i ] = _variables[ _variables_position[ variables_index[i] ] ]->get_value();
			_variables[ _variables_position[ variables_index[i] ] ]->set_value( new_values[i] );
		}

		auto error = this->error();

		for( int i = 0 ; i < static_cast<int>( new_values.size() ) ; ++i )
			_variables[ _variables_position[ variables_index[i] ] ]->set_value( backup_values[i] );

		return error - _current_error;
	}
}

bool Constraint::has_variable( int var_id ) const
{
	return _variables_position.count( var_id ) > 0;
}

double Constraint::optional_delta_error( const std::vector<Variable*>& variables, const std::vector<int>& indexes, const std::vector<int>& candidate_values ) const
{
	_is_optional_delta_error_defined = false;
	throw deltaErrorNotDefinedException();
}

void Constraint::conditional_update_data_structures( const std::vector<Variable*>& variables, int index, int new_value ) { }
