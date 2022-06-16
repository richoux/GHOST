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

#include "variable_heuristic.hpp"
#include "value_heuristic.hpp"
#include "algorithms/adaptive_search_variable_heuristic.hpp"
#include "algorithms/adaptive_search_value_heuristic.hpp"
#include "algorithms/antidote_search_variable_heuristic.hpp"
#include "algorithms/antidote_search_value_heuristic.hpp"

#if defined ANTIDOTE_SEARCH
#define ANTIDOTE_VARIABLE
#define ANTIDOTE_VALUE
#endif

#if defined GHOST_TRACE_PARALLEL
#define GHOST_TRACE
#include <fstream>
#include <sstream>
#define COUT _log_trace
#else
#define COUT std::cout
#endif

namespace ghost
{
	/*
	 * SearchUnit is the object called by Solver::solve to actually search for a solution.
	 * In parallel computing, one SearchUnit object is instanciated for every thread.
	 */
	class SearchUnit
	{
		std::promise<void> _stop_search_signal;
		std::future<void> _stop_search_check;
		std::thread::id _thread_id;

		std::unique_ptr<VariableHeuristic> variable_heuristic;
		std::unique_ptr<ValueHeuristic> value_heuristic;
		
#if defined GHOST_TRACE_PARALLEL
		std::stringstream _log_filename;
		std::ofstream _log_trace;
#endif

#if defined GHOST_TRACE
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
						COUT << "Better starting configuration found. Previous error: " << data.best_sat_error << ", now: " << best_sat_error_so_far << "\n";
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
				model.variables[ variables_index[ i ] ].pick_random_value();
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
			must_compute_worst_variables_list = true;
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

			// Reset variable costs
			std::fill( data.error_variables.begin(), data.error_variables.end(), 0.0 );
			// Recompute them
			compute_variables_errors();
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

#if not defined ANTIDOTE_VARIABLE // ADAPTIVE_SEARCH
		// To compute the vector of variables which are principal culprits for not satisfying the problem
		void compute_worst_variables()
		{
#if defined GHOST_TRACE
			print_errors();
#endif
			if( std::count_if( data.tabu_list.begin(),
			                   data.tabu_list.end(),
			                   [&](int end_tabu){ return end_tabu > data.local_moves; } ) >= options.reset_threshold ) ////////////////////// TODO À exécuter avant d'appeler l'heuristique
				worst_variables_list.clear();
			else
			{
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
			}
		}
#else // end ADAPTIVE_SEARCH ; start ANTIDOTE_SEARCH
		std::vector<double> mask_tabu( std::vector<double> error_variables )
		{
			if( std::count_if( data.tabu_list.begin(),
			                   data.tabu_list.end(),
			                   [&](int end_tabu){ return end_tabu > data.local_moves; } ) >= options.reset_threshold )
				error_variables.clear();
			else
			{
				for( int variable_id = 0; variable_id < data.number_variables; ++variable_id )
					if( data.tabu_list[ variable_id ] > data.local_moves )
						error_variables[ variable_id ] = 0.0;
			}

			return error_variables;
		}
#endif // end ANTIDOTE_SEARCH

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

		// Compute the variable cost of each variables and fill up error_variables
		void compute_variables_errors()
		{
			for( int variable_id = 0; variable_id < data.number_variables; ++variable_id )
				for( const int constraint_id : data.matrix_var_ctr.at( variable_id ) )
					data.error_variables[ variable_id ] += model.constraints[ constraint_id ]->_current_error;
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

					for( const int variable_id : model.constraints[ constraint_id ]->get_variable_ids() )
						data.error_variables[ variable_id ] += delta;

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

					for( const int variable_id : model.constraints[ constraint_id ]->get_variable_ids() )
						data.error_variables[ variable_id ] += delta;

					model.constraints[ constraint_id ]->update( variable_to_change, next_value );

					if( model.constraints[ constraint_id ]->has_variable( new_value ) )
						model.constraints[ constraint_id ]->update( new_value, current_value );
				}

