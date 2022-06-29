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

#include "algorithms/adaptive_search_error_projection_heuristic.hpp"

using ghost::algorithms::AdaptiveSearchErrorProjection;
using ghost::Variable;
using ghost::Constraint;

AdaptiveSearchErrorProjection::AdaptiveSearchErrorProjection()
	: ErrorProjection( "Adaptive Search" )
{ }
		
void AdaptiveSearchErrorProjection::compute_variable_errors( std::vector<double>& error_variables,
                                                             const std::vector<Variable>& variables,
                                                             const std::vector<std::vector<int>>& matrix_var_ctr,
                                                             const std::vector<std::shared_ptr<Constraint>>& constraints ) const
{
	for( int variable_id = 0; variable_id < static_cast<int>( variables.size() ); ++variable_id )
		for( int constraint_id : matrix_var_ctr.at( variable_id ) )
			error_variables[ variable_id ] += constraints[ constraint_id ]->_current_error;	
}

void AdaptiveSearchErrorProjection::update_variable_errors( std::vector<double>& error_variables,
                                                            std::shared_ptr<Constraint> constraint,
                                                            double delta ) const
{
	for( const int variable_id : constraint->get_variable_ids() )
		error_variables[ variable_id ] += delta;
}
