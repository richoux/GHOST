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

#include <cassert>
#include <cmath>
#include <limits>
#include <random>
#include <algorithm>
#include <vector>
#include <variant>
#include <chrono>
#include <memory>
#include <iterator>
#include <thread>
#include <future>

#include "variable.hpp"
#include "constraint.hpp"
#include "objective.hpp"
#include "misc/randutils.hpp"
#include "misc/print.hpp"

namespace ghost
{
	//! Options is a structure containing all optional arguments for Solver::solve.
	struct Options
	{
		bool custom_starting_point; //!< To force starting the search on a custom variables assignment.
		bool resume_search; //!< Allowing stop-and-resume computation.
		bool parallel_runs; //<! To enable parallel runs of the solver. Using all avaiable cores (including hyper-threaded cores) if number_threads is not specified.
		unsigned int number_threads; //<! Number of threads the solver will use for the search.
		std::shared_ptr<Print> print; //!< Allowing custom solution print (by derivating a class from ghost::Print)
		int tabu_time_local_min; //!< Number of local moves a variable of a local minimum is marked tabu.
		int tabu_time_selected; //!< Number of local moves a selected variable is marked tabu.
		int reset_threshold; //!< Number of variables marked as tabu required to trigger a reset.
		int restart_threshold; //!< Trigger a resart every 'restart_threshold' reset.
		int percent_to_reset; //<! Percentage of variables to randomly change the value at each reset.
		int number_start_samplings; //!< Number of variable assignments the solver randomly draw, if custom_starting_point and resume_search are false.
		
		Options()
			: custom_starting_point( false ),
			  resume_search( false ),
			  parallel_runs( false ),
			  number_threads( std::max( (unsigned)1, std::jthread::hardware_concurrency() ) ), // std::jthread::hardware_concurrency() returns 0 if it is not able to detect the number of threads
			  print( std::make_shared<Print>() ),
			  tabu_time_local_min( -1 ),
			  tabu_time_selected( -1 ),
			  reset_threshold( -1 ),
			  restart_threshold( -1 ),
			  percent_to_reset( -1 ),
			  number_start_samplings( 10 )
		{ }

		~Options() = default;
		Options( const Options& other )
			: custom_starting_point( other.custom_starting_point ),
			  resume_search( other.resume_search ),
			  parallel_runs( other.parallel_runs ),
			  number_threads( other.number_threads ),
			  print( other.print ),
			  tabu_time_local_min( other.tabu_time_local_min ),
			  tabu_time_selected( other.tabu_time_selected ),
			  reset_threshold( other.reset_threshold ),
			  restart_threshold( other.restart_threshold ),
			  percent_to_reset( other.percent_to_reset ),
			  number_start_samplings( other.number_start_samplings )
		{ }
		
		Options( Options&& other )
			: custom_starting_point( other.custom_starting_point ),
			  resume_search( other.resume_search ),
			  parallel_runs( other.parallel_runs ),
			  number_threads( other.number_threads ),
			  print( std::move( other.print ) ),
			  tabu_time_local_min( other.tabu_time_local_min ),
			  tabu_time_selected( other.tabu_time_selected ),
			  reset_threshold( other.reset_threshold ),
			  restart_threshold( other.restart_threshold ),
			  percent_to_reset( other.percent_to_reset ),
			  number_start_samplings( other.number_start_samplings )
		{	}
		
		Options& operator=( Options other )
		{
			if( this != &other )
			{
				custom_starting_point = other.custom_starting_point;
				resume_search = other.resume_search;
				parallel_runs = other.parallel_runs;
				number_threads = other.number_threads;
				std::swap( print, other.print );
				tabu_time_local_min = other.tabu_time_local_min;
				tabu_time_selected = other.tabu_time_selected;
				reset_threshold = other.reset_threshold;
				restart_threshold = other.restart_threshold;
				percent_to_reset = other.percent_to_reset;
				number_start_samplings = other.number_start_samplings;
			}

			return *this;
		}
	};

	template<typename ObjectiveType, typename ... ConstraintType>
	class SearchUnit
	{
		std::promise<void> _stop_search_signal;
		std::future<void> _stop_search_check;

#if defined(GHOST_TRACE)
		void print_errors()
		{
			std::cout << "Constraint errors:\n";
			for( unsigned int constraint_id = 0; constraint_id < number_constraints; ++constraint_id )
			{
				std::cout << "Constraint num. " << constraint_id << "=" << get_constraint_error( constraint_id ) << ": ";
				bool mark_comma = false;
				for( const auto variable_id : std::visit( [&](Constraint& ctr){ return ctr.get_variable_ids(); }, constraints[ constraint_id ] ) )
				{
					if( mark_comma )
						std::cout << ", ";
					else
						mark_comma = true;
					std::cout << "v[" << variable_id << "]=" << variables[variable_id].get_value();
				}
				std::cout << "\n";
			}

			std::cout << "\nVariable errors:\n";

			for( unsigned int variable_id = 0 ; variable_id < number_variables ; ++variable_id )
			{
				std::cout << "v[" << variable_id << "]=" << error_variables[variable_id] << ": ";
				bool mark_plus = false;
				for( unsigned int constraint_id : matrix_var_ctr.at( variable_id ) )
				{
					if( mark_plus )
						std::cout << " + ";
					else
						mark_plus = true;
					std::cout << "c[" << constraint_id << "]=" << get_constraint_error( constraint_id );
				}
				std::cout << "\n";				
			}
			
			std::cout << "\n";
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
			std::vector<int> best_values( number_variables, 0 );
			
			// To avoid weird samplings numbers like 0 or -1
			samplings = std::max( 1, samplings );
			int loops = 0;
			
			// In case we directly start with a solution, or something closed to be a solution
			do
			{
				if( loops > 0)
				{
					if( is_permutation_problem )
						random_permutations();
					else
						monte_carlo_sampling();
				}
				
				current_sat_error = 0.0;
				for( unsigned int constraint_id = 0 ; constraint_id < number_constraints ; ++constraint_id )
				{
					for( const auto variable_id : std::visit( [&](Constraint& ctr){ return ctr.get_variable_ids(); }, constraints[ constraint_id ] ) )
						call_update_variable( constraint_id, variable_id, variables[ variable_id ].get_value() );
					current_sat_error += call_error( constraint_id );
				}
				
				if( best_sat_error_so_far > current_sat_error )
				{
					best_sat_error_so_far = current_sat_error;
					
					if( best_sat_error > best_sat_error_so_far )
					{
#if defined GHOST_TRACE
						std::cout << "Better starting configuration found. Previous error: " << best_sat_error << ", now: " << best_sat_error_so_far << "\n";
#endif
						best_sat_error = best_sat_error_so_far;
					}
					for( int i = 0 ; i < number_variables ; ++i )
						best_values[ i ] = variables[ i ].get_value();
				}
				++loops;
			} while( loops < samplings && current_sat_error > 0.0 );
			
			for( unsigned int variable_id = 0 ; variable_id < number_variables ; ++variable_id )
				variables[ variable_id ].set_value( best_values[ variable_id ] );
		}

