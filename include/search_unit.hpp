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

#pragma once

#include <iostream>
#include <iomanip>
#include <limits>
#include <random>
#include <algorithm>
#include <vector>
#include <chrono>
#include <memory>
#include <iterator>
#include <thread>
#include <future>
#include <numeric>

#include "variable.hpp"
#include "constraint.hpp"
#include "objective.hpp"
#include "auxiliary_data.hpp"
#include "search_unit_data.hpp"
#include "model.hpp"
#include "options.hpp"
#include "thirdparty/randutils.hpp"

#include "algorithms/variable_heuristic.hpp"
#include "algorithms/variable_candidates_heuristic.hpp"
#include "algorithms/value_heuristic.hpp"
#include "algorithms/error_projection_algorithm.hpp"

#include "algorithms/uniform_variable_heuristic.hpp"
#include "algorithms/adaptive_search_variable_candidates_heuristic.hpp"
#include "algorithms/adaptive_search_value_heuristic.hpp"
#include "algorithms/adaptive_search_error_projection_algorithm.hpp"

#include "algorithms/antidote_search_variable_heuristic.hpp"
#include "algorithms/antidote_search_variable_candidates_heuristic.hpp"
#include "algorithms/antidote_search_value_heuristic.hpp"

#include "algorithms/culprit_search_error_projection_algorithm.hpp"

#if defined GHOST_RANDOM_WALK || defined GHOST_HILL_CLIMBING
#include "algorithms/all_free_variable_candidates_heuristic.hpp"
#include "algorithms/null_error_projection_algorithm.hpp"
#endif

#if defined GHOST_RANDOM_WALK 
#include "algorithms/random_walk_value_heuristic.hpp"
#endif

#include "macros.hpp"

namespace ghost
{
	/*
	 * SearchUnit is the object called by Solver::fast_search to actually search for a solution.
	 * In parallel computing, one SearchUnit object is instanciated for every thread.
	 */
	class SearchUnit
	{
		std::promise<void> _stop_search_signal;
		std::future<void> _stop_search_check;
		std::thread::id _thread_id;

#if defined GHOST_TRACE_PARALLEL
		std::stringstream _log_filename;
		std::ofstream _log_trace;
#endif

#if defined GHOST_TRACE
		void print_current_candidate()
		{
			for( int variable_id = 0 ; variable_id < data.number_variables ; ++variable_id )
				COUT << model.variables[variable_id].get_value() << " ";			
		}
		
		void print_errors()
		{
			COUT << "Constraint errors:\n";
			for( int constraint_id = 0; constraint_id < data.number_constraints; ++constraint_id )
			{
				COUT << "Constraint num. " << constraint_id << "=" << model.constraints[ constraint_id ]->_current_error << ": ";
				bool mark_comma = false;

				for( const auto variable_id : model.constraints[ constraint_id ]->get_variable_ids() )
				{
					if( mark_comma )
						COUT << ", ";
					else
						mark_comma = true;
					COUT << "v[" << variable_id << "]=" << model.variables[variable_id].get_value();
				}

				COUT << "\n";
			}

			COUT << "\nVariable errors:\n";

			for( int variable_id = 0 ; variable_id < data.number_variables ; ++variable_id )
			{
				COUT << "v[" << variable_id << "]=" << data.error_variables[variable_id] << ": ";
				bool mark_plus = false;
				for( int constraint_id : data.matrix_var_ctr.at( variable_id ) )
				{
					if( mark_plus )
						COUT << " + ";
					else
						mark_plus = true;

					COUT << "c[" << constraint_id << "]=" << model.constraints[ constraint_id ]->_current_error;
				}
				COUT << "\n";
			}

			COUT << "\n";
		}
#endif

		// Set the initial configuration by calling monte_carlo_sampling() 'samplings' times.
		/*
		 * After calling calling monte_carlo_sampling() 'samplings' times, the function keeps 
		 * the configuration with the lowest satisfaction cost. If some of them reach 0, it keeps 
		 * the configuration with the best optimization cost.
		 */
		void set_initial_configuration( int samplings )
		{
			double best_sat_error_so_far = std::numeric_limits<double>::max();
			double current_sat_error;
			std::vector<int> best_values( data.number_variables, 0 );

			// To avoid weird samplings numbers like 0 or -1
			samplings = std::max( 1, samplings );
			int loops = 0;

			do
			{
				if( model.permutation_problem )
					random_permutations();
				else
					monte_carlo_sampling();

				model.auxiliary_data->update();
				current_sat_error = 0.0;

				for( int constraint_id = 0 ; constraint_id < data.number_constraints ; ++constraint_id )
					current_sat_error += model.constraints[ constraint_id ]->error();

				if( best_sat_error_so_far > current_sat_error )
				{
					best_sat_error_so_far = current_sat_error;

					if( data.best_sat_error > best_sat_error_so_far )
					{
#if defined GHOST_TRACE
						COUT << "Better starting configuration found: ";
						print_current_candidate();
						COUT << "\nPrevious error: " << data.best_sat_error << ", now: " << best_sat_error_so_far << "\n\n";
#endif
						data.best_sat_error = best_sat_error_so_far;
					}
					for( int i = 0 ; i < data.number_variables ; ++i )
						best_values[ i ] = model.variables[ i ].get_value();
				}
			} while( ++loops < samplings && current_sat_error > 0.0 );

			for( int variable_id = 0 ; variable_id < data.number_variables ; ++variable_id )
				model.variables[ variable_id ].set_value( best_values[ variable_id ] );

			model.auxiliary_data->update();
		}

		// Sample an configuration
		void monte_carlo_sampling( int nb_var = -1 )
		{
			if( nb_var == -1 )
				nb_var = data.number_variables;

			std::vector<int> variables_index( data.number_variables );
			std::iota( variables_index.begin(), variables_index.end(), 0 );
			rng.shuffle( variables_index );

			for( int i = 0 ; i < nb_var ; ++i )
				model.variables[ variables_index[ i ] ].pick_random_value( rng );
		}

