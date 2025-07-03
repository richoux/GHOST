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

#include "algorithms/space_policy_regular.hpp"
#include "algorithms/space_of_violation.hpp"
#include "algorithms/error_projection_algorithm_adaptive_search.hpp"

using ghost::algorithms::Regular;

Regular::Regular( std::unique_ptr<algorithms::ErrorProjection> error_projection )
	: SpacePolicy( "Regular space policy",
	               std::move( error_projection ),
								 false )
{
	space_pool.emplace_back( std::make_unique<algorithms::SpaceOfViolation>() );
}

Regular::Regular()
	: Regular( std::make_unique<algorithms::ErrorProjectionAdaptiveSearch>() )
{ }

void Regular::update_errors( int variable_to_change,
                             int new_value,
                             SearchUnitData& data,
                             const Model& model ) const
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

			// TODO: extract this from update_errors. This is about maintaining the variable assignement within constraints.
			model.constraints[ constraint_id ]->update( variable_to_change, new_value );
		}

		// TODO: extract this from update_errors. This is about maintaining the variable assignement within the objective function.
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
						
			// TODO: extract this from update_errors. This is about maintaining the variable assignement within constraints.
			model.constraints[ constraint_id ]->update( variable_to_change, next_value );

			if( model.constraints[ constraint_id ]->has_variable( new_value ) )
				// TODO: extract this from update_errors. This is about maintaining the variable assignement within constraints.
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

		// TODO: extract this from update_errors. This is about maintaining the variable assignement within the objective function.
		if( data.is_optimization )
		{
			model.objective->update( variable_to_change, next_value );
			model.objective->update( new_value, current_value );
		}
	}	
}

// bool Regular::local_minimum_management( int variable_to_change,
//                                         SearchUnitData& data,
//                                         int tabu_time_local_min,
//                                         bool no_other_variables_to_try )
// {
// 	if( no_other_variables_to_try ) // || rng.uniform(1, 100) <= 10 //10% chance to force tabu-marking even if there are other variables to explore.
// 	{
// 		data.tabu_list[ variable_to_change ] = tabu_time_local_min + data.local_moves;
// 		++data.local_minimum;
// 	}
// #if defined GHOST_TRACE
// 	else
// 	{
// 		COUT << "Try other variables: not a local minimum yet.\n";
// 	}
// #endif
// 	return false;
// }

// bool Regular::plateau_management( int variable_to_change,
// 																	int new_value,
// 																	SearchUnitData& data,
// 																	const Options& options )
// {
// 	if( rng.uniform(1, 100) <= options.percent_chance_force_trying_on_plateau ) // no moves, try another variable
// 	{
// 		data.tabu_list[ variable_to_change ] = options.tabu_time_local_min + data.local_moves;
// 		must_compute_variable_candidates = true;
// 		++data.plateau_force_trying_another_variable;
// #if defined GHOST_TRACE
// 		COUT << "Force the exploration of another variable on a plateau; current variable marked as tabu.\n";
// #endif
// 	}
// 	else // We stay on the plateau with a local move
// 	{
// 		// min_conflict and delta_cost should be at 0 in a plateau situation
// 		// but just in case, we assigned them to 0 before calling local_move,
// 		// that will update the fitness
// 		data.min_conflict = 0;
// 		data.delta_cost = 0;
// 		data.increment_plateau_moves();
// 		local_move( variable_to_change, new_value );
// 	}

// 	return false
// }

// bool Regular::reset_management( SearchUnitData& data,
// 																const Options& options,
// 																const Model& model )
// {
// 	++data.resets;
// 	data.plateau_moves_in_a_row = 0;

// 	// if we reach the restart threshold, do a restart instead of a reset
// 	if( options.restart_threshold > 0 && ( data.resets % options.restart_threshold == 0 ) )
// 	{
// 		++data.restarts;

// 		// Start from a given starting configuration, or a random one.
// 		initialize_variable_values();

// #if defined GHOST_TRACE
// 		COUT << "Number of restarts performed so far: " << data.restarts << "\n";
// 		COUT << options.print->print_candidate( model.variables ).str();
// 		COUT << "\n";
// #endif
// 	}
// 	else // real reset
// 	{
// 		if( model.permutation_problem )
// 			random_permutations( options.number_variables_to_reset );
// 		else
// 			monte_carlo_sampling( options.number_variables_to_reset );

// 		model.auxiliary_data->update();
// #if defined GHOST_TRACE
// 		COUT << "Number of resets performed so far: " << data.resets << "\n";
// 		COUT << options.print->print_candidate( model.variables ).str();
// 		COUT << "\n";
// #endif
// 	}
			
// 	initialize_data_structures();

// 	return false;
// }