		//! Sample an configuration
		void monte_carlo_sampling( int nb_var = -1 )
		{
			if( nb_var == -1 )
				nb_var = number_variables;

			std::vector<int> variables_index( number_variables );
			std::iota( variables_index.begin(), variables_index.end(), 0 );
			rng.shuffle( variables_index );
			
			for( int i = 0 ; i < nb_var ; ++i )
				variables[ variables_index[ i ] ].pick_random_value();
		}

		//! Sample an configuration for permutation problems
		void random_permutations( int nb_var = -1 )
		{
			if( nb_var == -1 )
			{
				for( unsigned int i = 0 ; i < variables.size() - 1 ; ++i )
					for( unsigned int j = i + 1 ; j < variables.size() ; ++j )
					{
						// 50% to do a swap for each couple (var_i, var_j)
						if( rng.uniform( 0, 1 ) == 0
						    && i != j
						    && variables[ i ].get_value() != variables[ j ].get_value()
						    && std::find( variables[ j ].get_full_domain().begin(),
						                  variables[ j ].get_full_domain().end(),
						                  variables[ i ].get_value() ) != variables[ j ].get_full_domain().end()
						    && std::find( variables[ i ].get_full_domain().begin(),
						                  variables[ i ].get_full_domain().end(),
						                  variables[ j ].get_value() ) != variables[ i ].get_full_domain().end() )
						{
							std::swap( variables[i]._current_value, variables[j]._current_value );
						}
					}
			}
			else
			{
				std::vector<int> variables_index_A( number_variables );
				std::vector<int> variables_index_B( number_variables );
				std::iota( variables_index_A.begin(), variables_index_A.end(), 0 );
				std::iota( variables_index_B.begin(), variables_index_B.end(), 0 );
				rng.shuffle( variables_index_A );
				rng.shuffle( variables_index_B );

				for( int i = 0 ; i < nb_var ; ++i )
					if( variables_index_A[i] != variables_index_B[i]
					    && variables[ variables_index_A[i] ].get_value() != variables[ variables_index_B[i] ].get_value()
					    && std::find( variables[ variables_index_B[i] ].get_full_domain().begin(),
					                  variables[ variables_index_B[i] ].get_full_domain().end(),
					                  variables[ variables_index_A[i] ].get_value() ) != variables[ variables_index_B[i] ].get_full_domain().end()
					    && std::find( variables[ variables_index_A[i] ].get_full_domain().begin(),
					                  variables[ variables_index_A[i] ].get_full_domain().end(),
					                  variables[ variables_index_B[i] ].get_value() ) != variables[ variables_index_A[i] ].get_full_domain().end() )
						std::swap( variables[ variables_index_A[i] ]._current_value, variables[ variables_index_B[i] ]._current_value );
			}
		}

		void initialize_variable_values()
		{
			if( options.custom_starting_point || options.resume_search )
			{
				if( options.resume_search )
					options.resume_search = false;
				for( int i = 0 ; i < number_variables ; ++i )
					variables[i].set_value( variables_at_start[i].get_value() );
							
				std::transform( variables.begin(),
				                variables.end(),
				                final_solution.begin(),
				                [&](auto& var){ return var.get_value(); } );
			}
			else
				set_initial_configuration( options.number_start_samplings );			
		}

		void initialize_data_structures()
		{
			must_compute_worst_variables_list = true;
			std::fill( tabu_list.begin(), tabu_list.end(), 0 ); 

			// Reset constraints costs
			for( unsigned int constraint_id = 0; constraint_id < number_constraints; ++constraint_id )
				std::visit( [&](Constraint& ctr){ ctr._current_error = 0.0; }, constraints[ constraint_id ] );
			
			// Send the current variables assignment to the constraints.
			for( unsigned int variable_id = 0 ; variable_id < number_variables ; ++variable_id )
			{
				for( const unsigned int constraint_id : matrix_var_ctr.at( variable_id ) )
					call_update_variable( constraint_id, variable_id, variables[ variable_id ].get_value() );
				
				objective->update_variable( variable_id, variables[ variable_id ].get_value() );
			}
			
			// (Re)compute constraint error and get the total current satisfaction error
			current_sat_error = compute_constraints_errors();
			// (Re)compute the current optimization cost
			if( is_optimization )
			{
				if( current_sat_error == 0 ) [[unlikely]]
					current_opt_cost = objective->cost();
				else
					current_opt_cost = std::numeric_limits<double>::max();
			}
			
			// Reset variable costs
			std::fill( error_variables.begin(), error_variables.end(), 0.0 ); 
			// Recompute them
			compute_variables_errors();
		}
				
		void reset()
		{
			++resets;
			
			// if we reach the restart threshold, do a restart instead of a reset
			if( resets % options.restart_threshold == 0 )
			{
				++restarts;

				// Start from a given starting configuration, or a random one.
				initialize_variable_values();
				
#if defined(GHOST_TRACE)
				std::cout << "Number of restarts performed so far: " << restarts << "\n";
				options.print->print_candidate( variables );
				std::cout << "\n";
#endif
			}
			else // real reset
			{
				if( is_permutation_problem )
					random_permutations( options.percent_to_reset );
				else
					monte_carlo_sampling( options.percent_to_reset );
				
#if defined(GHOST_TRACE)
				std::cout << "Number of resets performed so far: " << resets << "\n";
				options.print->print_candidate( variables );
				std::cout << "\n";
#endif
			}
			
			initialize_data_structures();
		}

		inline double call_error( unsigned int constraint_id )
		{
			return std::visit( [&](Constraint& ctr){ return ctr.error(); }, constraints[ constraint_id ] );
		}

