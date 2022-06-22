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

#include "objective.hpp"

using namespace ghost;

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
		value *= -1;

	return value;
}

void Objective::conditional_update_data_structures( const std::vector<Variable*>& variables, int index, int new_value )
{ }

int Objective::expert_heuristic_value( const std::vector<Variable*>& variables,
                                       int variable_index,
                                       const std::vector<int>& possible_values ) const
{
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
			simulated_cost *= -1;

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

	if( !best_values.empty() )
		return rng.pick( best_values );
	else
		if( !possible_values.empty() )
			return rng.pick( possible_values );
		else
			return backup;			
}

int Objective::expert_heuristic_value_permutation( const std::vector<Variable*>& variables,
                                                   int variable_index,
                                                   const std::vector<int>& bad_variables ) const
{
	return rng.pick( bad_variables );
}

double Objective::expert_postprocess( const std::vector<Variable*>& variables,
                                      double best_cost ) const
{
	return best_cost;
}
