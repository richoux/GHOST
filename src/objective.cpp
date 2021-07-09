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

Objective::Objective( const std::vector<int>& variables_index, const std::string& name )
	: Objective( variables_index, false, name )
{ }

Objective::Objective( const std::vector<Variable>& variables, const std::string& name )
	: Objective( variables, false, name )
{ }

Objective::Objective( const std::vector<int>& variables_index, const char* name )
	: Objective( variables_index, false, std::string( name ) )
{ }

Objective::Objective( const std::vector<Variable>& variables, const char* name )
	: Objective( variables, false, std::string( name ) )
{ }

double Objective::cost() const
{
	double value = required_cost( _variables );
	
	if( std::isnan( value ) )
		throw nanException( _variables );

	if( _is_maximization )
		value *= -1;
		
	return value;
}

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
  
	return rng.pick( best_values );
}
  
int Objective::expert_heuristic_value_permutation( const std::vector<Variable*>& variables,
                                                   int variable_index,
                                                   const std::vector<int>& bad_variables ) const
{
	return rng.pick( bad_variables );
}
 
void Objective::expert_postprocess_satisfaction( const std::vector<Variable*>& variables,
                                                 double& bestCost,
                                                 std::vector<int>& solution ) const
{ }

void Objective::expert_postprocess_optimization( const std::vector<Variable*>& variables,
                                                 double& bestCost,
                                                 std::vector<int>& solution ) const
{ }
