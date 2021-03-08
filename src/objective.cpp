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

#include "objective.hpp"

using namespace ghost;

Objective::Objective( std::string name, const std::vector<Variable>& variables )
	: _name( name ),
	  _variables( variables )
{ }

void Objective::update_variable( unsigned int variable_id, int new_value )
{
	_variables[ _id_mapping[ variable_id ] ].set_value( new_value );
	update_objective( _variables, _id_mapping[ variable_id ], new_value );
}

void Objective::update_objective( const std::vector<Variable>& variables, unsigned int variable_id, int new_value ) { }

void Objective::make_variable_id_mapping( unsigned int new_id, unsigned int original_id )
{
	auto iterator = std::find_if( _variables.begin(), _variables.end(), [&](auto& v){ return v.get_id() == original_id; } );
	if( iterator == _variables.end() )
		throw variableOutOfTheScope( original_id, _name );

	_id_mapping[ new_id ] = static_cast<int>( iterator - _variables.begin() );
}

double Objective::cost() const
{
	double value = required_cost( _variables );
	if( std::isnan( value ) )
		throw nanException( _variables );
	return value;
}

// double Objective::simulate_cost( const std::vector<unsigned int>& variable_ids, const std::vector<int>& new_values )
// {			
// 	std::vector<int> backup_values( new_values.size() );
// 		std::copy( new_values.begin(), new_values.end(), backup_values.begin() );
		
// 	for( int i = 0 ; i < static_cast<int>( variable_ids.size() ) ; ++i )
// 		_variables[ _id_mapping[ variable_ids[ i ] ] ].set_value( new_values[ i ] );
	
// 	auto cost = this->cost();
	
// 	for( int i = 0 ; i < static_cast<int>( variable_ids.size() ) ; ++i )
// 		_variables[ _id_mapping[ variable_ids[ i ] ] ].set_value( backup_values[ i ] );
	
// 	return cost;
// }

int Objective::expert_heuristic_value( std::vector<Variable> variables,
                                       int variable_index,
                                       const std::vector<int>& possible_values ) const
{
	double min_cost = std::numeric_limits<double>::max();
	double simulated_cost;

	auto& var = variables[ variable_index ];
	int backup = var.get_value();
	std::vector<int> best_values;
  
	for( auto& v : possible_values )
	{
		var.set_value( v );
		simulated_cost = required_cost( variables );
    
		if( min_cost > simulated_cost )
		{
			min_cost = simulated_cost;
			best_values.clear();
			best_values.push_back( v );
		}
		else
			if( min_cost == simulated_cost )
				best_values.push_back( v );
	}
  
	var.set_value( backup );
  
	return rng.pick( best_values );
}
  
unsigned int Objective::expert_heuristic_value_permutation( std::vector<Variable> variables,
                                                            int variable_index,
                                                            const std::vector<int>& bad_variables ) const
{
	return rng.pick( bad_variables );
}
 
void Objective::expert_postprocess_satisfaction( const std::vector<Variable>& variables,
                                                 double& bestCost,
                                                 std::vector<int>& solution ) const
{ }

void Objective::expert_postprocess_optimization( const std::vector<Variable>& variables,
                                                 double& bestCost,
                                                 std::vector<int>& solution ) const
{ }

