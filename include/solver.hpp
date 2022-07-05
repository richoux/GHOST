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

#pragma once

#include <iostream>
#include <iomanip>
#include <cmath>
#include <limits>
#include <random>
#include <algorithm>
#include <vector>
#include <chrono>
#include <memory>
#include <iterator>
#include <thread>
#include <future>

#include "variable.hpp"
#include "constraint.hpp"
#include "objective.hpp"
#include "auxiliary_data.hpp"
#include "model.hpp"
#include "model_builder.hpp"
#include "options.hpp"
#include "search_unit.hpp"

#include "algorithms/variable_heuristic.hpp"
#include "algorithms/variable_candidates_heuristic.hpp"
#include "algorithms/value_heuristic.hpp"
#include "algorithms/error_projection_heuristic.hpp"

#include "algorithms/adaptive_search_variable_heuristic.hpp"
#include "algorithms/adaptive_search_variable_candidates_heuristic.hpp"
#include "algorithms/adaptive_search_value_heuristic.hpp"
#include "algorithms/adaptive_search_error_projection_heuristic.hpp"

#include "algorithms/antidote_search_variable_heuristic.hpp"
#include "algorithms/antidote_search_variable_candidates_heuristic.hpp"
#include "algorithms/antidote_search_value_heuristic.hpp"

#include "algorithms/culprit_search_error_projection_heuristic.hpp"

#include "macros.hpp"