		// Sample an configuration for permutation problems
		void random_permutations( int nb_var = -1 )
		{
			if( nb_var == -1 )
			{
				for( int i = 0 ; i < data.number_variables - 1 ; ++i )
					for( int j = i + 1 ; j < data.number_variables ; ++j )
					{
						// 50% to do a swap for each couple (var_i, var_j)
						if( rng.uniform( 0, 1 ) == 0
						    && i != j
						    && model.variables[ i ].get_value() != model.variables[ j ].get_value()
						    && std::find( model.variables[ j ].get_full_domain().begin(),
						                  model.variables[ j ].get_full_domain().end(),
						                  model.variables[ i ].get_value() ) != model.variables[ j ].get_full_domain().end()
						    && std::find( model.variables[ i ].get_full_domain().begin(),
						                  model.variables[ i ].get_full_domain().end(),
						                  model.variables[ j ].get_value() ) != model.variables[ i ].get_full_domain().end() )
						{
							std::swap( model.variables[i]._current_value, model.variables[j]._current_value );
						}
					}
			}
			else
			{
				std::vector<int> variables_index_A( data.number_variables );
				std::vector<int> variables_index_B( data.number_variables );
				std::iota( variables_index_A.begin(), variables_index_A.end(), 0 );
				std::iota( variables_index_B.begin(), variables_index_B.end(), 0 );
				rng.shuffle( variables_index_A );
				rng.shuffle( variables_index_B );

				for( int i = 0 ; i < nb_var ; ++i )
					if( variables_index_A[i] != variables_index_B[i]
					    && model.variables[ variables_index_A[i] ].get_value() != model.variables[ variables_index_B[i] ].get_value()
					    && std::find( model.variables[ variables_index_B[i] ].get_full_domain().begin(),
					                  model.variables[ variables_index_B[i] ].get_full_domain().end(),
					                  model.variables[ variables_index_A[i] ].get_value() ) != model.variables[ variables_index_B[i] ].get_full_domain().end()
					    && std::find( model.variables[ variables_index_A[i] ].get_full_domain().begin(),
					                  model.variables[ variables_index_A[i] ].get_full_domain().end(),
					                  model.variables[ variables_index_B[i] ].get_value() ) != model.variables[ variables_index_A[i] ].get_full_domain().end() )
						std::swap( model.variables[ variables_index_A[i] ]._current_value, model.variables[ variables_index_B[i] ]._current_value );
			}
		}

		void initialize_variable_values()
		{
			if( options.custom_starting_point || options.resume_search )
			{
				if( options.resume_search )
					options.resume_search = false;
				for( int i = 0 ; i < data.number_variables ; ++i )
					model.variables[i].set_value( variables_at_start[i].get_value() );

				model.auxiliary_data->update();
			}
			else
				set_initial_configuration( options.number_start_samplings );
		}

		void initialize_data_structures()
		{
			must_compute_variable_candidates = true;
			std::fill( data.tabu_list.begin(), data.tabu_list.end(), 0 );

			// Reset constraints costs
			for( int constraint_id = 0; constraint_id < data.number_constraints; ++constraint_id )
				model.constraints[ constraint_id ]->_current_error = 0.0;

			// (Re)compute constraint error and get the total current satisfaction error
			data.current_sat_error = compute_constraints_errors();
			if( data.best_sat_error > data.current_sat_error )
			{
				data.best_sat_error = data.current_sat_error;
				std::transform( model.variables.begin(),
				                model.variables.end(),
				                final_solution.begin(),
				                [&](auto& var){ return var.get_value(); } );
			}

			// (Re)compute the current optimization cost
			if( data.is_optimization )
			{
				if( data.current_sat_error == 0 ) [[unlikely]]
				{
					data.current_opt_cost = model.objective->cost();
					if( data.best_opt_cost > data.current_opt_cost )
					{
						data.best_opt_cost = data.current_opt_cost;
						std::transform( model.variables.begin(),
						                model.variables.end(),
						                final_solution.begin(),
						                [&](auto& var){ return var.get_value(); } );
					}
				}
				else
					data.current_opt_cost = std::numeric_limits<double>::max();
			}

			// Reset variable costs and recompute them
			error_projection_algorithm->compute_variable_errors( model.variables,
			                                                     model.constraints,
			                                                     data );
		}

		void initialize_data_structures( Model& model )
		{
			// Determine if optional_delta_error has been user defined or not for each constraint
			for( int constraint_id = 0; constraint_id < data.number_constraints; ++constraint_id )
				try
				{
					model.constraints[ constraint_id ]->optional_delta_error( model.constraints[ constraint_id ]->_variables, std::vector<int>{0}, std::vector<int>{model.variables[0].get_value()} );
				}
				catch( const std::exception& e )
				{
					std::cerr << "No optional_delta_error method defined for constraint num. " << constraint_id << "\n";
				}
		}

