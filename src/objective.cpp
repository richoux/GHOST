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
 * Copyright (C) 2014-2024 Florian Richoux
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

using ghost::Objective;

Objective::Objective( const std::vector<int>& variables_index, bool is_maximization, const std::string& name )
	: _variables_index( variables_index ),
	  _is_optimization( true ),
	  _is_maximization( is_maximization ),
	  _name( name )
{ }

Objective::Objective( const std::vector<Variable>& variables, bool is_maximization, const std::string& name )
	: _variables_index( std::vector<int>( variables.size() ) ),
	  _is_optimization( true ),
	  _is_maximization( is_maximization ),
	  _name( name )
{
	std::transform( variables.begin(),
	                variables.end(),
	                _variables_index.begin(),
	                [&](const auto& v){ return v.get_id(); } );
}

double Objective::cost() const
{
	double value = required_cost( _variables );

	if( std::isnan( value ) )
		throw nanException( _variables );

	if( _is_maximization )
		value = -value;

	return value;
}

void Objective::conditional_update_data_structures( const std::vector<Variable*>& variables, int index, int new_value )
{ }

int Objective::expert_heuristic_value( const std::vector<Variable*>& variables,
                                       int variable_index,
                                       const std::vector<int>& possible_values,
                                       randutils::mt19937_rng& rng ) const
{
	// Should never happen
	if( possible_values.empty() ) [[unlikely]]
		return variables[ variable_index ]->get_value();

	double min_cost = std::numeric_limits<double>::max();
	double simulated_cost;

	auto var = variables[ variable_index ];
	int backup = var->get_value();
	std::vector<int> best_values;

	for( auto v : possible_values )
	{
		var->set_value( v );
		simulated_cost = required_cost( variables );

		if( _is_maximization )
			simulated_cost = -simulated_cost;

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

	var->set_value( backup );

	if( !best_values.empty() ) [[likely]]
		return rng.pick( best_values );
	else
		return rng.pick( possible_values );
}

int Objective::expert_heuristic_value_permutation( const std::vector<Variable*>& variables,
                                                   int variable_index,
                                                   const std::vector<int>& candidate_variables,
                                                   randutils::mt19937_rng& rng ) const
{
	// Should never happen
	if( candidate_variables.empty() ) [[unlikely]]
		return variable_index;
		
	double min_cost = std::numeric_limits<double>::max();
	double simulated_cost;

	auto var = variables[ variable_index ];
	int backup = var->get_value();
	int tmp;
	size_t head = 0;
	size_t tail = 0;
	std::vector<int> best_values;

	while( head < candidate_variables.size() )
	{
		tmp = var->get_value();		
		var->set_value( variables[ candidate_variables[ head ] ]->get_value() );
		variables[ candidate_variables[ head ] ]->set_value( backup );
		variables[ candidate_variables[ tail ] ]->set_value( tmp );
		
		simulated_cost = required_cost( variables );

		if( _is_maximization )
			simulated_cost = -simulated_cost;

		if( min_cost > simulated_cost )
		{
			min_cost = simulated_cost;
			best_values.clear();
			best_values.push_back( candidate_variables[ head ] );
		}
		else
			if( min_cost == simulated_cost )
				best_values.push_back( candidate_variables[ head ] );

		tail = head; // first iteration, tail remains at 0
		++head;
	}

	var->set_value( backup );

	if( !best_values.empty() ) [[likely]]
		return rng.pick( best_values );
	else
		return rng.pick( candidate_variables );
}

double Objective::expert_postprocess( const std::vector<Variable*>& variables,
                                      double best_cost ) const
{
	return best_cost;
}
