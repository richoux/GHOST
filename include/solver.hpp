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
#include "options.hpp"
#include "search_unit.hpp"

#if defined ANTIDOTE_SEARCH
#define ANTIDOTE_VARIABLE
#define ANTIDOTE_VALUE
#endif

#if defined GHOST_TRACE_PARALLEL
#define GHOST_TRACE
// possible with g++11 but not before
// #include <syncstream>
// #define COUT std::osyncstream(std::cout)
#include <fstream>
#include <sstream>
#define COUT _log_trace
#else
#define COUT std::cout
#endif

namespace ghost
{	
	//! Solver is the class coding the solver itself.
	/*!
	 * To solve a problem instance, you must instanciate a Solver object, then run Solver::solve.
	 *
	 * Solver constructors need a vector of Variable, a vector of shared pointers on Constraint objects, an optional 
	 * shared pointer on an Objective object (the solver will create a special empty Objective object is none is given), 
	 * and finally an optionnal boolean to indicate if the problem has been modeled as a permutation problem (false by default).
	 *
	 * A permutation problem is a problem where all variables start with different values, and only swapping values is allowed.
	 * This is typically the case for scheduling problems, for instance: you want to do A first, then B second, C third, and so on. 
	 * The solution of the problem must assign a unique value for each variable. Try as much as possible to model your problems as 
	 * permutation problems, since it should greatly speed-up the search of solutions.
	 *
	 * \sa Variable, Constraint, Objective
	 */
	template<typename FactoryModelType> class Solver final
	{
		Model _model;
		FactoryModelType _factory_model; //!< Factory building the model
		
		int _number_variables; //!< Size of the vector of variables.

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

		bool _is_permutation_problem;
		Options _options; //!< Options for the solver (see the struct Options).
		
	public:
		//! Solver's regular constructor
		/*!
		 * \param model A shared pointer to the Model object.
		 * \param permutation_problem A boolean indicating if we work on a permutation problem. False by default.
		 */
		Solver( const FactoryModelType& factory_model,
		        bool permutation_problem = false )
			: _factory_model( factory_model ),
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
			  _plateau_local_minimum( 0 ),
			  _is_permutation_problem( permutation_problem )
		{	}

		//! Solver's main function, to solve the given CSP/COP/CFN.
		/*!
		 * This function is the heart of GHOST's solver: it will try to find a solution within a limited time. If it finds such a solution, 
		 * the function outputs the value true.\n
		 * Here how it works: if at least one solution is found, at the end of the computation, it will write in the two first
		 * parameters finalCost and finalSolution the cost of the best solution found and the value of each variable.\n
		 * For a satisfaction problem (without any objective function), the cost of a solution is the sum of the cost of each
		 * problem constraint (computated by Constraint::required_error). For an optimization problem, the cost is the value outputed
		 * by Objective::required_cost.\n
		 * For both, the lower value the better: A satisfaction cost of 0 means we have a solution to a satisfaction problem (ie, 
		 * all constraints are satisfied). An optimization cost should be as low as possible: GHOST is handling minimization problems 
		 * only. If you have a maximization problem (you are looking to the highest possible value of your objective function), look 
		 * at the Objective documentation to see how to easily convert your problem into a minimization problem.
		 *
		 * The two last parameters sat_timeout and opt_timeout are fundamental: sat_timeout is mandatory, opt_timeout is optional: 
		 * if not given, its value will be fixed to sat_timeout * 10.\n
		 * sat_timeout is the timeout in microseconds you give to GHOST to find a solution to the problem, ie, finding a value for 
		 * each variable such that each constraint of the problem is satisfied. For a satisfaction problem, this is the timeout within
		 * GHOST must output a solution.\n
		 * opt_timeout is only useful for optimization problems. Once GHOST finds a solution within sat_timeout, it saves it and try to find 
		 * other solutions leading to better (ie, smaller) values of the objective function. Then it restarts a fresh satisfaction search, 
		 * with once again sat_timeout as a timeout to find a solution. It will repeat this operation until opt_timeout is reached.
		 *
		 * Thus for instance, if you set sat_timeout to 20μs and opt_timeout to 60μs (or bit more like 65μs, see why below), you let GHOST 
		 * the time to run 3 satisfaction runs within a global runtime of 60μs (or 65μs), like illustrated below (with milliseconds instead of microseconds).
		 *
		 * \image html architecture.png "x and y milliseconds correspond respectively to sat_timeout and opt_timeout"
		 * \image latex architecture.png "x and y milliseconds correspond respectively to sat_timeout and opt_timeout"
		 *
		 * It is possible it returns no solutions after timeout; in that case Solver::solve returns false. If it is often the case, this is a 
		 * strong evidence the satisfaction timeout is too low, and the solver does not have time to find at least one solution. Thus, this is 
		 * the only parameter you may have to tweak in GHOST.
		 *
		 * The illustration above shows satisfaction and optimization post-processes. The first one is triggered each time the solver found a solution. 
		 * If the user overloads Objective::expert_postprocess_satisfaction, he or she must be sure that his or her function runs very quickly, otherwise
		 * it may slow down the whole optimization process and may limit the number of solutions found by the solver. Optimization post-process runtime 
		 * is not taken into account within opt_timeout, so the real GHOST runtime for optimization problems will be roughly equals to opt_timeout + 
		 * optimization post-process runtime.
		 *
		 * \param final_cost A reference to the double of the sum of constraints cost for satisfaction problems, 
		 * or the value of the objective function for optimization problems. For satisfaction problems, a cost of zero means a solution has been found.
		 * \param finalSolution The configuration of the best solution found, ie, a reference to the vector of assignements of each variable.
		 * \param sat_timeout The satisfaction timeout in microseconds.
		 * \param opt_timeout The optimization timeout in microseconds (optionnal, equals to 10 times sat_timeout is not set).
		 * \param options A reference to an Options object containing options such as a solution printer, Booleans indicating if the solver must start with a custom variable assignment, etc.
		 * \return True iff a solution has been found.
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

			std::chrono::duration<double,std::micro> timer_postprocess_sat( 0 );
			std::chrono::duration<double,std::micro> timer_postprocess_opt( 0 );

			/*****************
			* Initialization *
			******************/
			// Only to get the number of variables
			_factory_model.declare_variables();
			_number_variables = _factory_model.get_number_variables();

			_options = options;
			
			if( _options.tabu_time_local_min == -1 )
				_options.tabu_time_local_min = std::max( std::min( 5, static_cast<int>( _number_variables ) - 1 ), static_cast<int>( std::ceil( _number_variables / 5 ) ) ) + 1;
			  //_options.tabu_time_local_min = std::max( 2, _tabu_threshold ) );

			if( _options.tabu_time_selected == -1 )
				_options.tabu_time_selected = 0;

			if( _options.reset_threshold == -1 )
			{
#if defined ANTIDOTE_VARIABLE
			  _options.reset_threshold = 2 * static_cast<int>( std::ceil( std::sqrt( _number_variables ) ) );
#else
				_options.reset_threshold = _options.tabu_time_local_min;
#endif
			}
			
// #if defined ANTIDOTE_VARIABLE
// 			_options.reset_threshold = static_cast<int>( std::ceil( 1.5 * _options.reset_threshold ) );
// #endif
			
			if( _options.restart_threshold == -1 ) 
				_options.restart_threshold = _number_variables;

			if( _options.percent_to_reset == -1 )
				_options.percent_to_reset = std::max( 2, static_cast<int>( std::ceil( _number_variables * 0.1 ) ) ); // 10%

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
				SearchUnit search_unit( _factory_model.make_model(),
				                        _is_permutation_problem,
				                        _options );

				is_optimization = search_unit.is_optimization();
				std::future<bool> unit_future = search_unit.solution_found.get_future();
				
				start_search = std::chrono::steady_clock::now();
				search_unit.search( timeout );
				elapsed_time = std::chrono::steady_clock::now() - start_search;
				chrono_search = elapsed_time.count();

				solution_found = unit_future.get();
				final_solution = search_unit.final_solution;
				_best_sat_error = search_unit.best_sat_error;
				_best_opt_cost = search_unit.best_opt_cost;
				_restarts = search_unit.restarts;
				_resets = search_unit.resets;
				_local_moves = search_unit.local_moves;
				_search_iterations = search_unit.search_iterations;
				_local_minimum = search_unit.local_minimum;
				_plateau_moves = search_unit.plateau_moves;
				_plateau_local_minimum = search_unit.plateau_local_minimum;

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
					units.emplace_back( _factory_model.make_model(),
					                    _is_permutation_problem,
					                    _options );
				}