		void reset()
		{
			++data.resets;

			// if we reach the restart threshold, do a restart instead of a reset
			if( options.restart_threshold > 0 && ( data.resets % options.restart_threshold == 0 ) )
			{
				++data.restarts;

				// Start from a given starting configuration, or a random one.
				initialize_variable_values();

#if defined GHOST_TRACE
				COUT << "Number of restarts performed so far: " << data.restarts << "\n";
				COUT << options.print->print_candidate( model.variables ).str();
				COUT << "\n";
#endif
			}
			else // real reset
			{
				if( model.permutation_problem )
					random_permutations( options.number_variables_to_reset );
				else
					monte_carlo_sampling( options.number_variables_to_reset );

				model.auxiliary_data->update();
#if defined GHOST_TRACE
				COUT << "Number of resets performed so far: " << data.resets << "\n";
				COUT << options.print->print_candidate( model.variables ).str();
				COUT << "\n";
#endif
			}
			
			initialize_data_structures();
		}

#if defined GHOST_FITNESS_CLOUD
		void neighborhood_errors()
		{
			double error;

			COUT << "FITNESS_CLOUD Candidate: ";
			for( int variable_id = 0 ; variable_id < data.number_variables ; ++variable_id )
				COUT << model.variables[ variable_id ].get_value() << " ";
			COUT << "\nFITNESS_CLOUD Errors: " << data.current_sat_error << " ";

			if( model.permutation_problem )
			{
				for( int variable_id = 0 ; variable_id < data.number_variables - 1 ; ++variable_id )
					for( int variable_swap = variable_id + 1 ; variable_swap < data.number_variables ; ++variable_swap )
						if( model.variables[ variable_id ].get_value() != model.variables[ variable_swap ].get_value()
						    && std::find( model.variables[ variable_id ].get_full_domain().begin(),
						                  model.variables[ variable_id ].get_full_domain().end(),
						                  model.variables[ variable_swap ].get_value() ) != model.variables[ variable_id ].get_full_domain().end()
						    && std::find( model.variables[ variable_swap ].get_full_domain().begin(),
						                  model.variables[ variable_swap ].get_full_domain().end(),
						                  model.variables[ variable_id ].get_value() ) != model.variables[ variable_swap ].get_full_domain().end() )
						{
							error = data.current_sat_error;
							std::vector<bool> constraint_checked( data.number_constraints, false );
							int current_value = model.variables[ variable_id ].get_value();
							int candidate_value = model.variables[ variable_swap ].get_value();

							for( const int constraint_id : data.matrix_var_ctr.at( variable_id ) )
							{
								constraint_checked[ constraint_id ] = true;

								// check if the other variable also belongs to the constraint scope
								if( model.constraints[ constraint_id ]->has_variable( variable_swap ) )
									error += model.constraints[ constraint_id ]->simulate_delta( std::vector<int>{variable_id, variable_swap}, std::vector<int>{candidate_value, current_value} );
								else
									error += model.constraints[ constraint_id ]->simulate_delta( std::vector<int>{variable_id}, std::vector<int>{candidate_value} );
							}

							// Since we are switching the value of two variables, we need to also look at the delta error impact of changing the value of the non-selected variable
							for( const int constraint_id : data.matrix_var_ctr.at( variable_swap ) )
								// No need to look at constraint where variable_to_change also appears.
								if( !constraint_checked[ constraint_id ] )
									error += model.constraints[ constraint_id ]->simulate_delta( std::vector<int>{variable_swap}, std::vector<int>{current_value} );

							COUT << error << " ";
						}					
			}
			else
			{			
				for( int variable_id = 0 ; variable_id < data.number_variables ; ++variable_id )
					for( int value : model.variables[ variable_id ]._domain )
						if( value != model.variables[ variable_id ].get_value() )
						{						
							error = data.current_sat_error;
							for( const int constraint_id : data.matrix_var_ctr.at( variable_id ) )
								error += model.constraints[ constraint_id ]->simulate_delta( std::vector<int>{variable_id}, std::vector<int>{value} );
							COUT << error << " ";						
						}
			}
			
			COUT << "\n";
		}
#endif
		
		// Compute the cost of each constraints
		double compute_constraints_errors()
		{
			double satisfaction_error = 0.0;
			double error;

			for( int constraint_id = 0 ; constraint_id < data.number_constraints ; ++constraint_id )
			{
				error = model.constraints[ constraint_id ]->error();
				model.constraints[ constraint_id ]->_current_error = error;
				satisfaction_error += error;
			}

			return satisfaction_error;
		}