		inline void call_update_variable( unsigned int constraint_id, unsigned int variable_id, int value )
		{
			std::visit( [&](Constraint& ctr){ ctr.update_variable( variable_id, value ); }, constraints[ constraint_id ] );
		}

		inline double get_constraint_error( unsigned int constraint_id )
		{
			return std::visit( [&](Constraint& ctr){ return ctr._current_error; }, constraints[ constraint_id ] );
		}
		
		//! To compute the vector of variables which are principal culprits for not satisfying the problem
		void compute_worst_variables()
		{
#if defined(GHOST_TRACE)
			print_errors();
#endif
			if( std::count_if( tabu_list.begin(),
			                   tabu_list.end(),
			                   [&](int end_tabu){ return end_tabu > local_moves; } ) >= options.reset_threshold )
				worst_variables_list.clear();
			else
			{
				double worst_variable_cost = -1;
				
				for( unsigned int variable_id = 0; variable_id < number_variables; ++variable_id )
					if( worst_variable_cost <= error_variables[ variable_id ] && tabu_list[ variable_id ] <= local_moves )
					{
						if( worst_variable_cost < error_variables[ variable_id ] )
						{
							worst_variables_list.clear();
							worst_variables_list.push_back( variable_id );
							worst_variable_cost = error_variables[ variable_id ];
						}
						else
							if( worst_variable_cost == error_variables[ variable_id ] )
								worst_variables_list.push_back( variable_id );
					}
			}
		}

		//! Compute the cost of each constraints
		double compute_constraints_errors()
		{
			double satisfaction_error = 0.0;
			double error;

			for( unsigned int constraint_id = 0 ; constraint_id < number_constraints ; ++constraint_id )
			{
				error = call_error( constraint_id );
				std::visit( [&](Constraint& ctr){ ctr._current_error = error; }, constraints[ constraint_id ] );

				satisfaction_error += error;
			}

			return satisfaction_error;
		}

		//! Compute the variable cost of each variables and fill up _error_variables 
		void compute_variables_errors()
		{
			for( unsigned int variable_id = 0; variable_id < number_variables; ++variable_id )
				for( const unsigned int constraint_id : matrix_var_ctr.at( variable_id ) )
					error_variables[ variable_id ] += get_constraint_error( constraint_id );
		}

		void update_errors( unsigned int variable_to_change, int new_value, const std::map< int, std::vector<double>>& delta_errors )
		{
			int delta_index = 0;
			if( !is_permutation_problem )
			{
				for( const unsigned int constraint_id : matrix_var_ctr.at( variable_to_change ) )
				{
					auto delta = delta_errors.at( new_value )[ delta_index++ ];
					std::visit( [&](Constraint& ctr){ ctr._current_error += delta; }, constraints[ constraint_id ] );
					for( const unsigned int variable_id : std::visit( [&](Constraint& ctr){ return ctr.get_variable_ids(); }, constraints[ constraint_id ] ) )
						error_variables[ variable_id ] += delta;
					
					call_update_variable( constraint_id, variable_to_change, new_value );						
				}
			}
			else
			{
				std::vector<bool> constraint_checked( constraints.size(), false );
				int current_value = variables[ variable_to_change ].get_value();
				int next_value = variables[ new_value ].get_value();
				
				for( const unsigned int constraint_id : matrix_var_ctr.at( variable_to_change ) )
				{
					constraint_checked[ constraint_id ] = true;
					auto delta = delta_errors.at( new_value )[ delta_index++ ];
					std::visit( [&](Constraint& ctr){ ctr._current_error += delta; }, constraints[ constraint_id ] );
					for( const unsigned int variable_id : std::visit( [&](Constraint& ctr){ return ctr.get_variable_ids(); }, constraints[ constraint_id ] ) )
						error_variables[ variable_id ] += delta;
					
					call_update_variable( constraint_id, variable_to_change, current_value );
					if(	std::visit( [&](Constraint& ctr){ return ctr.has_variable( new_value ); }, constraints[ constraint_id ] ) )
						call_update_variable( constraint_id, new_value, next_value );						
				}
				
				for( const unsigned int constraint_id : matrix_var_ctr.at( new_value ) )
					if( !constraint_checked[ constraint_id ] )
					{
						auto delta = delta_errors.at( new_value )[ delta_index++ ];
						std::visit( [&](Constraint& ctr){ ctr._current_error += delta; }, constraints[ constraint_id ] );
						for( const unsigned int variable_id : std::visit( [&](Constraint& ctr){ return ctr.get_variable_ids(); }, constraints[ constraint_id ] ) )
							error_variables[ variable_id ] += delta;
						
						call_update_variable( constraint_id, new_value, next_value );						
					}
			}
		}

		// A. Local move (perform local move and update variables/constraints/objective function)
		void local_move( unsigned int variable_to_change, int new_value, int min_conflict, const std::map< int, std::vector<double>>& delta_errors )
		{
			++local_moves;
			current_sat_error += min_conflict;
			tabu_list[ variable_to_change ] = options.tabu_time_selected + local_moves;
			must_compute_worst_variables_list = true;
	
			if( is_permutation_problem )
			{
				int current_value = variables[ variable_to_change ].get_value();
				int next_value = variables[ new_value ].get_value();
				variables[ variable_to_change ].set_value( next_value );
				variables[ new_value ].set_value( current_value );

				if( is_optimization )
				{
					objective->update_variable( variable_to_change, next_value );
					objective->update_variable( new_value, current_value );
				}
			}
			else
			{
				variables[ variable_to_change ].set_value( new_value );
				
				if( is_optimization )
					objective->update_variable( variable_to_change, new_value );
			}

			update_errors( variable_to_change, new_value, delta_errors );
		}
		
		// B. Plateau management (local move on the plateau, but 10% of chance to escape it, mark the variables as tabu and loop to 2.)
		// Return true iff the solver escapes from the plateau.
		void plateau_management( unsigned int variable_to_change, int new_value, const std::map< int, std::vector<double>>& delta_errors )
		{
			if( rng.uniform(0, 100) <= 10 )
			{
				tabu_list[ variable_to_change ] = options.tabu_time_local_min + local_moves;
				must_compute_worst_variables_list = true;
				++plateau_local_minimum;
#if defined(GHOST_TRACE)
				std::cout << "Escape from plateau; variables marked as tabu.\n";
#endif
			}
			else
			{
				local_move( variable_to_change, new_value, 0, delta_errors );
				++plateau_moves;
			}
		}
		