				is_optimization = units[0].is_optimization();
				
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
									if( _best_opt_cost > units.at( thread_number ).best_opt_cost )
									{
										_best_opt_cost = units.at( thread_number ).best_opt_cost;
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
										
					_restarts_total += units.at(i).restarts;
					_resets_total += units.at(i).resets;
					_local_moves_total += units.at(i).local_moves;
					_search_iterations_total += units.at(i).search_iterations;
					_local_minimum_total += units.at(i).local_minimum;
					_plateau_moves_total += units.at(i).plateau_moves;
					_plateau_local_minimum_total += units.at(i).plateau_local_minimum;
				}

				// ..then the most important: the best solution found so far.
				if( solution_found )
				{
#if defined GHOST_TRACE
					std::cout << "Parallel run, thread number " << winning_thread << " has found a solution.\n";
#endif
					final_solution = units.at( winning_thread ).final_solution;
					_best_sat_error = units.at( winning_thread ).best_sat_error;
					_best_opt_cost = units.at( winning_thread ).best_opt_cost;

					_restarts = units.at( winning_thread ).restarts;
					_resets = units.at( winning_thread ).resets;
					_local_moves = units.at( winning_thread ).local_moves;
					_search_iterations = units.at( winning_thread ).search_iterations;
					_local_minimum = units.at( winning_thread ).local_minimum;
					_plateau_moves = units.at( winning_thread ).plateau_moves;
					_plateau_local_minimum = units.at( winning_thread ).plateau_local_minimum;
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
						if( _best_sat_error > units.at( i ).best_sat_error )
						{
							best_non_solution = i;
							_best_sat_error = units.at( i ).best_sat_error;
						}
						if( is_optimization && _best_sat_error == 0.0 )
							if( units.at( i ).best_sat_error == 0.0 && _best_opt_cost > units.at( i ).best_opt_cost )
							{
								best_non_solution = i;
								_best_opt_cost = units.at( i ).best_opt_cost;
							}
					}

