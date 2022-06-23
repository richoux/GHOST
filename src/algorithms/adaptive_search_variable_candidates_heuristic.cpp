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

#include "algorithms/adaptive_search_variable_candidates_heuristic.hpp"

using ghost::algorithms::AdaptiveSearchVariableCandidatesHeuristic;

AdaptiveSearchVariableCandidatesHeuristic::AdaptiveSearchVariableCandidatesHeuristic()
	: VariableCandidatesHeuristic( "Adaptive Search" )
{ }
		
std::vector<double> AdaptiveSearchVariableCandidatesHeuristic::compute_variable_candidates( const SearchUnitData& data ) const
{
	std::vector<double> worst_variables_list;
	double worst_variable_cost = -1;

	for( int variable_id = 0; variable_id < data.number_variables; ++variable_id )
		if( worst_variable_cost <= data.error_variables[ variable_id ]
		    && data.tabu_list[ variable_id ] <= data.local_moves
		    && ( !data.matrix_var_ctr.at( variable_id ).empty() || ( data.is_optimization && data.current_sat_error == 0 ) ) )
		{
			if( worst_variable_cost < data.error_variables[ variable_id ] )
			{
				worst_variables_list.clear();
				worst_variables_list.push_back( variable_id );
				worst_variable_cost = data.error_variables[ variable_id ];
			}
			else
				if( worst_variable_cost == data.error_variables[ variable_id ] )
					worst_variables_list.push_back( variable_id );
		}
		
	return worst_variables_list;
}