		// C. local minimum management (if there are no other worst variables to try, mark the variables as tabu and loop to 2.
		//                              Otherwise try them first, but with 10% of chance, the solver marks the variables as tabu and loop to 2)
		void local_minimum_management( unsigned int variable_to_change, int new_value, bool no_other_variables_to_try )
		{
			if( no_other_variables_to_try || rng.uniform(0, 100) <= 10 )
			{
				tabu_list[ variable_to_change ] = options.tabu_time_local_min + local_moves;
				must_compute_worst_variables_list = true;
				++local_minimum;
			}
			else
			{
#if defined(GHOST_TRACE)
				std::cout << "Try other variables: not a local minimum yet.\n";
#endif
				must_compute_worst_variables_list = false;
			}
		}

	public:
		std::vector<Variable> variables; 
		std::vector<std::variant<ConstraintType ...>> constraints; 
		std::shared_ptr<ObjectiveType> objective;

		std::vector<Variable> variables_at_start; 
		randutils::mt19937_rng rng; 

		bool is_optimization; 

		unsigned int number_variables; 
		unsigned int number_constraints; 

		std::vector<int> final_solution;

		std::vector<std::vector<unsigned int> > matrix_var_ctr; 
		std::vector<int> tabu_list; 
		
		std::vector<unsigned int> worst_variables_list;
		bool must_compute_worst_variables_list;
		
		std::vector<double> error_variables;

		double best_sat_error; 
		double best_opt_cost; 
		double current_sat_error;
		double current_opt_cost;
		double cost_before_postprocess;

		// stats variables (however _local_moves is important for the solver logic)
		int restarts;
		int resets;
		int local_moves;
		int search_iterations;
		int local_minimum;
		int plateau_moves;
		int plateau_local_minimum;
		
		bool is_permutation_problem;
		std::promise<bool> solution_found;
		
		Options options;

		SearchUnit( const std::vector<Variable>& variables, 
		            const std::vector<std::variant<ConstraintType ...>>&	constraints,
		            std::shared_ptr<ObjectiveType> objective,
		            bool is_optimization,
		            const std::vector<std::vector<unsigned int>>& matrix_var_ctr,
		            bool permutation_problem,
		            const Options& options )
			: _stop_search_check( _stop_search_signal.get_future() ),
			  variables ( variables ), 
			  constraints ( constraints ),
			  objective ( make_shared<ObjectiveType>( *objective ) ),
			  variables_at_start( variables ),
			  is_optimization ( is_optimization ),
			  number_variables ( static_cast<unsigned int>( variables.size() ) ),
			  number_constraints ( static_cast<unsigned int>( constraints.size() ) ),
			  final_solution ( number_variables, 0),
			  matrix_var_ctr ( matrix_var_ctr ),
			  tabu_list ( std::vector<int>( number_variables, 0 ) ),
			  worst_variables_list ( std::vector<unsigned int>( number_variables, 0 ) ),
			  must_compute_worst_variables_list ( true ),
			  error_variables ( std::vector<double>( number_variables, 0.0 ) ),
			  best_sat_error ( std::numeric_limits<double>::max() ),
			  best_opt_cost ( std::numeric_limits<double>::max() ),
			  current_sat_error ( std::numeric_limits<double>::max() ),
			  current_opt_cost ( std::numeric_limits<double>::max() ),
			  cost_before_postprocess ( std::numeric_limits<double>::max() ),
			  restarts ( 0 ),
			  resets ( 0 ),
			  local_moves ( 0 ),
			  search_iterations ( 0 ),
			  local_minimum ( 0 ),
			  plateau_moves ( 0 ),
			  plateau_local_minimum ( 0 ),
			  is_permutation_problem ( permutation_problem ),
			  options ( options )
		{
#if defined(GHOST_TRACE)
			std::cout << "Creating a search unit\n";
#endif
		}
		
		SearchUnit( SearchUnit && unit )
			: _stop_search_signal( std::move( unit._stop_search_signal ) ),
			  _stop_search_check( std::move( unit._stop_search_check ) ),
			  variables ( unit.variables ), 
			  constraints ( unit.constraints ),
			  objective ( nullptr ),
			  variables_at_start ( unit.variables_at_start ),
			  is_optimization ( unit.is_optimization ),
			  number_variables ( unit.number_variables ),
			  number_constraints ( unit.number_constraints ),
			  final_solution ( unit.final_solution ),
			  matrix_var_ctr ( unit.matrix_var_ctr ),
			  tabu_list ( unit.tabu_list ),
			  worst_variables_list ( unit.worst_variables_list ),
			  must_compute_worst_variables_list ( unit.must_compute_worst_variables_list ),
			  error_variables ( unit.error_variables ),
			  best_sat_error ( unit.best_sat_error ),
			  best_opt_cost ( unit.best_opt_cost ),
			  current_sat_error ( unit.current_sat_error ),
			  current_opt_cost ( unit.current_opt_cost ),
			  cost_before_postprocess ( unit.cost_before_postprocess ),
			  restarts ( unit.restarts ),
			  resets ( unit.resets ),
			  local_moves ( unit.local_moves ),
			  search_iterations ( unit.search_iterations ),
			  local_minimum ( unit.local_minimum ),
			  plateau_moves ( unit.plateau_moves ),
			  plateau_local_minimum ( unit.plateau_local_minimum ),
			  is_permutation_problem ( unit.is_permutation_problem ),
			  options ( unit.options )
		{
			objective = unit.objective;
			unit.objective = nullptr;
		}
		
		SearchUnit& operator=( SearchUnit && unit )
		{
			_stop_search_signal = std::move( unit._stop_search_signal );
			_stop_search_check = std::move( unit._stop_search_check );
			variables = unit.variables; 
			constraints = unit.constraints;
			objective = unit.objective;
			unit.objective = nullptr;
			variables_at_start = unit.variables_at_start;
			is_optimization = unit.is_optimization;
			number_variables = unit.number_variables;
			number_constraints = unit.number_constraints;
			final_solution = unit.final_solution;
			matrix_var_ctr = unit.matrix_var_ctr;
			tabu_list = unit.tabu_list;
			worst_variables_list = unit.worst_variables_list;
			must_compute_worst_variables_list = unit.must_compute_worst_variables_list;
			error_variables = unit.error_variables;
			best_sat_error = unit.best_sat_error;
			best_opt_cost = unit.best_opt_cost;
			current_sat_error = unit.current_sat_error;
			current_opt_cost = unit.current_opt_cost;
			cost_before_postprocess = unit.cost_before_postprocess;
			restarts = unit.restarts;
			resets = unit.resets;
			local_moves = unit.local_moves;
			search_iterations = unit.search_iterations;
			local_minimum = unit.local_minimum;
			plateau_moves = unit.plateau_moves;
			plateau_local_minimum = unit.plateau_local_minimum;
			is_permutation_problem = unit.is_permutation_problem;
			options = unit.options;
			return *this;
		}
		
