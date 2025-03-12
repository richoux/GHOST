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

#include <cstdlib> // std::abs
#include "algorithms/space_policy_switch_optimization.hpp"
#include "algorithms/space_of_violation.hpp"
#include "algorithms/space_of_optimization.hpp"
#include "algorithms/error_projection_algorithm_adaptive_search.hpp"

using ghost::algorithms::SwitchOptimization;

SwitchOptimization::SwitchOptimization( std::unique_ptr<algorithms::ErrorProjection> error_projection )
	: SpacePolicy( "SwitchOptimization space policy",
	               std::move( error_projection ) )
{
	// space_pool[0] : violation space
	space_pool.emplace_back( std::make_unique<algorithms::SpaceOfViolation>() );
	// space_pool[1] : optimization space
	space_pool.emplace_back( std::make_unique<algorithms::SpaceOfOptimization>() );
}

SwitchOptimization::SwitchOptimization()
	: SwitchOptimization( std::make_unique<algorithms::ErrorProjectionAdaptiveSearch>() )
{ }

void SwitchOptimization::update_errors( int variable_to_change,
                                        int new_value,
                                        SearchUnitData& data,
                                        const Model& model ) const
{
	if( index_space_pool == 0 ) // if we are currently exploring the violation space
	{
		int delta_index = 0;
		if( !model.permutation_problem )
		{
			for( const int constraint_id : data.matrix_var_ctr.at( variable_to_change ) )
			{
				auto delta = data.delta_errors.at( new_value )[ delta_index++ ];
				model.constraints[ constraint_id ]->_current_error += delta;
					
				error_projection->update_variable_errors( model.variables,
				                                          model.constraints[ constraint_id ],
				                                          data,
				                                          delta );

				model.constraints[ constraint_id ]->update( variable_to_change, new_value );
			}

			if( data.is_optimization )
				model.objective->update( variable_to_change, new_value );
		}
		else
		{
			std::vector<bool> constraint_checked( data.number_constraints, false );
			int current_value = model.variables[ variable_to_change ].get_value();
			int next_value = model.variables[ new_value ].get_value();

			for( const int constraint_id : data.matrix_var_ctr.at( variable_to_change ) )
			{
				constraint_checked[ constraint_id ] = true;
				auto delta = data.delta_errors.at( new_value )[ delta_index++ ];
				model.constraints[ constraint_id ]->_current_error += delta;

				error_projection->update_variable_errors( model.variables,
				                                          model.constraints[ constraint_id ],
				                                          data,
				                                          delta );
						
				model.constraints[ constraint_id ]->update( variable_to_change, next_value );

				if( model.constraints[ constraint_id ]->has_variable( new_value ) )
					model.constraints[ constraint_id ]->update( new_value, current_value );
			}

			for( const int constraint_id : data.matrix_var_ctr.at( new_value ) )
				if( !constraint_checked[ constraint_id ] )
				{
					auto delta = data.delta_errors.at( new_value )[ delta_index++ ];
					model.constraints[ constraint_id ]->_current_error += delta;

					error_projection->update_variable_errors( model.variables,
					                                          model.constraints[ constraint_id ],
					                                          data,
					                                          delta );
						
					model.constraints[ constraint_id ]->update( new_value, current_value );
				}

			if( data.is_optimization )
			{
				model.objective->update( variable_to_change, next_value );
				model.objective->update( new_value, current_value );
			}
		}	
	}
}

bool SwitchOptimization::local_minimum_management( int variable_to_change,
                                                   SearchUnitData& data,
                                                   int tabu_time_local_min,
                                                   bool no_other_variables_to_try )
{
	if( no_other_variables_to_try )
	{
		// data.tabu_list[ variable_to_change ] = options.tabu_time_local_min + data.local_moves;
		// must_compute_variable_candidates = true;
		++data.local_minimum;
		index_space_pool = std::abs( index_space_pool - 1 );
		return true;
	}
	else
	{
#if defined GHOST_TRACE
		COUT << "Try other variables: not a local minimum yet.\n";
		// must_compute_variable_candidates = false;
#endif
		return false;
	}
}