		void update_errors( int variable_to_change, int new_value, const std::map< int, std::vector<double>>& delta_errors )
		{
			int delta_index = 0;
			if( !model.permutation_problem )
			{
				for( const int constraint_id : data.matrix_var_ctr.at( variable_to_change ) )
				{
					auto delta = delta_errors.at( new_value )[ delta_index++ ];
					model.constraints[ constraint_id ]->_current_error += delta;
					
					error_projection_algorithm->update_variable_errors( model.variables,
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
					auto delta = delta_errors.at( new_value )[ delta_index++ ];
					model.constraints[ constraint_id ]->_current_error += delta;

					error_projection_algorithm->update_variable_errors( model.variables,
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
						auto delta = delta_errors.at( new_value )[ delta_index++ ];
						model.constraints[ constraint_id ]->_current_error += delta;

						error_projection_algorithm->update_variable_errors( model.variables,
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

		// A. Local move (perform local move and update variables/constraints/objective function)
		void local_move( int variable_to_change, int new_value, double min_conflict, const std::map< int, std::vector<double>>& delta_errors )
		{
			++data.local_moves;
			data.current_sat_error += min_conflict;
			data.tabu_list[ variable_to_change ] = options.tabu_time_selected + data.local_moves;
			must_compute_variable_candidates = true;

			update_errors( variable_to_change, new_value, delta_errors );

			if( model.permutation_problem )
			{
				int current_value = model.variables[ variable_to_change ].get_value();
				int next_value = model.variables[ new_value ].get_value();

				model.variables[ variable_to_change ].set_value( next_value );
				model.variables[ new_value ].set_value( current_value );

				model.auxiliary_data->update( variable_to_change, next_value );
				model.auxiliary_data->update( new_value, current_value );
			}
			else
			{
				model.variables[ variable_to_change ].set_value( new_value );
				model.auxiliary_data->update( variable_to_change, new_value );
			}
		}

		// B. Plateau management (local move on the plateau, but options.percent_chance_escape_plateau
		//                        of chance to escape it and mark the variable as tabu.)
		void plateau_management( int variable_to_change, int new_value, const std::map< int, std::vector<double>>& delta_errors )
		{
			if( rng.uniform(1, 100) <= options.percent_chance_escape_plateau )
			{
				data.tabu_list[ variable_to_change ] = options.tabu_time_local_min + data.local_moves;
				must_compute_variable_candidates = true;
				++data.plateau_local_minimum;
#if defined GHOST_TRACE
				COUT << "Escape from plateau; variables marked as tabu.\n";
#endif
			}
			else
			{
				local_move( variable_to_change, new_value, 0, delta_errors );
				++data.plateau_moves;
			}
		}

		// C. local minimum management (if there are no other worst variables to try, mark the variable as tabu.
		//                              Otherwise try them first.)
		void local_minimum_management( int variable_to_change, int new_value, bool no_other_variables_to_try )
		{
			must_compute_variable_candidates = false;

			if( no_other_variables_to_try ) // || rng.uniform(1, 100) <= 10 //10% chance to force tabu-marking even if there are other variables to explore.
			{
				data.tabu_list[ variable_to_change ] = options.tabu_time_local_min + data.local_moves;
				// must_compute_variable_candidates = true;
				++data.local_minimum;
			}
			else
			{
#if defined GHOST_TRACE
				COUT << "Try other variables: not a local minimum yet.\n";
#endif
				// must_compute_variable_candidates = false;
			}
		}

	public:
		Model model;

		std::vector<Variable> variables_at_start;
		randutils::mt19937_rng rng;

		SearchUnitData data;
		std::unique_ptr<algorithms::VariableHeuristic> variable_heuristic;
		std::unique_ptr<algorithms::VariableCandidatesHeuristic> variable_candidates_heuristic;
		std::unique_ptr<algorithms::ValueHeuristic> value_heuristic;
		std::unique_ptr<algorithms::ErrorProjection> error_projection_algorithm;
				
		std::vector<int> final_solution;

		std::vector<double> variable_candidates;
		bool must_compute_variable_candidates;

		std::promise<bool> solution_found;

		Options options;

		SearchUnit( Model&& moved_model,
		            const Options& options,
		            std::unique_ptr<algorithms::VariableHeuristic> variable_heuristic,
		            std::unique_ptr<algorithms::VariableCandidatesHeuristic> variable_candidates_heuristic,
		            std::unique_ptr<algorithms::ValueHeuristic> value_heuristic,
		            std::unique_ptr<algorithms::ErrorProjection> error_projection_algorithm )
			: _stop_search_check( _stop_search_signal.get_future() ),
			  model( std::move( moved_model ) ),
			  data( model ),
			  variable_heuristic( std::move( variable_heuristic ) ),
			  variable_candidates_heuristic( std::move( variable_candidates_heuristic ) ),
			  value_heuristic( std::move( value_heuristic ) ),
			  error_projection_algorithm( std::move( error_projection_algorithm ) ),
			  final_solution( std::vector<int>( data.number_variables, 0 ) ),
			  variable_candidates(), 
			  must_compute_variable_candidates ( true ),
			  options ( options )
		{
			std::transform( model.variables.begin(),
			                model.variables.end(),
			                std::back_inserter( variables_at_start ),
			                [&]( auto& v){ return v; } );

			initialize_data_structures( model );
			data.initialize_matrix( model );
			this->error_projection_algorithm->initialize_data_structures( data );

#if defined GHOST_TRACE
			COUT << "Creating a Solver object\n\n"
			     << "Variables:\n";

			for( const auto& variable : model.variables )
				COUT << variable << "\n";

			COUT << "\nConstraints:\n";
			for( auto constraint : model.constraints )
				COUT << *constraint << "\n";

			COUT << "\nObjective function:\n"
			     << *(model.objective) << "\n";
#endif
		}

		SearchUnit( Model&& moved_model, const Options& options )
			: SearchUnit( std::move( moved_model ),
			              options,
			              std::make_unique<algorithms::UniformVariableHeuristic>(),
			              std::make_unique<algorithms::AdaptiveSearchVariableCandidatesHeuristic>(),
			              std::make_unique<algorithms::AdaptiveSearchValueHeuristic>(),
			              std::make_unique<algorithms::AdaptiveSearchErrorProjection>() )
		{ }
		
		// Check if the thread must stop search
		bool stop_search_requested()
		{
			if( _stop_search_check.wait_for( std::chrono::microseconds( 0 ) ) == std::future_status::ready )
				return true;
			else
				return false;
		}

		void get_thread_id( std::thread::id id )
		{
			_thread_id = id;
#if defined GHOST_TRACE_PARALLEL
			_log_filename << "test_run_parallel_" << _thread_id << ".txt";
			_log_trace.open( _log_filename.str() );
#endif

#if defined GHOST_TRACE
			COUT << "Creating a search unit for thread number " << _thread_id << "\n";
#endif
		}

		// Request the thread to stop searching
		inline void stop_search()	{	_stop_search_signal.set_value(); }
		inline Model&& transfer_model() { return std::move( model ); }

		// Method doing the search; called by Solver::fast_search (eventually in several threads).
		// Return true iff a solution has been found
		void local_search( double timeout )
		{
			// TODO: Antidote search
			// TODO: Neighborhood

			// 1. Choice of worst variable(s) to change
			// 2. Choice of their new value
			// 3. Error improved => make local move
			// 4. Same error
			// 4.a. Optimization cost improved => make local move
			// 4.b. Same optimization cost => plateau
			// 4.c. Worst optimization cost => local minimum
			// 4.d. Not an optimization problem => plateau
			// 5. Worst error => local minimum

			// A. Local move (perform local move and update variables/constraints/objective function)
			// B. Plateau management (local move on the plateau, but x% of chance to escape it, mark the variable as tabu.)
			// C. local minimum management (if there are no other worst variables to try, mark the variable as tabu.
			//                              Otherwise try them first, but with x% of chance, the solver fianlly marks the variable as tabu.)

			std::chrono::duration<double,std::micro> elapsed_time( 0 );
			std::chrono::time_point<std::chrono::steady_clock> start( std::chrono::steady_clock::now() );

			data.best_sat_error = std::numeric_limits<double>::max();
			data.best_opt_cost = std::numeric_limits<double>::max();

			initialize_variable_values();
			initialize_data_structures();

			std::transform( model.variables.begin(),
			                model.variables.end(),
			                final_solution.begin(),
			                [&](auto& var){ return var.get_value(); } );

			elapsed_time = std::chrono::steady_clock::now() - start;
			using namespace std::chrono_literals;

			int variable_to_change;

			// While timeout is not reached, and the solver didn't satisfied all constraints
			// OR
			// it is working on an optimization problem,
			// continue the search.
			while( !stop_search_requested()
			       &&  elapsed_time.count() < timeout
			       && ( data.best_sat_error > 0.0 || ( data.best_sat_error == 0.0 && data.is_optimization ) ) )
			{
				++data.search_iterations;
				
#if defined GHOST_FITNESS_CLOUD
				neighborhood_errors();
#endif

				/********************************************
				 * 1. Choice of worst variable(s) to change *
				 ********************************************/
#if defined GHOST_TRACE && not defined GHOST_FITNESS_CLOUD
				print_errors();

				for( int i = 0 ; i < data.number_variables; ++i )
					COUT << "Projected error of var[" << i << "]: " << data.error_variables[i] << "\n";
#endif

				// Estimate which variables need to be changed
				if( must_compute_variable_candidates )
					variable_candidates = variable_candidates_heuristic->compute_variable_candidates( data );

#if defined GHOST_TRACE
				if( std::count_if( data.tabu_list.begin(),
				                   data.tabu_list.end(),
				                   [&](int end_tabu){ return end_tabu > data.local_moves; } ) >= options.reset_threshold )
					COUT << "Number of variables marked as tabu above the threshold " << data.local_moves << "\n";
				if( variable_candidates.empty() )
					COUT << "Vector of variable candidates empty\n";
#endif
				
				if( std::count_if( data.tabu_list.begin(),
				                   data.tabu_list.end(),
				                   [&](int end_tabu){ return end_tabu > data.local_moves; } ) >= options.reset_threshold
				    || variable_candidates.empty() )
				{
#if defined GHOST_TRACE
					COUT << "No variables left to be changed: reset.\n";
#endif
					reset();
					continue;
				}

#if defined GHOST_TRACE  && not defined GHOST_FITNESS_CLOUD
				if( variable_candidates_heuristic->get_name().compare( "Adaptive Search" ) == 0 )
				{
					COUT << "\n(Adaptive Search Variable Candidates Heuristic) Variable candidates: v[" << static_cast<int>( variable_candidates[0] ) << "]=" << model.variables[ static_cast<int>( variable_candidates[0] ) ].get_value();
					for( int i = 1 ; i < static_cast<int>( variable_candidates.size() ) ; ++i )
						COUT << ", v[" << static_cast<int>( variable_candidates[i] ) << "]=" << model.variables[ static_cast<int>( variable_candidates[i] ) ].get_value();
					COUT << "\n";
				}
				else
					if( variable_candidates_heuristic->get_name().compare( "All Free" ) == 0 )
					{
						COUT << "\n(All Free Variable Candidates Heuristic) Variable candidates: v[" << static_cast<int>( variable_candidates[0] ) << "]=" << model.variables[ static_cast<int>( variable_candidates[0] ) ].get_value();
						for( int i = 1 ; i < static_cast<int>( variable_candidates.size() ) ; ++i )
							COUT << ", v[" << static_cast<int>( variable_candidates[i] ) << "]=" << model.variables[ static_cast<int>( variable_candidates[i] ) ].get_value();
						COUT << "\n";
					}
					else
						if( variable_candidates_heuristic->get_name().compare( "Antidote Search" ) == 0 )
						{
							auto distrib = std::discrete_distribution<int>( data.error_variables.begin(), data.error_variables.end() );
							std::vector<int> vec( data.number_variables, 0 );
							for( int n = 0 ; n < 10000 ; ++n )
								++vec[ rng.variate<int, std::discrete_distribution>( distrib ) ];
							std::vector<std::pair<int,int>> vec_pair( data.number_variables );
							for( int n = 0 ; n < data.number_variables ; ++n )
								vec_pair[n] = std::make_pair( n, vec[n] );
							std::sort( vec_pair.begin(), vec_pair.end(), [&](std::pair<int, int> &a, std::pair<int, int> &b){ return a.second > b.second; } );
							COUT << "\n(Antidote Search Variable Candidates Heuristic) Variable errors (normalized):\n";
							for( auto &v : vec_pair )
								COUT << "v[" << v.first << "]: " << std::fixed << std::setprecision(3) << static_cast<double>( v.second ) / 10000 << "\n";
						}
#endif

				variable_to_change = variable_heuristic->select_variable( variable_candidates, data, rng );

#if defined GHOST_TRACE  && not defined GHOST_FITNESS_CLOUD
				COUT << options.print->print_candidate( model.variables ).str();
				COUT << "\n********\nNumber of loop iteration: " << data.search_iterations << "\n";
				COUT << "Number of local moves performed: " << data.local_moves << "\n";
				COUT << "Tabu list <until_iteration>:";
				for( int i = 0 ; i < data.number_variables ; ++i )
					if( data.tabu_list[i] > data.local_moves )
						COUT << " v[" << i << "]:<" << data.tabu_list[i] << ">";
				COUT << "\n\nCurrent candidate: ";
				print_current_candidate();
				COUT << "\nCurrent error: " << data.current_sat_error;
				COUT << "\nPicked worst variable: v[" << variable_to_change << "]=" << model.variables[ variable_to_change ].get_value() << "\n\n";
#endif // end GHOST_TRACE

				/********************************
				 * 2. Choice of their new value *
				 ********************************/
				// Can we erase an element by "mistake" if we use the Antidote's variable heuristic? Is it a big deal?
				auto ref = std::find( variable_candidates.begin(), variable_candidates.end(), static_cast<double>( variable_to_change ) );
				if( ref != variable_candidates.end() )
					variable_candidates.erase( ref );
				
				// So far, we consider full domains only.
				auto domain_to_explore = model.variables[ variable_to_change ].get_full_domain();
				// Remove the current value
				domain_to_explore.erase( std::find( domain_to_explore.begin(), domain_to_explore.end(), model.variables[ variable_to_change ].get_value() ) );
				std::map<int, std::vector<double>> delta_errors;

				if( !model.permutation_problem )
				{
					// Simulate delta errors (or errors is not Constraint::optional_delta_error method is defined) for each neighbor
					for( const auto candidate_value : domain_to_explore )
					{
						if( !data.matrix_var_ctr.at( variable_to_change ).empty() ) [[likely]]
						{
							delta_errors[ candidate_value ].reserve( data.matrix_var_ctr.at( variable_to_change ).size() );
							for( const int constraint_id : data.matrix_var_ctr.at( variable_to_change ) )
								delta_errors[ candidate_value ].push_back( model.constraints[ constraint_id ]->simulate_delta( std::vector<int>{variable_to_change}, std::vector<int>{candidate_value} ) );
						}
						else
							delta_errors[ candidate_value ].push_back( 0.0 );
					}
				}
				else
				{
					for( int variable_id = 0 ; variable_id < data.number_variables; ++variable_id )
						// look at other variables than the selected one, with other values but contained into the selected variable's domain
						if( variable_id != variable_to_change
						    && model.variables[ variable_id ].get_value() != model.variables[ variable_to_change ].get_value()
						    && std::find( domain_to_explore.begin(), domain_to_explore.end(), model.variables[ variable_id ].get_value() ) != domain_to_explore.end()
						    && std::find( model.variables[ variable_id ].get_full_domain().begin(),
						                  model.variables[ variable_id ].get_full_domain().end(),
						                  model.variables[ variable_to_change ].get_value() ) != model.variables[ variable_id ].get_full_domain().end() )
						{
							std::vector<bool> constraint_checked( data.number_constraints, false );
							int current_value = model.variables[ variable_to_change ].get_value();
							int candidate_value = model.variables[ variable_id ].get_value();

							delta_errors[ variable_id ].reserve( data.matrix_var_ctr.at( variable_to_change ).size() + data.matrix_var_ctr.at( variable_id ).size() );
							for( const int constraint_id : data.matrix_var_ctr.at( variable_to_change ) )
							{
								constraint_checked[ constraint_id ] = true;

								// check if the other variable also belongs to the constraint scope
								if( model.constraints[ constraint_id ]->has_variable( variable_id ) )
									delta_errors[ variable_id ].push_back( model.constraints[ constraint_id ]->simulate_delta( std::vector<int>{variable_to_change, variable_id},
										                     std::vector<int>{candidate_value, current_value} ) );
								else
									delta_errors[ variable_id ].push_back( model.constraints[ constraint_id ]->simulate_delta( std::vector<int>{variable_to_change},
										                     std::vector<int>{candidate_value} ) );
							}

							// Since we are switching the value of two variables, we need to also look at the delta error impact of changing the value of the non-selected variable
							for( const int constraint_id : data.matrix_var_ctr.at( variable_id ) )
								// No need to look at constraint where variable_to_change also appears.
								if( !constraint_checked[ constraint_id ] )
									delta_errors[ variable_id ].push_back( model.constraints[ constraint_id ]->simulate_delta( std::vector<int>{variable_id},
										                     std::vector<int>{current_value} ) );
						}
				}

				// Select the next current configuration (local move)
				double min_conflict = std::numeric_limits<double>::max();
				int new_value = value_heuristic->select_value( variable_to_change, data, model, delta_errors, min_conflict, rng );
				
#if defined GHOST_TRACE && not defined GHOST_FITNESS_CLOUD
				std::vector<int> candidate_values;
				std::map<int, double> cumulated_delta_errors;
				std::vector<double> cumulated_delta_errors_antidote( delta_errors.size() );
				std::vector<double> cumulated_delta_errors_for_distribution( delta_errors.size() );
				std::vector<int> cumulated_delta_errors_variable_index_correspondance( delta_errors.size() );
				int index = 0;
				
				for( const auto& deltas : delta_errors )
				{
					cumulated_delta_errors[ deltas.first ] = std::accumulate( deltas.second.begin(), deltas.second.end(), 0.0 );
					cumulated_delta_errors_antidote[ index ] = cumulated_delta_errors[ deltas.first ];
					cumulated_delta_errors_variable_index_correspondance[ index ] = deltas.first;
					
					if( model.permutation_problem )
					{
						if( value_heuristic->get_name().compare( "Adaptive Search" ) == 0 )
						{
							COUT << "(Adaptive Search Value Heuristic) Error for switching var[" << variable_to_change << "]=" << model.variables[ variable_to_change ].get_value()
							     << " with var[" << deltas.first << "]=" << model.variables[ deltas.first ].get_value()
							     << ": " << cumulated_delta_errors[ deltas.first ] << "\n";
						}
						else
							if( value_heuristic->get_name().compare( "Random Walk" ) == 0 )
							{
								COUT << "(Random Walk Value Heuristic) Error for switching var[" << variable_to_change << "]=" << model.variables[ variable_to_change ].get_value()
								     << " with var[" << deltas.first << "]=" << model.variables[ deltas.first ].get_value()
								     << ": " << cumulated_delta_errors[ deltas.first ] << "\n";
							}
							else
								if( value_heuristic->get_name().compare( "Antidote Search" ) == 0 )
								{
									double transformed = cumulated_delta_errors_antidote[ index ] >= 0 ? 0.0 : -cumulated_delta_errors_antidote[ index ];
									COUT << "(Antidote Search Value Heuristic) Error for switching var[" << variable_to_change << "]=" << model.variables[ variable_to_change ].get_value()
									     << " with var[" << deltas.first << "]=" << model.variables[ deltas.first ].get_value()
									     << ": " << cumulated_delta_errors_antidote[ index ] << ", transformed: " << transformed << "\n";
								}
					}
					else
					{
						if( value_heuristic->get_name().compare( "Adaptive Search" ) == 0 )
							COUT << "(Adaptive Search Value Heuristic) Error for the value " << deltas.first << ": " << cumulated_delta_errors[ deltas.first ] << "\n";
						else
							if( value_heuristic->get_name().compare( "Random Walk" ) == 0 )
								COUT << "(Random Walk Value Heuristic) Error for the value " << deltas.first << ": " << cumulated_delta_errors[ deltas.first ] << "\n";
							else
								if( value_heuristic->get_name().compare( "Antidote Search" ) == 0 )
									COUT << "(Antidote Search Value Heuristic) Error for the value " << deltas.first << ": " << cumulated_delta_errors_antidote[ index ] << "\n";
					}
					++index;
				}
				
				std::transform( cumulated_delta_errors_antidote.begin(),
				                cumulated_delta_errors_antidote.end(),
				                cumulated_delta_errors_for_distribution.begin(),
				                []( auto delta ){ if( delta >= 0) return 0.0; else return -delta; } );

				auto min_conflict_copy = min_conflict;
				for( const auto& deltas : cumulated_delta_errors )
				{
					// Should not happen, except for Random Walks. min_conflict is supposed to be, well, the min conflict.
					if( min_conflict_copy > deltas.second )
					{
						candidate_values.clear();
						candidate_values.push_back( deltas.first );
						min_conflict_copy = deltas.second;
					}
					else
						if( min_conflict_copy == deltas.second )
							candidate_values.push_back( deltas.first );
				}
				
				if( value_heuristic->get_name().compare( "Adaptive Search" ) == 0 )
				{
					COUT << "(Adaptive Search Value Heuristic) Min conflict value candidates list: " << candidate_values[0];
					for( int i = 1 ; i < static_cast<int>( candidate_values.size() ); ++i )
						COUT << ", " << candidate_values[i];
					COUT << "\n";
				}
				else
					if( value_heuristic->get_name().compare( "Random Walk" ) == 0 )
					{
						COUT << "(Random Walk Value Heuristic) Min conflict value candidates list: " << candidate_values[0];
						for( int i = 1 ; i < static_cast<int>( candidate_values.size() ); ++i )
							COUT << ", " << candidate_values[i];
						COUT << "\n";
					}
					else
						if( value_heuristic->get_name().compare( "Antidote Search" ) == 0 )
						{				
							auto distrib_value = std::discrete_distribution<int>( cumulated_delta_errors_for_distribution.begin(), cumulated_delta_errors_for_distribution.end() );
							std::vector<int> vec_value( domain_to_explore.size(), 0 );
							for( int n = 0 ; n < 10000 ; ++n )
								++vec_value[ rng.variate<int, std::discrete_distribution>( distrib_value ) ];
							std::vector<std::pair<int,int>> vec_value_pair( domain_to_explore.size() );
							for( int n = 0 ; n < domain_to_explore.size() ; ++n )
								vec_value_pair[n] = std::make_pair( cumulated_delta_errors_variable_index_correspondance[n], vec_value[n] );
							std::sort( vec_value_pair.begin(), vec_value_pair.end(), [&](std::pair<int, int> &a, std::pair<int, int> &b){ return a.second > b.second; } );
							COUT << "\n(Antidote Search Value Heuristic) Cumulated delta error distribution (normalized):\n";
							for( int n = 0 ; n < domain_to_explore.size() ; ++n )
								COUT << "value " <<  vec_value_pair[ n ].first << " => " << std::fixed << std::setprecision(3) << static_cast<double>( vec_value_pair[ n ].second ) / 10000 << "\n";
						}
				
				if( model.permutation_problem )
					COUT << "\nPicked variable index for min conflict: "
					     << new_value << "\n"
					     << "Delta: " << min_conflict << "\n\n";
				else
					COUT << "\nPicked value for min conflict: "
					     << new_value << "\n"
					     << "Delta: " << min_conflict << "\n\n";
					
#endif // GHOST_TRACE

#if defined GHOST_RANDOM_WALK
				local_move( variable_to_change, new_value, min_conflict, delta_errors );
				if( data.is_optimization )
					data.current_opt_cost = model.objective->cost();
				if( data.best_sat_error > data.current_sat_error )
				{
#if defined GHOST_TRACE
					COUT << "Best satisfaction error so far (in an optimization problem). Before: " << data.best_sat_error << ", now: " << data.current_sat_error << "\n";
#endif
					data.best_sat_error = data.current_sat_error;
					std::transform( model.variables.begin(),
					                model.variables.end(),
					                final_solution.begin(),
					                [&](auto& var){ return var.get_value(); } );
				}
				else
					if( data.is_optimization && data.current_sat_error == 0.0 && data.best_opt_cost > data.current_opt_cost )
					{
#if defined GHOST_TRACE
						COUT << "Best objective function value so far. Before: " << data.best_opt_cost << ", now: " << data.current_opt_cost << "\n";
#endif
						data.best_opt_cost = data.current_opt_cost;
						std::transform( model.variables.begin(),
						                model.variables.end(),
						                final_solution.begin(),
						                [&](auto& var){ return var.get_value(); } );
					}

				elapsed_time = std::chrono::steady_clock::now() - start;
				continue;				
#endif
				
				/****************************************
				 * 3. Error improved => make local move *
				 ****************************************/
				if( min_conflict < 0.0 )
				{
#if defined GHOST_TRACE
					COUT << "Global error improved (" << data.current_sat_error << " -> " << data.current_sat_error + min_conflict << "): make local move.\n";
#endif
					local_move( variable_to_change, new_value, min_conflict, delta_errors );
					if( data.is_optimization )
						data.current_opt_cost = model.objective->cost();
				}
				else
				{
					/*****************
					 * 4. Same error *
					 *****************/
					if( min_conflict == 0.0 )
					{
#if defined GHOST_TRACE
						COUT << "Global error stable; ";
#endif
						if( data.is_optimization )
						{
							double candidate_opt_cost;
							if( model.permutation_problem )
							{
								int backup_variable_to_change = model.variables[ variable_to_change ].get_value();
								int backup_variable_new_value = model.variables[ new_value ].get_value();

								model.variables[ variable_to_change ].set_value( backup_variable_new_value );
								model.variables[ new_value ].set_value( backup_variable_to_change );

								model.auxiliary_data->update( variable_to_change, backup_variable_new_value );
								model.auxiliary_data->update( new_value, backup_variable_to_change );

								candidate_opt_cost = model.objective->cost();

								model.variables[ variable_to_change ].set_value( backup_variable_to_change );
								model.variables[ new_value ].set_value( backup_variable_new_value );

								model.auxiliary_data->update( variable_to_change, backup_variable_to_change );
								model.auxiliary_data->update( new_value, backup_variable_new_value );
							}
							else
							{
								int backup = model.variables[ variable_to_change ].get_value();

								model.variables[ variable_to_change ].set_value( new_value );
								model.auxiliary_data->update( variable_to_change, new_value );

								candidate_opt_cost = model.objective->cost();

								model.variables[ variable_to_change ].set_value( backup );
								model.auxiliary_data->update( variable_to_change, backup );
							}

							/******************************************************
							 * 4.a. Optimization cost improved => make local move *
							 ******************************************************/
							if( data.current_opt_cost > candidate_opt_cost )
							{
#if defined GHOST_TRACE
								COUT << "Optimization cost improved (" << data.current_opt_cost << " -> " << candidate_opt_cost << "): make local move.\n";
#endif
								local_move( variable_to_change, new_value, min_conflict, delta_errors );
								data.current_opt_cost = candidate_opt_cost;
							}
							else
								/******************************************
								 * 4.b. Same optimization cost => plateau *
								 ******************************************/
								if( data.current_opt_cost == candidate_opt_cost )
								{
#if defined GHOST_TRACE
									COUT << "Optimization cost stable (" << data.current_opt_cost << "): plateau.\n";
#endif
									plateau_management( variable_to_change, new_value, delta_errors );
								}
								else // data.current_opt_cost < candidate_opt_cost
								{
									/*************************************************
									 * 4.c. Worst optimization cost => local minimum *
									 *************************************************/
#if defined GHOST_TRACE
									COUT << "Optimization cost increase (" << data.current_opt_cost << " -> " << candidate_opt_cost << "): local minimum.\n";
									// Real local minimum in the following case
									if( variable_candidates.empty() )
									{
										COUT << "Local minimum candidate: ";
										print_current_candidate();
										COUT << "\nLocal minimum cost: " << data.current_opt_cost << "\n";
									}
#endif
									local_minimum_management( variable_to_change, new_value, variable_candidates.empty() );
								}
						}
						else
						{
							/***********************************************
							 * 4.d. Not an optimization problem => plateau *
							 ***********************************************/
#if defined GHOST_TRACE
							COUT << "No optimization: plateau.\n";
#endif
							plateau_management( variable_to_change, new_value, delta_errors );
						}
					}
					else // min_conflict > 0.0
					{
						/***********************************
						 * 5. Worst error => local minimum *
						 ***********************************/
#if defined GHOST_TRACE
						COUT << "Global error increase: local minimum.\n";
						// Real local minimum in the following case
						if( variable_candidates.empty() )
						{
							COUT << "Local minimum candidate: ";
							print_current_candidate();
							COUT << "\nLocal minimum error: " << data.current_sat_error << "\n";
						}
#endif
						local_minimum_management( variable_to_change, new_value, variable_candidates.empty() );
					}
				}

				if( data.best_sat_error > data.current_sat_error )
				{
#if defined GHOST_TRACE
					COUT << "Best satisfaction error so far (in an optimization problem). Before: " << data.best_sat_error << ", now: " << data.current_sat_error << "\n";
#endif
					data.best_sat_error = data.current_sat_error;
					std::transform( model.variables.begin(),
					                model.variables.end(),
					                final_solution.begin(),
					                [&](auto& var){ return var.get_value(); } );
				}
				else
					if( data.is_optimization && data.current_sat_error == 0.0 && data.best_opt_cost > data.current_opt_cost )
					{
#if defined GHOST_TRACE
						COUT << "Best objective function value so far. Before: " << data.best_opt_cost << ", now: " << data.current_opt_cost << "\n";
#endif
						data.best_opt_cost = data.current_opt_cost;
						std::transform( model.variables.begin(),
						                model.variables.end(),
						                final_solution.begin(),
						                [&](auto& var){ return var.get_value(); } );
					}

				elapsed_time = std::chrono::steady_clock::now() - start;
			} // while loop

			for( int i = 0 ; i < data.number_variables ; ++i )
				model.variables[i].set_value( final_solution[i] );

			solution_found.set_value( data.best_sat_error == 0.0 );

#if defined GHOST_TRACE_PARALLEL
			_log_trace.close();
#endif
		}
	};
}