		// Check if the thread must stop search
		bool stop_search_requested()
		{
			if( _stop_search_check.wait_for( std::chrono::microseconds( 0 ) ) == std::future_status::ready )
				return true;
			else
				return false;
		}
		
		// Request the thread to stop searching
		void stop_search()
		{
			_stop_search_signal.set_value();
		}

		// Method doing the search; called by Solver::solve (eventually in several threads).
		// Return true iff a solution has been found
		void search( double timeout )
		{
			// TODO: Antidote search
			// TODO: Neighborhood
			// TODO: Postprocess

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
			std::chrono::time_point<std::chrono::steady_clock> start_postprocess;

			std::chrono::duration<double,std::micro> timer_postprocess_sat( 0 );
			std::chrono::duration<double,std::micro> timer_postprocess_opt( 0 );
			
			best_sat_error = std::numeric_limits<double>::max();
			best_opt_cost = std::numeric_limits<double>::max();

			initialize_variable_values();
			initialize_data_structures();
			
			std::transform( variables.begin(),
			                variables.end(),
			                final_solution.begin(),
			                [&](auto& var){ return var.get_value(); } );
			
			elapsed_time = std::chrono::steady_clock::now() - start;
			using namespace std::chrono_literals;
			
			// While timeout is not reached, and the solver didn't satisfied
			// all constraints OR it is working on an optimization problem, continue the search.
			while( !stop_search_requested()
			       &&  elapsed_time.count() < timeout
			       && ( best_sat_error > 0.0 || ( best_sat_error == 0.0 && is_optimization ) ) )
			{
				++search_iterations;
				
				/********************************************
				 * 1. Choice of worst variable(s) to change *
				 ********************************************/
				// Estimate which variables need to be changed
				if( must_compute_worst_variables_list )
					compute_worst_variables();

// #if !defined(GHOST_EXPERIMENTAL)
// 				compute_worst_variables();

// 				if( _number_worst_variables > 1 )
// 					_worst_variable = _rng.pick( _worst_variables_list );
// 				else
// 					_worst_variable = _worst_variables_list[ 0 ];
// #else
// 				if( _free_variables )
// 					_worst_variable = _rng.variate<int, std::discrete_distribution>( _error_non_tabu_variables );
// 				else
// 					_worst_variable = _rng.variate<int, std::discrete_distribution>( _error_variables );
// #endif

				if( worst_variables_list.empty() )
				{
#if defined(GHOST_TRACE)
					std::cout << "No variables left to be changed: reset.\n";
#endif					
					reset();
					continue;
				}
				
				auto variable_to_change = rng.pick( worst_variables_list );
				
#if defined(GHOST_TRACE)
				options.print->print_candidate( variables );
				std::cout << "\n\nNumber of loop iteration: " << search_iterations << "\n";
				std::cout << "Number of local moves performed: " << local_moves << "\n";
				std::cout << "Tabu list:";
				for( int i = 0 ; i < number_variables ; ++i )
					if( tabu_list[i] > local_moves )
						std::cout << " v[" << i << "]:" << tabu_list[i];
				std::cout << "\nWorst variables list: v[" << worst_variables_list[0] << "]=" << variables[ worst_variables_list[0] ].get_value();
				for( int i = 1 ; i < static_cast<int>( worst_variables_list.size() ) ; ++i )
					std::cout << ", v[" << worst_variables_list[i] << "]=" << variables[ worst_variables_list[i] ].get_value();
				std::cout << "\nPicked worst variable: v[" << variable_to_change << "]=" << variables[ variable_to_change ].get_value() << "\n\n";
#endif

				/********************************
				 * 2. Choice of their new value *
				 ********************************/
				worst_variables_list.erase( std::find( worst_variables_list.begin(), worst_variables_list.end(), variable_to_change ) );
				// So far, we consider full domains only.
				auto domain_to_explore = variables[ variable_to_change ].get_full_domain();
				// Remove the current value
				domain_to_explore.erase( std::find( domain_to_explore.begin(), domain_to_explore.end(), variables[ variable_to_change ].get_value() ) );
				std::map<int, std::vector<double>> delta_errors;

				if( !is_permutation_problem )
				{
					// Simulate delta errors (or errors is not Constraint::expert_delta_error method is defined) for each neighbor
					for( const auto candidate_value : domain_to_explore )
						for( const unsigned int constraint_id : matrix_var_ctr.at( variable_to_change ) )
							delta_errors[ candidate_value ].push_back( std::visit( [&](Constraint& ctr){ return ctr.simulate_delta( std::vector<unsigned int>{variable_to_change}, std::vector<int>{candidate_value} ); },
							                                                       constraints[ constraint_id ] ) );							
				}
				else
				{
					for( unsigned int variable_id = 0 ; variable_id < number_variables; ++variable_id )
						// look at other variables than the selected one, with other values but contained into the selected variable's domain
						if( variable_id != variable_to_change
						    && variables[ variable_id ].get_value() != variables[ variable_to_change ].get_value()
						    && std::find( domain_to_explore.begin(), domain_to_explore.end(), variables[ variable_id ].get_value() ) != domain_to_explore.end()
						    && std::find( variables[ variable_id ].get_full_domain().begin(),
						                  variables[ variable_id ].get_full_domain().end(),
						                  variables[ variable_to_change ].get_value() ) != variables[ variable_id ].get_full_domain().end() )
						{
							std::vector<bool> constraint_checked( constraints.size(), false );
							int current_value = variables[ variable_to_change ].get_value();
							int candidate_value = variables[ variable_id ].get_value();

							// TODO BUG THREADS
							for( const unsigned int constraint_id : matrix_var_ctr.at( variable_to_change ) )
							{
								constraint_checked[ constraint_id ] = true;
								
								// check if the other variable also belongs to the constraint scope
								if( std::visit( [&](Constraint& ctr){ return ctr.has_variable( variable_id ); }, constraints[ constraint_id ] ) )
									delta_errors[ variable_id ].push_back( std::visit( [&](Constraint& ctr){ return ctr.simulate_delta( std::vector<unsigned int>{variable_to_change, variable_id}, std::vector<int>{candidate_value, current_value} ); },
									                                                   constraints[ constraint_id ] ) );
								else
									delta_errors[ variable_id ].push_back( std::visit( [&](Constraint& ctr){ return ctr.simulate_delta( std::vector<unsigned int>{variable_to_change}, std::vector<int>{candidate_value} ); },
									                                                   constraints[ constraint_id ] ) );
							}
							
							// Since we are switching the value of two variables, we need to also look at the delta error impact of changing the value of the non-selected variable
							for( const unsigned int constraint_id : matrix_var_ctr.at( variable_id ) )
								// No need to look at constraint where variable_to_change also appears.
								if( !constraint_checked[ constraint_id ] )
									delta_errors[ variable_id ].push_back( std::visit( [&](Constraint& ctr){ return ctr.simulate_delta( std::vector<unsigned int>{variable_id}, std::vector<int>{current_value} ); },
									                                                   constraints[ constraint_id ] ) );
						}
				}
				
				// Select the next current configuration (local move)
				std::vector<int> candidate_values;
				std::map<int, double> cumulated_delta_errors;
				double min_conflict = std::numeric_limits<double>::max();
				double new_value;
				for( const auto& deltas : delta_errors )
				{
					cumulated_delta_errors[ deltas.first ] = std::accumulate( deltas.second.begin(), deltas.second.end(), 0.0 );
#if defined(GHOST_TRACE)
					if( is_permutation_problem )
						std::cout << "Error for switching var[" << variable_to_change << "]=" << variables[ variable_to_change ].get_value()
						          << " with var[" << deltas.first << "]=" << variables[ deltas.first ].get_value()
						          << ": " << cumulated_delta_errors[ deltas.first ] << "\n";
					else
						std::cout << "Error for the value " << deltas.first << ": " << cumulated_delta_errors[ deltas.first ] << "\n";
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
				if( is_optimization )
				{
					if( is_permutation_problem )
						new_value = objective->heuristic_value_permutation( variable_to_change, candidate_values );
					else
						new_value = objective->heuristic_value( variable_to_change, candidate_values );
					
					// // to change/test with heuristic_value
					// double objective_cost = std::numeric_limits<double>::max();
					// double simulate_objective_function;
					// for( int value : candidate_values )
					// {
					// 	if( _is_permutation_problem )
					// 		simulate_objective_function = _objective->simulate_cost( std::vector<unsigned int>{variable_to_change, value}, std::vector<int>{_variables[ value ].get_value(),_variables[ variable_to_change ].get_value()} );
					// 	else
					// 		simulate_objective_function = _objective->simulate_cost( std::vector<unsigned int>{variable_to_change}, std::vector<int>{value} );
						
					// 	if( objective_cost > simulate_objective_function )
					// 	{
					// 		objective_cost = simulate_objective_function;
					// 		new_value = value;
					// 	}
					// }
				}
				else
					new_value = rng.pick( candidate_values );

#if defined(GHOST_TRACE)
				std::cout << "Min conflict value candidates list: " << candidate_values[0];
				for( int i = 1 ; i < static_cast<int>( candidate_values.size() ); ++i )
					std::cout << ", " << candidate_values[i];
				if( is_permutation_problem )
					std::cout << "\nPicked variable index for min conflict: "
					          << new_value << "\n"
					          << "Current error: " << current_sat_error << "\n"
					          << "Delta: " << min_conflict << "\n\n";
				else
					std::cout << "\nPicked value for min conflict: "
					          << new_value << "\n"
					          << "Current error: " << current_sat_error << "\n"
					          << "Delta: " << min_conflict << "\n\n";
#endif

				/****************************************
				 * 3. Error improved => make local move *
				 ****************************************/
				if( min_conflict < 0.0 )
				{
#if defined(GHOST_TRACE)
					std::cout << "Global error improved (" << current_sat_error << " -> " << current_sat_error + min_conflict << "): make local move.\n";
#endif
					local_move( variable_to_change, new_value, min_conflict, delta_errors );
					if( is_optimization )
						current_opt_cost = objective->cost();
				}
				else
				{
					/*****************
					 * 4. Same error *
					 *****************/
					if( min_conflict == 0.0 )
					{
#if defined(GHOST_TRACE)
						std::cout << "Global error stable; ";
#endif
						if( is_optimization )
						{
							double candidate_opt_cost;
							if( is_permutation_problem )
							{
								int backup_variable_to_change = variables[ variable_to_change ].get_value();
								int backup_variable_new_value = variables[ new_value ].get_value();
								objective->update_variable( variable_to_change, backup_variable_new_value );
								objective->update_variable( new_value, backup_variable_to_change );
								candidate_opt_cost = objective->cost();
								objective->update_variable( variable_to_change, backup_variable_to_change );
								objective->update_variable( new_value, backup_variable_new_value );
							}
							else
							{
								int backup = variables[ variable_to_change ].get_value();
								objective->update_variable( variable_to_change, new_value );
								candidate_opt_cost = objective->cost();
								objective->update_variable( variable_to_change, backup );
							}

							/******************************************************
							 * 4.a. Optimization cost improved => make local move *
							 ******************************************************/
							if( current_opt_cost > candidate_opt_cost )
							{								
#if defined(GHOST_TRACE)
								std::cout << "optimization cost improved (" << current_opt_cost << " -> " << candidate_opt_cost << "): make local move.\n";
#endif
								local_move( variable_to_change, new_value, min_conflict, delta_errors );
								current_opt_cost = candidate_opt_cost;
							}
							else
								/******************************************
								 * 4.b. Same optimization cost => plateau *
								 ******************************************/
								if( current_opt_cost == candidate_opt_cost )
								{
#if defined(GHOST_TRACE)
									std::cout << "optimization cost stable (" << current_opt_cost << "): plateau.\n";
#endif
									plateau_management( variable_to_change, new_value, delta_errors );
								}
								else // _current_opt_cost < candidate_opt_cost
								{
									/*************************************************
									 * 4.c. Worst optimization cost => local minimum *
									 *************************************************/
#if defined(GHOST_TRACE)
									std::cout << "optimization cost increase (" << current_opt_cost << " -> " << candidate_opt_cost << "): local minimum.\n";
#endif
									local_minimum_management( variable_to_change, new_value, worst_variables_list.empty() );
								}
						}
						else
						{
							/***********************************************
							 * 4.d. Not an optimization problem => plateau *
							 ***********************************************/
#if defined(GHOST_TRACE)
							std::cout << "no optimization: plateau.\n";
#endif
							plateau_management( variable_to_change, new_value, delta_errors );
						}
					}
					else // min_conflict > 0.0
					{
						/***********************************
						 * 5. Worst error => local minimum *
						 ***********************************/
#if defined(GHOST_TRACE)
						std::cout << "Global error increase: local minimum.\n";
#endif
						local_minimum_management( variable_to_change, new_value, worst_variables_list.empty() );
					}
				}

				if( best_sat_error > current_sat_error )
				{
#if defined(GHOST_TRACE)
					std::cout << "Best satisfaction error so far (in an optimization problem). Before: " << best_sat_error << ", now: " << current_sat_error << "\n";
#endif
					best_sat_error = current_sat_error;
					std::transform( variables.begin(),
					                variables.end(),
					                final_solution.begin(),
					                [&](auto& var){ return var.get_value(); } );
				}

				if( is_optimization && best_opt_cost > current_opt_cost )
				{
#if defined(GHOST_TRACE)
					std::cout << "Best objective function value so far. Before: " << best_opt_cost << ", now: " << current_opt_cost << "\n";
#endif
					best_opt_cost = current_opt_cost;
					std::transform( variables.begin(),
					                variables.end(),
					                final_solution.begin(),
					                [&](auto& var){ return var.get_value(); } );
				}

				elapsed_time = std::chrono::steady_clock::now() - start;
			} // while loop

			solution_found.set_value( best_sat_error == 0.0 );
		}
	};