namespace ghost
{
	/*!
	 * Solver is the class coding the solver itself.
	 *
	 * To solve a problem instance, users must instanciate a Solver object, then run Solver::solve.
	 *
	 * The unique Solver constructor needs a derived ghost::ModelBuilder object,
	 * as well as an optional boolean indicating if the solver is dealing with a permutation problem,
	 * i.e., if the solver needs to swap variable values instead of picking new values from domains.
	 *
	 * Declaring combinatorial problems as permutation problems can lead to a huge performance boost
	 * for the solver. For this, the problem needs to be declared with all variables starting with a 
	 * value that belongs to a solution.
	 *
	 * This is typically the case for scheduling problems, for instance: imagine we want to do three
	 * tasks A, B and C. Thus, we give A as the starting value to the first variable, B to the second 
	 * and C to the third. Then, instead of assigning the task A to the second variable for instance, 
	 * the solver will swap tasks of the first and the second variables.
	 *
	 * Users are invited to model as much as possible their problems as permutation problems, since
	 * it would greatly speed-up the search of solutions.
	 *
	 * Many options compiled in a ghost::Options object can be passed to the method Solver::solve, to
	 * allow for instance parallel computing, as well as parameter tweaking for local search experts.
	 *
	 * ghost::Solver is a template class, although users should never need to instantiate the template
	 * with modern C++ compilers.
	 *
	 * \sa ModelBuilder, Options
	 */
	template<typename ModelBuilderType> class Solver final
	{
		Model _model;
		ModelBuilderType _model_builder; // Factory building the model

		int _number_variables; // Size of the vector of variables.
		int _number_constraints; // Size of the vector of constraints.

		double _best_sat_error;
		double _best_opt_cost;
		double _cost_before_postprocess;

		// global statistics, cumulation of all threads stats.
		int _restarts_total;
		int _resets_total;
		int _local_moves_total;
		int _search_iterations_total;
		int _local_minimum_total;
		int _plateau_moves_total;
		int _plateau_local_minimum_total;

		// stats of the winning thread
		int _restarts;
		int _resets;
		int _local_moves;
		int _search_iterations;
		int _local_minimum;
		int _plateau_moves;
		int _plateau_local_minimum;

		std::string _variable_heuristic;
		std::string _variable_candidates_heuristic;
		std::string _value_heuristic;
		std::string _error_projection_heuristic;
		
		Options _options; // Options for the solver (see the struct Options).

	public:
		/*!
		 * Unique constructor of ghost::Solver
		 *
		 * \param model_builder a const reference to a derived ModelBuilder object.
		 * \param permutation_problem a boolean indicating if the solver will work on a permutation
		 * problem. False by default.
		 */
		Solver( const ModelBuilderType& model_builder )
			: _model_builder( model_builder ),
			  _best_sat_error( std::numeric_limits<double>::max() ),
			  _best_opt_cost( std::numeric_limits<double>::max() ),
			  _cost_before_postprocess( std::numeric_limits<double>::max() ),
			  _restarts_total( 0 ),
			  _resets_total( 0 ),
			  _local_moves_total( 0 ),
			  _search_iterations_total( 0 ),
			  _local_minimum_total( 0 ),
			  _plateau_moves_total( 0 ),
			  _plateau_local_minimum_total( 0 ),
			  _restarts( 0 ),
			  _resets( 0 ),
			  _local_moves( 0 ),
			  _search_iterations( 0 ),
			  _local_minimum( 0 ),
			  _plateau_moves( 0 ),
			  _plateau_local_minimum( 0 )
		{	}

		/*!
		 * Method to solve the given CSP/COP/ESFP/EFOP model. Users should favor the two versions of
		 * Solver::solve taking a std::chrono::microseconds value as a parameter.
		 *
		 * This method is the heart of GHOST's solver: it will try to find a solution within a
		 * limited time. If it finds such a solution, the function outputs the value true.\n
		 * Here how it works: if at least one solution is found, at the end of the computation,
		 * it will write in the two first parameters final_cost and final_solution the error/cost
		 * of the best candidate or solution found and the value of each variable.\n
		 * For a satisfaction problem (without any objective function), the error of a candidate
		 * is the sum of the error of each problem constraint (computated by
		 * Constraint::required_error). For an optimization problem, the cost is the value outputed
		 * by Objective::required_cost.\n
		 * For both, the lower value the better: A satisfaction error of 0 means we have a solution
		 * to a satisfaction problem (ie, all constraints are satisfied). An optimization cost should
		 * be as low as possible: GHOST is always trying to minimize problems. If you have a
		 * maximization problem, GHOST will automatically convert it into a minimization problem.
		 *
		 * The timeout parameter is fundamental: it represents a time budget, in microseconds, for
		 * the solver. The behavior will differ from satisfaction and optimization problems.
		 *
		 * For satisfaction problems modeled with an CSP or EFSP, the solver stops as soon as it
		 * finds a solution. Then, it outputs 'true', writes 0 into the final_cost variable and the 
		 * values of the variables composing the solution into the final_solution vector.\n
		 * If no solutions are found within the timeout, the solver stops, outputs 'false', writes
		 * in final_cost the error of the best candidate found during the search (i.e., the candidate
		 * being the closest from a solution) and writes the best candidate's values into the 
		 * final_solution vector.
		 *
		 * For optimization problems modeled with an COP or EFOP, the solver will always continue 
		 * running until reaching the timeout. If a solution is found, it outputs 'true' and writes
		 * into the final_cost variable the cost of the best solution optimizating the given objective
		 * function. It also writes the values of the solution into the final_solution vector.\n
		 * If no solutions are found, the solver outputs 'false' and adopt the same behavior as not
		 * finding a solution for satisfaction problems.
		 *
		 * Finally, options to change the solver behaviors (parallel runs, user-defined solution
		 * printing, user-defined starting candidate, parameter tweaking, etc) can be given as
		 * a last parameter.
		 *
		 * \param final_cost a reference to a double to get the error of the best candidate or
		 * solution for satisfaction problems, or the objective function value of the best solution
		 * for optimization problems (or the cost of the best candidate if no solution has been
		 * found). For satisfaction problems, a cost of zero means a solution has been found.
		 * \param final_solution a reference to a vector of integers, to get values of the best
		 *  candidate or solution found.
		 * \param timeout a double for the time budget allowed to the solver to find a solution,
		 * in microseconds.
		 * \param options a reference to an Options object containing options such as parallel runs,
		 * a solution printer, if the solver must start with a custom variable assignment,
		 * parameter tuning, etc.
		 * \return True if and only if a solution has been found.
		 */
		bool solve( double& final_cost,
		            std::vector<int>& final_solution,
		            double timeout,
		            Options& options )
		{
			std::chrono::time_point<std::chrono::steady_clock> start_wall_clock( std::chrono::steady_clock::now() );
			std::chrono::time_point<std::chrono::steady_clock> start_search;
			std::chrono::time_point<std::chrono::steady_clock> start_postprocess;
			std::chrono::duration<double,std::micro> elapsed_time( 0 );
			std::chrono::duration<double,std::micro> timer_postprocess( 0 );

			/*****************
			* Initialization *
			******************/
			// Only to get the number of variables and constraints
			_model_builder.declare_variables();
			_number_variables = _model_builder.get_number_variables();

			_options = options;

			if( _options.tabu_time_local_min < 0 )
				_options.tabu_time_local_min = std::max( std::min( 5, static_cast<int>( _number_variables ) - 1 ), static_cast<int>( std::ceil( _number_variables / 5 ) ) ) + 1;
			  //_options.tabu_time_local_min = std::max( 2, _tabu_threshold ) );

			if( _options.tabu_time_selected < 0 )
				_options.tabu_time_selected = 0;

			if( _options.percent_chance_escape_plateau < 0 || _options.percent_chance_escape_plateau > 100 )
				_options.percent_chance_escape_plateau = 10;

			if( _options.reset_threshold < 0 )
				_options.reset_threshold = _options.tabu_time_local_min;
// 			_options.reset_threshold = static_cast<int>( std::ceil( 1.5 * _options.reset_threshold ) );
//		  _options.reset_threshold = 2 * static_cast<int>( std::ceil( std::sqrt( _number_variables ) ) );

			if( _options.restart_threshold < 0 )
				_options.restart_threshold = _number_variables;

			if( _options.number_variables_to_reset < 0 )
				_options.number_variables_to_reset = std::max( 2, static_cast<int>( std::ceil( _number_variables * 0.1 ) ) ); // 10%

			if( _options.number_start_samplings < 0 )
				_options.number_start_samplings = 10;

			double chrono_search;
			double chrono_full_computation;

			// In case final_solution is not a vector of the correct size,
			// ie, equals to the number of variables.
			final_solution.resize( _number_variables );
			bool solution_found = false;
			bool is_sequential;
			bool is_optimization;

#if defined GHOST_DEBUG || defined GHOST_TRACE || defined GHOST_BENCH
			// this is to make proper benchmarks/debugging with 1 thread.
			is_sequential = !_options.parallel_runs;
#else
			is_sequential = ( !_options.parallel_runs || _options.number_threads == 1 );
#endif

			// sequential runs
			if( is_sequential )
			{
				// SearchUnit search_unit( _model_builder.build_model(),
				//                         _options );
				SearchUnit search_unit( _model_builder.build_model(),
				                        _options,
				                        std::make_unique<algorithms::AdaptiveSearchVariableHeuristic>(),
				                        std::make_unique<algorithms::AdaptiveSearchVariableCandidatesHeuristic>(),
				                        std::make_unique<algorithms::AdaptiveSearchValueHeuristic>(),
				                        std::make_unique<algorithms::AdaptiveSearchErrorProjection>() );
				// SearchUnit search_unit( _model_builder.build_model(),
				//                         _options,
				//                         std::make_unique<algorithms::AntidoteSearchVariableHeuristic>(),
				//                         std::make_unique<algorithms::AntidoteSearchVariableCandidatesHeuristic>(),
				//                         std::make_unique<algorithms::AntidoteSearchValueHeuristic>(),
				//                         std::make_unique<algorithms::CulpritSearchErrorProjection>() );

				is_optimization = search_unit.data.is_optimization;
				std::future<bool> unit_future = search_unit.solution_found.get_future();

				start_search = std::chrono::steady_clock::now();
				search_unit.search( timeout );
				elapsed_time = std::chrono::steady_clock::now() - start_search;
				chrono_search = elapsed_time.count();

				solution_found = unit_future.get();
				_best_sat_error = search_unit.data.best_sat_error;
				_best_opt_cost = search_unit.data.best_opt_cost;
				_restarts = search_unit.data.restarts;
				_resets = search_unit.data.resets;
				_local_moves = search_unit.data.local_moves;
				_search_iterations = search_unit.data.search_iterations;
				_local_minimum = search_unit.data.local_minimum;
				_plateau_moves = search_unit.data.plateau_moves;
				_plateau_local_minimum = search_unit.data.plateau_local_minimum;

				_variable_heuristic = search_unit.variable_heuristic->get_name();
				_variable_candidates_heuristic = search_unit.variable_candidates_heuristic->get_name();
				_value_heuristic = search_unit.value_heuristic->get_name();
				_error_projection_heuristic = search_unit.error_projection_heuristic->get_name();
				
				_model = std::move( search_unit.transfer_model() );
			}
			else // call threads
			{
				std::vector<SearchUnit> units;
				units.reserve( _options.number_threads );
				std::vector<std::thread> unit_threads;

				for( int i = 0 ; i < _options.number_threads; ++i )
				{
					// Instantiate one model per thread
					// units.emplace_back( _model_builder.build_model(),
					//                     _options );
					units.emplace_back( _model_builder.build_model(),
					                    _options,
					                    std::make_unique<algorithms::AdaptiveSearchVariableHeuristic>(),
					                    std::make_unique<algorithms::AdaptiveSearchVariableCandidatesHeuristic>(),
					                    std::make_unique<algorithms::AdaptiveSearchValueHeuristic>(),
					                    std::make_unique<algorithms::AdaptiveSearchErrorProjection>() );
					// units.emplace_back( _model_builder.build_model(),
					//                     _options,
					//                     std::make_unique<algorithms::AntidoteSearchVariableHeuristic>(),
					//                     std::make_unique<algorithms::AntidoteSearchVariableCandidatesHeuristic>(),
					//                     std::make_unique<algorithms::AntidoteSearchValueHeuristic>(),
					//                     std::make_unique<algorithms::CulpritSearchErrorProjection>() );
				}

				is_optimization = units[0].data.is_optimization;

				std::vector<std::future<bool>> units_future;
				std::vector<bool> units_terminated( _options.number_threads, false );

				start_search = std::chrono::steady_clock::now();

				for( int i = 0 ; i < _options.number_threads; ++i )
				{
					unit_threads.emplace_back( &SearchUnit::search, &units.at(i), timeout );
					units.at( i ).get_thread_id( unit_threads.at( i ).get_id() );
					units_future.emplace_back( units.at( i ).solution_found.get_future() );
				}

				int thread_number = 0;
				int winning_thread = 0;
				bool end_of_computation = false;
				int number_timeouts = 0;

				while( !end_of_computation )
				{
					for( thread_number = 0 ; thread_number < _options.number_threads ; ++thread_number )
					{
						if( !units_terminated[ thread_number ] && units_future.at( thread_number ).wait_for( std::chrono::microseconds( 0 ) ) == std::future_status::ready )
						{
							if( is_optimization )
							{
								++number_timeouts;
								units_terminated[ thread_number ] = true;

								if( units_future.at( thread_number ).get() ) // equivalent to if( units.at( thread_number ).best_sat_error == 0.0 )
								{
									solution_found = true;
									if( _best_opt_cost > units.at( thread_number ).data.best_opt_cost )
									{
										_best_opt_cost = units.at( thread_number ).data.best_opt_cost;
										winning_thread = thread_number;
									}
								}

								if( number_timeouts >= _options.number_threads )
								{
									end_of_computation = true;
									break;
								}
							}
							else // then it is a satisfaction problem
							{
								if( units_future.at( thread_number ).get() )
								{
									solution_found = true;
									units_terminated[ thread_number ] = true;
									winning_thread = thread_number;
									end_of_computation = true;
									break;
								}
								else
								{
									++number_timeouts;
									units_terminated[ thread_number ] = true;
									if( number_timeouts >= _options.number_threads )
									{
										end_of_computation = true;
										break;
									}
								}
							}
						}
					}
				}

				elapsed_time = std::chrono::steady_clock::now() - start_search;
				chrono_search = elapsed_time.count();

				// Collect all interesting data before terminating threads.
				// Stats first...
				for( int i = 0 ; i < _options.number_threads ; ++i )
				{
					units.at(i).stop_search();

					_restarts_total += units.at(i).data.restarts;
					_resets_total += units.at(i).data.resets;
					_local_moves_total += units.at(i).data.local_moves;
					_search_iterations_total += units.at(i).data.search_iterations;
					_local_minimum_total += units.at(i).data.local_minimum;
					_plateau_moves_total += units.at(i).data.plateau_moves;
					_plateau_local_minimum_total += units.at(i).data.plateau_local_minimum;
				}

				// ..then the most important: the best solution found so far.
				if( solution_found )
				{
#if defined GHOST_TRACE
					std::cout << "Parallel run, thread number " << winning_thread << " has found a solution.\n";
#endif
					_best_sat_error = units.at( winning_thread ).data.best_sat_error;
					_best_opt_cost = units.at( winning_thread ).data.best_opt_cost;

					_restarts = units.at( winning_thread ).data.restarts;
					_resets = units.at( winning_thread ).data.resets;
					_local_moves = units.at( winning_thread ).data.local_moves;
					_search_iterations = units.at( winning_thread ).data.search_iterations;
					_local_minimum = units.at( winning_thread ).data.local_minimum;
					_plateau_moves = units.at( winning_thread ).data.plateau_moves;
					_plateau_local_minimum = units.at( winning_thread ).data.plateau_local_minimum;

					_variable_heuristic = units.at( winning_thread ).variable_heuristic->get_name();
					_variable_candidates_heuristic = units.at( winning_thread ).variable_candidates_heuristic->get_name();
					_value_heuristic = units.at( winning_thread ).value_heuristic->get_name();
					_error_projection_heuristic = units.at( winning_thread ).error_projection_heuristic->get_name();

					_model = std::move( units.at( winning_thread ).transfer_model() );
				}
				else
				{
#if defined GHOST_TRACE
					std::cout << "Parallel run, no solutions found.\n";
#endif
					int best_non_solution = 0;
					for( int i = 0 ; i < _options.number_threads ; ++i )
					{
						if( _best_sat_error > units.at( i ).data.best_sat_error )
						{
							best_non_solution = i;
							_best_sat_error = units.at( i ).data.best_sat_error;
						}
						if( is_optimization && _best_sat_error == 0.0 )
							if( units.at( i ).data.best_sat_error == 0.0 && _best_opt_cost > units.at( i ).data.best_opt_cost )
							{
								best_non_solution = i;
								_best_opt_cost = units.at( i ).data.best_opt_cost;
							}
					}

					_restarts = units.at( best_non_solution ).data.restarts;
					_resets = units.at( best_non_solution ).data.resets;
					_local_moves = units.at( best_non_solution ).data.local_moves;
					_search_iterations = units.at( best_non_solution ).data.search_iterations;
					_local_minimum = units.at( best_non_solution ).data.local_minimum;
					_plateau_moves = units.at( best_non_solution ).data.plateau_moves;
					_plateau_local_minimum = units.at( best_non_solution ).data.plateau_local_minimum;

					_variable_heuristic = units.at( best_non_solution ).variable_heuristic->get_name();
					_variable_candidates_heuristic = units.at( best_non_solution ).variable_candidates_heuristic->get_name();
					_value_heuristic = units.at( best_non_solution ).value_heuristic->get_name();
					_error_projection_heuristic = units.at( best_non_solution ).error_projection_heuristic->get_name();
					
					_model = std::move( units.at( best_non_solution ).transfer_model() );
				}

				for( auto& thread: unit_threads )
				{
#if defined GHOST_TRACE
					std::cout << "Joining and terminating thread number " << thread.get_id() << "\n";
#endif
					thread.join();
				}
			}

			if( solution_found && is_optimization )
			{
				_cost_before_postprocess = _best_opt_cost;

				start_postprocess = std::chrono::steady_clock::now();
				_best_opt_cost = _model.objective->postprocess( _best_opt_cost );
				timer_postprocess = std::chrono::steady_clock::now() - start_postprocess;
			}

			if( is_optimization )
			{
				if( _best_opt_cost < 0 )
				{
					_best_opt_cost = -_best_opt_cost;
					_cost_before_postprocess = -_cost_before_postprocess;
				}

				final_cost = _best_opt_cost;
			}
			else
				final_cost = _best_sat_error;

			std::transform( _model.variables.begin(),
			                _model.variables.end(),
			                final_solution.begin(),
			                [&](auto& var){ return var.get_value(); } );

			elapsed_time = std::chrono::steady_clock::now() - start_wall_clock;
			chrono_full_computation = elapsed_time.count();

#if defined GHOST_DEBUG || defined GHOST_TRACE || defined GHOST_BENCH
			std::cout << "@@@@@@@@@@@@" << "\n"
			          << "Variable heuristic: " << _variable_heuristic << "\n"
			          << "Variable candidate heuristic: " << _variable_candidates_heuristic << "\n"
			          << "Value heuristic: " << _value_heuristic << "\n"
			          << "Error projection heuristic: " << _error_projection_heuristic << "\n"
			          << "Started from a custom variables assignment: " << std::boolalpha << _options.custom_starting_point << "\n"
			          << "Search resumed from a previous run: " << std::boolalpha << _options.resume_search << "\n"
			          << "Parallel search: " << std::boolalpha << _options.parallel_runs << "\n"
			          << "Number of threads (not used if no parallel search): " << _options.number_threads << "\n"
			          << "Number of variable assignments samplings at start (if custom start and resume are set to false): " << _options.number_start_samplings << "\n"
			          << "Variables of local minimum are frozen for: " << _options.tabu_time_local_min << " local moves\n"
			          << "Selected variables are frozen for: " << _options.tabu_time_selected << " local moves\n"
			          << "Percentage of chance to espace a plateau rather than exploring it: " << _options.percent_chance_escape_plateau << "%\n"
			          << _options.number_variables_to_reset << " variables are reset when " << _options.reset_threshold << " variables are frozen\n";
			if( _options.restart_threshold > 0 )
				std::cout << "Do a restart each time " << _options.restart_threshold << " resets are performed\n";
			else
				std::cout << "Never perform restarts\n";
			std::cout << "############" << "\n";

			// Print solution
			std::cout << _options.print->print_candidate( _model.variables ).str();

			std::cout << "\n";

			if( !is_optimization )
				std::cout << "SATISFACTION run" << "\n";
			else
			{
				std::cout << "OPTIMIZATION run with objective " << _model.objective->get_name() << "\n";
				if( _model.objective->is_maximization() )
					std::cout << _model.objective->get_name() << " must be maximized.\n";
				else
					std::cout << _model.objective->get_name() << " must be minimized.\n";					
			}

			std::cout << "Permutation problem: " << std::boolalpha << _model.permutation_problem << "\n"
			          << "Time budget: " << timeout << "us (= " << timeout/1000 << "ms, " << timeout/1000000 << "s)\n"
			          << "Search time: " << chrono_search << "us (= " << chrono_search / 1000 << "ms, " << chrono_search / 1000000 << "s)\n"
			          << "Wall-clock time (full call): " << chrono_full_computation << "us (= " << chrono_full_computation/1000 << "ms, " << chrono_full_computation/1000000 << "s)\n"
			          << "Satisfaction error: " << _best_sat_error << "\n"
			          << "Number of search iterations: " << _search_iterations << "\n"
			          << "Number of local moves: " << _local_moves << " (including on plateau: " << _plateau_moves << ")\n"
			          << "Number of local minimum: " << _local_minimum << " (including on plateau: " << _plateau_local_minimum << ")\n"
			          << "Number of resets: " << _resets << "\n"
			          << "Number of restarts: " << _restarts << "\n";

			if( _options.parallel_runs )
				std::cout << "Total number of search iterations: " << _search_iterations_total << "\n"
				          << "Total number of local moves: " << _local_moves_total << " (including on plateau: " << _plateau_moves_total << ")\n"
				          << "Total number of local minimum: " << _local_minimum_total << " (including on plateau: " << _plateau_local_minimum_total << ")\n"
				          << "Total number of resets: " << _resets_total << "\n"
				          << "Total number of restarts: " << _restarts_total << "\n";

			if( is_optimization )
				std::cout << "\nOptimization cost: " << _best_opt_cost << "\n";

			// If post-processing takes more than 1 microsecond, print details about it on the screen
			// This is to avoid printing something with empty post-processing, taking usually less than 0.1 microsecond (tested on a Core i9 9900)
			if( timer_postprocess.count() > 1 )
				std::cout << "Optimization Cost BEFORE post-processing: " << _cost_before_postprocess << "\n"
				          << "Optimization post-processing time: " << timer_postprocess.count() << "us (= " << timer_postprocess.count()/1000 << "ms, " << timer_postprocess.count()/1000000 << "s)\n"; 

			std::cout << "\n";
#endif

			return solution_found;
		}

		/*!
		 * Call Solver::solve with default options.
		 *
		 * \param final_cost a reference to a double to get the error of the best candidate or
		 * solution for satisfaction problems, or the objective function value of the best solution
		 * for optimization problems (or the cost of the best candidate if no solution has been
		 * found). For satisfaction problems, a cost of zero means a solution has been found.
		 * \param final_solution a reference to a vector of integers, to get values of the best
		 *  candidate or solution found.
		 * \param timeout a double for the time budget allowed to the solver to find a solution,
		 * in microseconds.
		 * \return True if and only if a solution has been found.
		 */
		bool solve( double& final_cost, std::vector<int>& final_solution, double timeout )
		{
			Options options;
			return solve( final_cost, final_solution, timeout, options );
		}

		/*!
		 * Call Solver::solve with a chrono literal timeout in microseconds.
		 *
		 * Users should favor this Solver::solve method if they need to give the solver
		 * user-defined options.
		 *
		 * \param final_cost a reference to a double to get the error of the best candidate or
		 * solution for satisfaction problems, or the objective function value of the best solution
		 * for optimization problems (or the cost of the best candidate if no solution has been
		 * found). For satisfaction problems, a cost of zero means a solution has been found.
		 * \param final_solution a reference to a vector of integers, to get values of the best
		 *  candidate or solution found.
		 * \param timeout a std::chrono::microseconds for the time budget allowed to the solver
		 * to find a solution. Higher std::chrono durations (such as milliseconds, seconds, etc)
		 * would be automatically converted into microseconds.
		 * \param options a reference to an Options object containing options such as parallel runs,
		 * a solution printer, if the solver must start with a custom variable assignment,
		 * parameter tuning, etc.
		 * \return True if and only if a solution has been found.
		 */
		bool solve( double& final_cost, std::vector<int>& final_solution, std::chrono::microseconds timeout, Options& options )
		{
			return solve( final_cost, final_solution, timeout.count(), options );
		}

		/*!
		 * Call Solver::solve with a chrono literal timeout in microseconds and default options.
		 *
		 * Users should favor this Solver::solve method if they want default options.
		 *
		 * \param final_cost a reference to a double to get the error of the best candidate or
		 * solution for satisfaction problems, or the objective function value of the best solution
		 * for optimization problems (or the cost of the best candidate if no solution has been
		 * found). For satisfaction problems, a cost of zero means a solution has been found.
		 * \param final_solution a reference to a vector of integers, to get values of the best
		 *  candidate or solution found.
		 * \param timeout a std::chrono::microseconds for the time budget allowed to the solver
		 * to find a solution. Higher std::chrono durations (such as milliseconds, seconds, etc)
		 * would be automatically converted into microseconds.
		 * \return True if and only if a solution has been found.
		 */
		bool solve( double& final_cost, std::vector<int>& final_solution, std::chrono::microseconds timeout )
		{
			Options options;
			return solve( final_cost, final_solution, timeout, options );
		}

		inline std::vector<Variable> get_variables() { return _model.variables; }
	};
}