					final_solution = units.at( best_non_solution ).final_solution;
					_restarts = units.at( best_non_solution ).restarts;
					_resets = units.at( best_non_solution ).resets;
					_local_moves = units.at( best_non_solution ).local_moves;
					_search_iterations = units.at( best_non_solution ).search_iterations;
					_local_minimum = units.at( best_non_solution ).local_minimum;
					_plateau_moves = units.at( best_non_solution ).plateau_moves;
					_plateau_local_minimum = units.at( best_non_solution ).plateau_local_minimum;
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
				_model.objective->postprocess_optimization( _best_opt_cost, final_solution );
				timer_postprocess_opt = std::chrono::steady_clock::now() - start_postprocess;
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

			// Set the variables to the best solution values.
			// Useful if the user prefer to directly use the vector of Variables
			// to manipulate and exploit the solution.
			for( int variable_id = 0 ; variable_id < _number_variables; ++variable_id )
				_model.variables[ variable_id ].set_value( final_solution[ variable_id ] );

			elapsed_time = std::chrono::steady_clock::now() - start_wall_clock;
			chrono_full_computation = elapsed_time.count();
			
#if defined GHOST_DEBUG || defined GHOST_TRACE || defined GHOST_BENCH
			std::cout << "@@@@@@@@@@@@" << "\n"
			          << "Variable heuristic: "
#if not defined ANTIDOTE_VARIABLE // ADAPTIVE_SEARCH
			          << "Adaptive Search\n"
#else // Antidote Search
			          << "Antidote Search\n"
#endif
			          << "Value heuristic: "
#if not defined ANTIDOTE_VALUE // ADAPTIVE_SEARCH
			          << "Adaptive Search\n"
#else // Antidote Search
			          << "Antidote Search\n"
#endif
			          << "Started from a custom variables assignment: " << std::boolalpha << _options.custom_starting_point << "\n"
			          << "Search resumed from a previous run: " << std::boolalpha << _options.resume_search << "\n"
			          << "Parallel search: " << std::boolalpha << _options.parallel_runs << "\n"
			          << "Number of threads (not used if no parallel search): " << _options.number_threads << "\n"
			          << "Number of variable assignments samplings at start (if custom start and resume are set to false): " << _options.number_start_samplings << "\n"
			          << "Variables of local minimum are frozen for: " << _options.tabu_time_local_min << " local moves\n"
			          << "Selected variables are frozen for: " << _options.tabu_time_selected << " local moves\n"
			          << _options.percent_to_reset << " variables are reset when " << _options.reset_threshold << " variables are frozen\n"
			          << "Do a restart each time " << _options.restart_threshold << " resets are performed\n"
			          << "############" << "\n";