	//! NullObjective is used when no objective functions have been given to the solver (ie, for pure satisfaction runs). 
	class NullObjective : public Objective
	{
		//using Objective::rng;
		
	public:
		NullObjective( const std::vector<Variable>& variables ) : Objective( "nullObjective", variables ) { }
		
	private:
		double required_cost( const std::vector<Variable>& variables ) const override { return 0.0; }
		
		int expert_heuristic_value( std::vector<Variable> variables,
		                            int variable_index,
		                            const std::vector<int>& values_list ) const override
		{
			return rng.pick( values_list );
		}
	};
	
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
	template<typename ObjectiveType = NullObjective, typename ... ConstraintType>
	class Solver final
	{
		std::vector<Variable> _variables; //!< Vector of Variable objects.
		std::vector<std::variant<ConstraintType ...>> _constraints; //!< Vector of Constraint variants.
		std::shared_ptr<ObjectiveType> _objective; //!< Shared pointer of the objective function.

		bool _is_optimization; //!< A boolean to know if it is a satisfaction or optimization run.

		unsigned int _number_variables; //!< Size of the vector of variables.
		unsigned int _number_constraints; //!< Size of the vector of constraints.
		std::vector<std::vector<unsigned int> > _matrix_var_ctr; //!< Matrix to know in which constraints each variable is.

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
		 * \param variables A const reference to the vector of Variables.
		 * \param constraints A const reference to the vector of variant Constraint-derivated objects.
		 * \param obj A shared pointer to the Objective.
		 * \param permutation_problem A boolean indicating if we work on a permutation problem. False by default.
		 */
		Solver( const std::vector<Variable>& variables, 
		        const std::vector<std::variant<ConstraintType ...>>&	constraints,
		        std::shared_ptr<ObjectiveType> objective,
		        bool permutation_problem = false )
			: _variables ( variables ), 
			  _constraints ( constraints ),
			  _objective ( objective ),
			  _is_optimization ( _objective == nullptr ? false : true ),
			  _number_variables ( static_cast<unsigned int>( variables.size() ) ),
			  _number_constraints ( static_cast<unsigned int>( constraints.size() ) ),
			  _matrix_var_ctr ( std::vector<std::vector<unsigned int> >( _number_variables ) ),
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
		{
			if( !_is_optimization )
				_objective = std::make_shared<NullObjective>( _variables );
			
			// Set the id of each constraint object to be their index in the _constraints vector
			for( unsigned int constraint_id = 0; constraint_id < _number_constraints; ++constraint_id )
				std::visit( [&](Constraint& ctr){ ctr._id = constraint_id; }, _constraints[ constraint_id ] );
			
			for( unsigned int variable_id = 0; variable_id < _number_variables; ++variable_id )
			{
				// Get the original id of each variable to make the mapping between the variable's id in the solver and its id within constraints (i.e., its original id)
				unsigned int original_variable_id = _variables[ variable_id ]._id;
				
				// Set the id of each variable object to be their index in the _variables vector
				_variables[ variable_id ]._id = variable_id;

				// Save the id of each constraint where the current variable appears in.
				for( unsigned int constraint_id = 0; constraint_id < _number_constraints; ++constraint_id )
					if(	std::visit( [&](Constraint& ctr){ return ctr.has_variable_unshifted( original_variable_id ); }, _constraints[ constraint_id ] ) )
					{
						_matrix_var_ctr[ variable_id ].push_back( constraint_id );
						std::visit( [&](Constraint& ctr){ ctr.make_variable_id_mapping( variable_id, original_variable_id ); }, _constraints[ constraint_id ] );
					}

				_objective->make_variable_id_mapping( variable_id, original_variable_id );
			}

			// Determine if expert_delta_error has been user defined or not for each constraint
			for( unsigned int constraint_id = 0; constraint_id < _number_constraints; ++constraint_id )
				try
				{
					std::visit( [&](Constraint& ctr){ ctr.expert_delta_error( _variables, std::vector<unsigned int>{0}, std::vector<int>{_variables[0].get_value()} ); }, _constraints[ constraint_id ] );
				}
				catch( std::exception e )
				{
					std::cerr << "No expert_delta_error method defined for constraint num. " << constraint_id << "\n";
				}

#if defined(GHOST_TRACE)
			std::cout << "Creating a Solver object\n\n"
			          << "Variables:\n";

			for( const auto& variable : _variables )
				std::cout << variable << "\n";

			std::cout << "\nConstraints:\n";
			for( const auto& constraint : _constraints )
				std::visit( [&](Constraint& ctr){ std::cout << ctr << "\n"; }, constraint );
			
			std::cout << "\nObjective function:\n"
			          << *_objective << "\n";				
#endif
		}

