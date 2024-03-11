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

#include <algorithm>
#include <numeric>
#include <functional>
#include <iostream>

#include "algorithms/culprit_search_error_projection_algorithm.hpp"

using ghost::algorithms::CulpritSearchErrorProjection;
using ghost::Variable;
using ghost::Constraint;

CulpritSearchErrorProjection::CulpritSearchErrorProjection()
	: ErrorProjection( "Culprit Search" )
{ }

void CulpritSearchErrorProjection::initialize_data_structures( const SearchUnitData& data )
{
	_error_variables_by_constraints = std::vector<std::vector<double>>( data.number_constraints, std::vector<double>( data.number_variables, 0. ) );
}

void CulpritSearchErrorProjection::compute_variable_errors_on_constraint( const std::vector<Variable>& variables,
	                                                                        const std::vector<std::vector<int>>& matrix_var_ctr,
	                                                                        std::shared_ptr<Constraint> constraint )
{
	auto& current_errors = _error_variables_by_constraints[ constraint->_id ];
	std::fill( current_errors.begin(), current_errors.end(), 0. );

	if( constraint->_current_error > 0 )
	{
		int previous_value;
		int next_value;
		
		for( const int variable_id : constraint->get_variable_ids() )
		{
			if( variables[ variable_id ].get_domain_size() > 2 )
			{
				auto range = variables[ variable_id ].get_partial_domain( 3 );
				previous_value = range[0];
				next_value = range[2];
				
				current_errors[ variable_id ] =
					constraint->simulate_delta( std::vector<int>{variable_id},
					                            std::vector<int>{previous_value} )
					+
					constraint->simulate_delta( std::vector<int>{variable_id},
					                            std::vector<int>{next_value} );				
			}
			else
			{
				if( variables[ variable_id ].get_domain_size() == 2 )
				{
					auto range = variables[ variable_id ].get_full_domain();
					range.erase( std::find( range.begin(), range.end(), variables[ variable_id ].get_value() ) );
					next_value = range[0];
				
					current_errors[ variable_id ] =	constraint->simulate_delta( std::vector<int>{variable_id}, std::vector<int>{next_value} );
				}
				else
				{
					current_errors[ variable_id ] =	constraint->simulate_delta( std::vector<int>{variable_id}, std::vector<int>{variables[ variable_id ].get_value()} );
				}				
			}
		}
		
		double max = *std::max_element( current_errors.cbegin(), current_errors.cend() );
		
		// max becomes 0, the lowest delta becomes the highest one.
		std::transform( current_errors.cbegin(),
		                current_errors.cend(),
		                current_errors.begin(),
		                [max](auto delta){ return -delta + max; } );
		
		double sum = std::accumulate( current_errors.cbegin(), current_errors.cend(), 0. );
		
		// normalize deltas such that their sum equals to 1.
		std::transform( current_errors.cbegin(),
		                current_errors.cend(),
		                current_errors.begin(),
		                [sum, &constraint](auto delta){ return delta == 0 ? 0 : ( delta / sum ) * constraint->_current_error; } );
	}
}

void CulpritSearchErrorProjection::compute_variable_errors( const std::vector<Variable>& variables,
                                                            const std::vector<std::shared_ptr<Constraint>>& constraints,
                                                            SearchUnitData& data )
{
	std::fill( data.error_variables.begin(), data.error_variables.end(), 0. );
	
	for( auto constraint : constraints )
	{
		compute_variable_errors_on_constraint( variables, data.matrix_var_ctr, constraint );
		
		// add normalize deltas of the current constraint to the error variables vector.
		std::transform( _error_variables_by_constraints[ constraint->_id ].cbegin(),
		                _error_variables_by_constraints[ constraint->_id ].cend(),
		                data.error_variables.cbegin(),
		                data.error_variables.begin(),
		                std::plus<>{} );
	}
}

void CulpritSearchErrorProjection::update_variable_errors( const std::vector<Variable>& variables,
                                                           std::shared_ptr<Constraint> constraint,
                                                           SearchUnitData& data,
                                                           double delta )
{
	// remove current deltas of the given constraint to the error variables vector.
	std::transform( _error_variables_by_constraints[ constraint->_id ].cbegin(),
	                _error_variables_by_constraints[ constraint->_id ].cend(),
	                data.error_variables.cbegin(),
	                data.error_variables.begin(),
	                std::minus<>{} );

	compute_variable_errors_on_constraint( variables, data.matrix_var_ctr, constraint );

	// add normalize deltas of the current constraint to the error variables vector.
	std::transform( _error_variables_by_constraints[ constraint->_id ].cbegin(),
	                _error_variables_by_constraints[ constraint->_id ].cend(),
	                data.error_variables.cbegin(),
	                data.error_variables.begin(),
	                std::plus<>{} );
}