				for( const int constraint_id : data.matrix_var_ctr.at( new_value ) )
					if( !constraint_checked[ constraint_id ] )
					{
						auto delta = delta_errors.at( new_value )[ delta_index++ ];
						model.constraints[ constraint_id ]->_current_error += delta;

						for( const int variable_id : model.constraints[ constraint_id ]->get_variable_ids() )
							data.error_variables[ variable_id ] += delta;

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
			must_compute_worst_variables_list = true;

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
		//                        of chance to escape it, mark the variables as tabu and loop to 2.)
		void plateau_management( int variable_to_change, int new_value, const std::map< int, std::vector<double>>& delta_errors )
		{
			if( rng.uniform(0, 100) <= options.percent_chance_escape_plateau )
			{
				data.tabu_list[ variable_to_change ] = options.tabu_time_local_min + data.local_moves;
				must_compute_worst_variables_list = true;
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

		// C. local minimum management (if there are no other worst variables to try, mark the variables as tabu and loop to 2.
		//                              Otherwise try them first, but with 10% of chance, the solver marks the variables as tabu and loop to 2)
		void local_minimum_management( int variable_to_change, int new_value, bool no_other_variables_to_try )
		{
			if( no_other_variables_to_try || rng.uniform(0, 100) <= 10 )
			{
				data.tabu_list[ variable_to_change ] = options.tabu_time_local_min + data.local_moves;
				must_compute_worst_variables_list = true;
				++data.local_minimum;
			}
			else
			{
#if defined GHOST_TRACE
				COUT << "Try other variables: not a local minimum yet.\n";
#endif
				must_compute_worst_variables_list = false;
			}
		}

	public:
		Model model;

		std::vector<Variable> variables_at_start;
		randutils::mt19937_rng rng;

		SearchUnitData data;
		
		std::vector<int> final_solution;

		std::vector<int> worst_variables_list;
		bool must_compute_worst_variables_list;

		std::promise<bool> solution_found;

		Options options;

		SearchUnit( Model&& moved_model,
		            const Options& options )
			: _stop_search_check( _stop_search_signal.get_future() ),
			  variable_heuristic( std::make_unique<AdaptiveSearchVariableHeuristic>() ),
			  value_heuristic( std::make_unique<AdaptiveSearchValueHeuristic>() ),
			  model( std::move( moved_model ) ),
			  data( model ),
			  final_solution( std::vector<int>( data.number_variables, 0 ) ),
			  worst_variables_list (),
			  must_compute_worst_variables_list ( true ),
			  options ( options )
		{
			std::transform( model.variables.begin(),
			                model.variables.end(),
			                std::back_inserter( variables_at_start ),
			                [&]( auto& v){ return v; } );

			data.initialize_matrix( model );

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

		// Method doing the search; called by Solver::solve (eventually in several threads).
		// Return true iff a solution has been found
		void search( double timeout )
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
			// B. Plateau management (local move on the plateau, but x% of chance to escape it, mark the variables as tabu and loop to 2.)
			// C. local minimum management (if there are no other worst variables to try, mark the variables as tabu and loop to 2.
			//                              Otherwise try them first, but with x% of chance, the solver marks the variables as tabu and loop to 2)

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

				/********************************************
				 * 1. Choice of worst variable(s) to change *
				 ********************************************/
				// Estimate which variables need to be changed
				if( must_compute_worst_variables_list )
					if( std::count_if( data.tabu_list.begin(),
					                   data.tabu_list.end(),
					                   [&](int end_tabu){ return end_tabu > data.local_moves; } ) >= options.reset_threshold )
					{
#if defined GHOST_TRACE
						COUT << "No variables left to be changed: reset.\n";
#endif
						reset();
						continue;
					}
				
				variable_to_change = variable_heuristic->select_variable_candidates( data );

#if defined GHOST_TRACE
				COUT << options.print->print_candidate( model.variables ).str();
				COUT << "\n\nNumber of loop iteration: " << data.search_iterations << "\n";
				COUT << "Number of local moves performed: " << data.local_moves << "\n";
				COUT << "Tabu list:";
				for( int i = 0 ; i < data.number_variables ; ++i )
					if( data.tabu_list[i] > data.local_moves )
						COUT << " v[" << i << "]:" << data.tabu_list[i];
#if not defined ANTIDOTE_VARIABLE // ADAPTIVE_SEARCH
				COUT << "\nWorst variables list: v[" << worst_variables_list[0] << "]=" << model.variables[ worst_variables_list[0] ].get_value();
				for( int i = 1 ; i < static_cast<int>( worst_variables_list.size() ) ; ++i )
					COUT << ", v[" << worst_variables_list[i] << "]=" << model.variables[ worst_variables_list[i] ].get_value();
#else // end ADAPTIVE_SEARCH ; start ANTIDOTE_SEARCH
				auto distrib = std::discrete_distribution<int>( masked_error_variables.begin(), masked_error_variables.end() );
				std::vector<int> vec( data.number_variables, 0 );
				for( int n = 0 ; n < 10000 ; ++n )
					++vec[ rng.variate<int, std::discrete_distribution>( distrib ) ];
				std::vector<std::pair<int,int>> vec_pair( data.number_variables );
				for( int n = 0 ; n < data.number_variables ; ++n )
					vec_pair[n] = std::make_pair( n, vec[n] );
				std::sort( vec_pair.begin(), vec_pair.end(), [&](std::pair<int, int> &a, std::pair<int, int> &b){ return a.second > b.second; } );
				COUT << "\nVariable errors (normalized):\n";
				for( auto &v : vec_pair )
					COUT << "v[" << v.first << "]: " << std::fixed << std::setprecision(3) << static_cast<double>( v.second ) / 10000 << "\n";
#endif // end ANTIDOTE_SEARCH
				COUT << "\nPicked worst variable: v[" << variable_to_change << "]=" << model.variables[ variable_to_change ].get_value() << "\n\n";
#endif // end GHOST_TRACE

				/********************************
				 * 2. Choice of their new value *
				 ********************************/
#if not defined ANTIDOTE_VARIABLE // ADAPTIVE_SEARCH
				// TODO: delete this
				// worst_variables_list.erase( std::find( worst_variables_list.begin(), worst_variables_list.end(), variable_to_change ) );
#endif
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
							for( const int constraint_id : data.matrix_var_ctr.at( variable_to_change ) )
								delta_errors[ candidate_value ].push_back( model.constraints[ constraint_id ]->simulate_delta( std::vector<int>{variable_to_change}, std::vector<int>{candidate_value} ) );
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
				int new_value;
				double min_conflict = std::numeric_limits<double>::max();
				std::vector<int> candidate_values;

#if not defined ANTIDOTE_VALUE // ADAPTIVE_SEARCH
				std::map<int, double> cumulated_delta_errors;
				for( const auto& deltas : delta_errors )
				{
					cumulated_delta_errors[ deltas.first ] = std::accumulate( deltas.second.begin(), deltas.second.end(), 0.0 );
#if defined GHOST_TRACE
					if( model.permutation_problem )
						COUT << "Error for switching var[" << variable_to_change << "]=" << model.variables[ variable_to_change ].get_value()
						     << " with var[" << deltas.first << "]=" << model.variables[ deltas.first ].get_value()
						     << ": " << cumulated_delta_errors[ deltas.first ] << "\n";
					else
						COUT << "Error for the value " << deltas.first << ": " << cumulated_delta_errors[ deltas.first ] << "\n";
#endif
				}

				for( const auto& deltas : cumulated_delta_errors )
				{
					if( min_conflict > deltas.second )
					{
						candidate_values.clear();
						candidate_values.push_back( deltas.first );
						min_conflict = deltas.second;
					}
					else
						if( min_conflict == deltas.second )
							candidate_values.push_back( deltas.first );
				}

				// if we deal with an optimization problem, find the value minimizing to objective function
				if( data.is_optimization )
				{
					if( model.permutation_problem )
						new_value = static_cast<int>( model.objective->heuristic_value_permutation( variable_to_change, candidate_values ) );
					else
						new_value = model.objective->heuristic_value( variable_to_change, candidate_values );

					// // to change/test with heuristic_value
					// double objective_cost = std::numeric_limits<double>::max();
					// double simulate_objective_function;
					// for( int value : candidate_values )
					// {
					// 	if( model.permutation_problem )
					// 		simulate_objective_function = _objective.simulate_cost( std::vector<int>{variable_to_change, value}, std::vector<int>{_variables[ value ].get_value(),_variables[ variable_to_change ].get_value()} );
					// 	else
					// 		simulate_objective_function = _objective.simulate_cost( std::vector<int>{variable_to_change}, std::vector<int>{value} );

					// 	if( objective_cost > simulate_objective_function )
					// 	{
					// 		objective_cost = simulate_objective_function;
					// 		new_value = value;
					// 	}
					// }
				}
				else
					new_value = rng.pick( candidate_values );
#else // end ADAPTIVE_SEARCH ; start ANTIDOTE_SEARCH
				std::vector<double> cumulated_delta_errors( delta_errors.size() );
				std::vector<double> cumulated_delta_errors_for_distribution( delta_errors.size() );
				std::vector<int> cumulated_delta_errors_variable_index_correspondance( delta_errors.size() ); // longest variable name ever

				int index = 0;

				for( const auto& deltas : delta_errors )
				{
					cumulated_delta_errors[ index ] = std::accumulate( deltas.second.begin(), deltas.second.end(), 0.0 );
					cumulated_delta_errors_variable_index_correspondance[ index ] = deltas.first;

					double transformed = cumulated_delta_errors[ index ] >= 0 ? 0.0 : -cumulated_delta_errors[ index ];

#if defined GHOST_TRACE
					if( model.permutation_problem )
						COUT << "Error for switching var[" << variable_to_change << "]=" << model.variables[ variable_to_change ].get_value()
						     << " with var[" << deltas.first << "]=" << model.variables[ deltas.first ].get_value()
						     << ": " << cumulated_delta_errors[ index ] << ", transformed: " << transformed << "\n";
					else
						COUT << "Error for the value " << deltas.first << ": " << cumulated_delta_errors[ index ] << "\n";
#endif
					++index;
				}

				std::transform( cumulated_delta_errors.begin(),
				                cumulated_delta_errors.end(),
				                cumulated_delta_errors_for_distribution.begin(),
				                [&]( auto delta ){ if( delta >= 0) return 0.0; else return -delta; } );

				if( *std::max_element( cumulated_delta_errors_for_distribution.begin(), cumulated_delta_errors_for_distribution.end() ) == 0.0 )
					index = rng.uniform( 0, static_cast<int>( delta_errors.size() ) - 1 );
				else
					index = rng.variate<int, std::discrete_distribution>( cumulated_delta_errors_for_distribution.begin(), cumulated_delta_errors_for_distribution.end() );

				min_conflict = cumulated_delta_errors[ index ];
				new_value = cumulated_delta_errors_variable_index_correspondance[ index ];				
#endif // end ANTIDOTE_SEARCH

#if defined GHOST_TRACE
#if not defined ANTIDOTE_VALUE // ADAPTIVE_SEARCH
				COUT << "Min conflict value candidates list: " << candidate_values[0];
				for( int i = 1 ; i < static_cast<int>( candidate_values.size() ); ++i )
					COUT << ", " << candidate_values[i];
#else // end ADAPTIVE_SEARCH ; start ANTIDOTE_SEARCH
				auto distrib_value = std::discrete_distribution<int>( cumulated_delta_errors_for_distribution.begin(), cumulated_delta_errors_for_distribution.end() );
				std::vector<int> vec_value( domain_to_explore.size(), 0 );
				for( int n = 0 ; n < 10000 ; ++n )
					++vec_value[ rng.variate<int, std::discrete_distribution>( distrib_value ) ];
				std::vector<std::pair<int,int>> vec_value_pair( domain_to_explore.size() );
				for( int n = 0 ; n < domain_to_explore.size() ; ++n )
					vec_value_pair[n] = std::make_pair( cumulated_delta_errors_variable_index_correspondance[n], vec_value[n] );
				std::sort( vec_value_pair.begin(), vec_value_pair.end(), [&](std::pair<int, int> &a, std::pair<int, int> &b){ return a.second > b.second; } );
				COUT << "Cumulated delta error distribution (normalized):\n";
				for( int n = 0 ; n < domain_to_explore.size() ; ++n )
					COUT << "val[" <<  vec_value_pair[ n ].first << "]=" << model.variables[ vec_value_pair[ n ].first ].get_value() << " => " << std::fixed << std::setprecision(3) << static_cast<double>( vec_value_pair[ n ].second ) / 10000 << "\n";
#endif // end ANTIDOTE_SEARCH
				if( model.permutation_problem )
					COUT << "\nPicked variable index for min conflict: "
					     << new_value << "\n"
					     << "Current error: " << data.current_sat_error << "\n"
					     << "Delta: " << min_conflict << "\n\n";
				else
					COUT << "\nPicked value for min conflict: "
					     << new_value << "\n"
					     << "Current error: " << data.current_sat_error << "\n"
					     << "Delta: " << min_conflict << "\n\n";
#endif // GHOST_TRACE

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
								COUT << "optimization cost improved (" << data.current_opt_cost << " -> " << candidate_opt_cost << "): make local move.\n";
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
									COUT << "optimization cost stable (" << data.current_opt_cost << "): plateau.\n";
#endif
									plateau_management( variable_to_change, new_value, delta_errors );
								}
								else // data.current_opt_cost < candidate_opt_cost
								{
									/*************************************************
									 * 4.c. Worst optimization cost => local minimum *
									 *************************************************/
#if defined GHOST_TRACE
									COUT << "optimization cost increase (" << data.current_opt_cost << " -> " << candidate_opt_cost << "): local minimum.\n";
#endif
									local_minimum_management( variable_to_change, new_value, worst_variables_list.empty() );
								}
						}
						else
						{
							/***********************************************
							 * 4.d. Not an optimization problem => plateau *
							 ***********************************************/
#if defined GHOST_TRACE
							COUT << "no optimization: plateau.\n";
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
#endif
						local_minimum_management( variable_to_change, new_value, worst_variables_list.empty() );
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