		//! Second Solver's constructor, without Objective
		/*!
		 * \param variables A const reference to the vector of Variables.
		 * \param constraints A const reference to the vector of variant Constraint-derivated objects.
		 * \param permutation_problem A boolean indicating if we work on a permutation problem. False by default.
		 */
		Solver( const std::vector<Variable>& variables, 
		        const std::vector<std::variant<ConstraintType ...>>&	constraints,
		        bool permutation_problem = false )
			: Solver( variables, constraints, nullptr, permutation_problem )
		{ }
    
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
			/*****************
			* Initialization *
			******************/
			_options = options;
			
			if( _options.tabu_time_local_min == -1 )
				_options.tabu_time_local_min = std::max( std::min( 5, static_cast<int>( _number_variables ) - 1 ), static_cast<int>( std::ceil( _number_variables / 5 ) ) ) + 1;
			  //_options.tabu_time_local_min = std::max( 2, _tabu_threshold ) );

			if( _options.tabu_time_selected == -1 )
				_options.tabu_time_selected = 0;

			if( _options.reset_threshold == -1 ) 
				_options.reset_threshold = _options.tabu_time_local_min;
			  //_options.reset_threshold = static_cast<int>( std::ceil( std::sqrt( _number_variables ) ) );

			if( _options.restart_threshold == -1 ) 
				_options.restart_threshold = _number_variables;