			// Print solution
			std::cout << _options.print->print_candidate( _model.variables ).str();

			std::cout << "\n";
			
			if( !is_optimization )
				std::cout << "SATISFACTION run" << "\n";
			else
				std::cout << "OPTIMIZATION run with objective " << _model.objective->get_name() << "\n";

			std::cout << "Permutation problem: " << std::boolalpha << _is_permutation_problem << "\n"
			          << "Time budget: " << timeout / 1000 << "ms\n"
			          << "Search time: " << chrono_search / 1000 << "ms\n"
			          << "Wall-clock time (full program): " << chrono_full_computation / 1000 << "ms\n"
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
				std::cout << "\nOptimization cost: " << _best_opt_cost << "\n"
				          << "Opt Cost BEFORE post-processing: " << _cost_before_postprocess << "\n";
  
			// if( timer_postprocess_sat.count() > 0 )
			// 	std::cout << "Satisfaction post-processing time: " << timer_postprocess_sat.count() / 1000 << "\n"; 

			if( timer_postprocess_opt.count() > 0 )
				std::cout << "Optimization post-processing time: " << timer_postprocess_opt.count() / 1000 << "\n"; 

			std::cout << "\n";
#endif
          
			return solution_found;
		}

		//! Call Solver::solve with default options.
		bool solve( double& final_cost, std::vector<int>& final_solution, double timeout )
		{
			Options options;
			return solve( final_cost, final_solution, timeout, options );
		}

		//! Call Solver::solve with a chrono literal timeout in seconds
		bool solve( double& final_cost, std::vector<int>& final_solution, std::chrono::seconds timeout, Options& options )
		{
			return solve( final_cost, final_solution, std::chrono::duration_cast<std::chrono::microseconds>(timeout).count(), options );
		}

		//! Call Solver::solve with a chrono literal timeout in seconds and default options.
		bool solve( double& final_cost, std::vector<int>& final_solution, std::chrono::seconds timeout )
		{
			Options options;
			return solve( final_cost, final_solution, timeout, options );
		}

		//! Call Solver::solve with a chrono literal timeout in milliseconds
		bool solve( double& final_cost, std::vector<int>& final_solution, std::chrono::milliseconds timeout, Options& options )
		{
			return solve( final_cost, final_solution, std::chrono::duration_cast<std::chrono::microseconds>(timeout).count(), options );
		}

		//! Call Solver::solve with a chrono literal timeout in milliseconds and default options.
		bool solve( double& final_cost, std::vector<int>& final_solution, std::chrono::milliseconds timeout )
		{
			Options options;
			return solve( final_cost, final_solution, timeout, options );
		}

		//! Call Solver::solve with a chrono literal timeout in microseconds
		bool solve( double& final_cost, std::vector<int>& final_solution, std::chrono::microseconds timeout, Options& options )
		{
			return solve( final_cost, final_solution, timeout.count(), options );
		}

		//! Call Solver::solve with a chrono literal timeout in microseconds and default options.
		bool solve( double& final_cost, std::vector<int>& final_solution, std::chrono::microseconds timeout )
		{
			Options options;
			return solve( final_cost, final_solution, timeout, options );
		}

		//! Call Solver::solve with a chrono literal timeout in nanoseconds
		bool solve( double& final_cost, std::vector<int>& final_solution, std::chrono::nanoseconds timeout, Options& options )
		{
			return solve( final_cost, final_solution, std::chrono::duration_cast<std::chrono::microseconds>(timeout).count(), options );
		}

		//! Call Solver::solve with a chrono literal timeout in nanoseconds and default options.
		bool solve( double& final_cost, std::vector<int>& final_solution, std::chrono::nanoseconds timeout )
		{
			Options options;
			return solve( final_cost, final_solution, timeout, options );
		}

		inline std::vector<Variable> get_variables() { return _model.variables; }
	};
}
