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
#include <numeric>

#include "algorithms/culprit_search_error_projection_heuristic.hpp"

using ghost::algorithms::CulpritSearchErrorProjection;
using ghost::Variable;
using ghost::Constraint;

CulpritSearchErrorProjection::CulpritSearchErrorProjection()
	: ErrorProjection( "Culprit Search" )
{ }
		
void CulpritSearchErrorProjection::compute_variable_errors( std::vector<double>& error_variables,
                                                            const std::vector<Variable>& variables,
                                                            const std::vector<std::vector<int>>& matrix_var_ctr,
                                                            const std::vector<std::shared_ptr<Constraint>>& constraints ) const
{
	int previous_value;
	int next_value;
	std::vector<double> deltas( variables.size() );
	
	std::fill( error_variables.begin(), error_variables.end(), 0. );
	
	for( auto constraint : constraints )
		if( constraint->_current_error > 0 )
		{
			std::fill( deltas.begin(), deltas.end(), 0. );

			for( const int variable_id : constraint->get_variable_ids() )
			{
				auto range = variables[ variable_id ].get_partial_domain( 3 );
				previous_value = range[0];
				next_value = range[2];
				
				deltas[ variable_id ] =
					constraint->simulate_delta( std::vector<int>{variable_id},
					                            std::vector<int>{previous_value} )
					+
					constraint->simulate_delta( std::vector<int>{variable_id},
					                            std::vector<int>{next_value} );
			}
			
			double max = std::max_element( deltas.begin(), deltas.end() );

			// max becomes 0, the lowest delta becomes the highest one.
			std::transform( deltas.begin(), deltas.end(), [max](auto delta){ return delta == 0 ? 0 : -delta + max; } );
			double sum = std::accumulate( deltas.begin(), deltas.end(), 0. );

			// normalize deltas such that their sum equals to 1.
			std::transform( deltas.begin(), deltas.end(), [sum](auto delta){ return ( delta / sum ) * constraint->_current_error; } );

			// add normalize deltas of the current constraint to the error variables vector.
			std::transform( deltas.begin(), deltas.end(), error_variables.begin(), error_variables.end(), std::plus<>{} );
		}
}

void CulpritSearchErrorProjection::update_variable_errors( std::vector<double>& error_variables,
                                                           std::shared_ptr<Constraint> constraint,
                                                           double delta ) const
{

}