			if( _options.percent_to_reset == -1 )
				_options.percent_to_reset = std::max( 2, static_cast<int>( std::ceil( _number_variables * 0.1 ) ) ); // 10%
				
			std::chrono::duration<double,std::micro> elapsed_time( 0 );
			std::chrono::time_point<std::chrono::steady_clock> start( std::chrono::steady_clock::now() );
			std::chrono::time_point<std::chrono::steady_clock> start_postprocess;

			std::chrono::duration<double,std::micro> timer_postprocess_sat( 0 );
			std::chrono::duration<double,std::micro> timer_postprocess_opt( 0 );

			double chrono_search;
			double chrono_full_computation;
			
			// In case final_solution is not a vector of the correct size,
			// ie, equals to the number of variables.
			final_solution.resize( _number_variables );
			bool solution_found;			
			
			// sequential runs
			if( !_options.parallel_runs || _options.number_threads == 1 )
			{
				SearchUnit search_unit( _variables,
				                        _constraints,
				                        _objective,
				                        _is_optimization,
				                        _matrix_var_ctr,
				                        _is_permutation_problem,
				                        _options );
				
				std::future<bool> unit_future = search_unit.solution_found.get_future();
				
				search_unit.search( timeout );
				elapsed_time = std::chrono::steady_clock::now() - start;
				
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
			}
			else // call threads
			{
				std::vector<SearchUnit<ObjectiveType, ConstraintType ...> > units;
				units.reserve( _options.number_threads );
				std::vector<std::jthread> unit_threads;
				
				for( int i = 0 ; i < _options.number_threads; ++i )
					units.emplace_back( _variables,
					                    _constraints,
					                    _objective,
					                    _is_optimization,
					                    _matrix_var_ctr,
					                    _is_permutation_problem,
					                    _options );

				std::vector<std::future<bool>> units_future;
				std::vector<bool> units_terminated( _options.number_threads, false );
				
				for( int i = 0 ; i < _options.number_threads; ++i )
				{
					unit_threads.emplace_back( &SearchUnit<ObjectiveType, ConstraintType ...>::search, &units.at(i), timeout );
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
									solution_found = false;
									end_of_computation = true;
									break;
								}
							}
						}							
					}
				}
				
				elapsed_time = std::chrono::steady_clock::now() - start;
				chrono_search = elapsed_time.count();

				// Collect all interesting data before terminating jthreads.
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

					if( i == winning_thread ) // if no search units found a solution, this would give the stats of the first jthread
					{
						_restarts = units.at(i).restarts;
						_resets = units.at(i).resets;
						_local_moves = units.at(i).local_moves;
						_search_iterations = units.at(i).search_iterations;
						_local_minimum = units.at(i).local_minimum;
						_plateau_moves = units.at(i).plateau_moves;
						_plateau_local_minimum = units.at(i).plateau_local_minimum;
					}
				}

				// ..then the most important: the best solution found so far.
				if( solution_found )
				{
					final_solution = units.at( winning_thread ).final_solution;
					_best_sat_error = units.at( winning_thread ).best_sat_error;
					_best_opt_cost = units.at( winning_thread ).best_opt_cost;
				}
				else
					for( int i = 0 ; i < _options.number_threads ; ++i )
					{
						if( _best_sat_error > units.at( i ).best_sat_error )
						{
							final_solution = units.at( i ).final_solution;
							_best_sat_error = units.at( i ).best_sat_error;
						}
						if( _is_optimization && _best_sat_error == 0.0 )
							if( units.at( i ).best_sat_error == 0.0 && _best_opt_cost > units.at( i ).best_opt_cost )
							{
								final_solution = units.at( i ).final_solution;
								_best_opt_cost = units.at( i ).best_opt_cost;
							}
					}
			}
			
			if( solution_found && _is_optimization )
			{
				_cost_before_postprocess = _best_opt_cost;

				start_postprocess = std::chrono::steady_clock::now();
				_objective->postprocess_optimization( _best_opt_cost, final_solution );
				timer_postprocess_opt = std::chrono::steady_clock::now() - start_postprocess;
			}

			if( _is_optimization )
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
			for( unsigned int variable_id = 0 ; variable_id < _number_variables; ++variable_id )
				_variables[ variable_id ].set_value( final_solution[ variable_id ] );

			elapsed_time = std::chrono::steady_clock::now() - start;
			chrono_full_computation = elapsed_time.count();
			
#if defined(GHOST_DEBUG) || defined(GHOST_TRACE) || defined(GHOST_BENCH)
			std::cout << "@@@@@@@@@@@@" << "\n"
			          << "Options:\n"
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
			_options.print->print_candidate( _variables );

			std::cout << "\n";
			
			if( !_is_optimization )
				std::cout << "SATISFACTION run" << "\n";
			else
				std::cout << "OPTIMIZATION run with objective " << _objective->get_name() << "\n";

			std::cout << "Search time: " << chrono_search / 1000 << "ms\n"
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
			
			if( _is_optimization )
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
			return solve( final_cost, final_solution, timeout, Options() );
		}

	};
}
