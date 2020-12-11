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
 * Copyright (C) 2014-2020 Florian Richoux
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

#include "variable.hpp"
#include "constraint.hpp"
#include "objective.hpp"
#include "neighborhood.hpp"
#include "misc/randutils.hpp"

namespace ghost
{
	template<typename ... ConstraintType>
	std::vector<std::variant<ConstraintType ...>> initiate_vector_constraints()
	{
		return std::vector<std::variant<ConstraintType ...>>();
	}
	
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
	template<typename ... ConstraintType>
	class Solver final
	{
		std::vector<Variable> _variables; //!< Reference to the vector of variables.
		std::vector<std::variant<ConstraintType ...>> _constraints; //!< Reference to the vector of shared pointer constraints.
		std::unique_ptr<Objective> _objective; //!< Shared pointer of the objective function.

		//Random _random; //!< The random generator used by the solver.
		randutils::mt19937_rng _rng; //!< A neat random generator from randutils.hpp.

		bool _is_optimization; //!< A boolean to know if it is a satisfaction or optimization run.
		bool _no_random_starting_point;
		bool _free_variables;
		unsigned int _number_variables; //!< Size of the vector of variables.
		unsigned int _number_constraints; //!< Size of the vector of constraints.
		std::vector<std::vector<unsigned int> > _map_var_ctr; //!< Map to know in which constraints each variable are.
		std::vector<int> _weak_tabu_list; //!< The weak tabu list, frozing used variables for tabu_time iterations.
		                                  // Weak, because it can be violated.
		                                  // Implemented by a map<int variable_id, int number_of_iterations_before_getting_out>.
		std::vector<unsigned int> _worst_variables_list;
		int _number_worst_variables;
		
		std::vector<double> _error_constraints;
		std::vector<double> _error_variables;
		std::vector<double> _error_non_tabu_variables;

		unsigned int _worst_variable;
		double _best_sat_error; //!< The satisfaction cost of the best solution.
		double _best_sat_error_opt_loop; //!< The satisfaction cost of the best solution in the current optimization loop.
		double _best_opt_cost; //!< The optimization cost of the best solution.
		double _current_sat_error;
		double _current_opt_cost;
		double _cost_before_postprocess;

		Neighborhood _neighborhood;
		std::vector< std::vector<int> > _neighbors; 
		
		//! NullObjective is used when no objective functions have been given to the solver (ie, for pure satisfaction runs). 
		class NullObjective : public Objective
		{
			using Objective::rng;
      
		public:
			NullObjective( const std::vector<Variable>& variables ) : Objective( "nullObjective", variables ) { }
      
		private:
			double required_cost( const std::vector<Variable>& variables ) const override { return 0.; }
      
			int expert_heuristic_value( const std::vector<Variable>& variables,
			                            Variable& var,
			                            const std::vector<int>& values_list ) const override
			{
				return rng.pick( values_list );
			}
		};
    
		//! Set the initial configuration by calling monte_carlo_sampling() 'samplings' times.
		/*!
		 * After calling calling monte_carlo_sampling() 'samplings' times, the function keeps 
		 * the configuration wth the lowest satisfaction cost. If some of them reach 0, it keeps 
		 * the configuration with the best optimization cost. 
		 */
		void set_initial_configuration( int samplings = 1 )
		{
			if( !_neighborhood.is_permutation() )
			{
				if( samplings == 1 )
				{
					monte_carlo_sampling();
				}
				else
				{
					// To avoid weird samplings numbers like 0 or -1
					samplings = std::max( 2, samplings );
    
					double best_sat_error = std::numeric_limits<double>::max();
					double _current_sat_error;

					std::vector<int> best_values( _number_variables, 0 );
    
					for( int i = 0 ; i < samplings ; ++i )
					{
						monte_carlo_sampling();
						_current_sat_error = 0.;
						for( unsigned int constraint_id = 0; constraint_id < _number_constraints; ++constraint_id )
							_current_sat_error += call_cost( constraint_id );
      
						if( best_sat_error > _current_sat_error )
							update_better_configuration( best_sat_error, _current_sat_error, best_values );

						if( _current_sat_error == 0. )
							break;
					}

					for( unsigned int variable_id = 0; variable_id < _number_variables; ++variable_id )
						_variables[ variable_id ].set_value( best_values[ variable_id ] );
				}
			}
			else
			{
				// To avoid weird samplings numbers like 0 or -1
				samplings = std::max( 1, samplings );
		
				double best_sat_error = std::numeric_limits<double>::max();
				double _current_sat_error;
		
				std::vector<int> best_values( _number_variables, 0 );
		
				for( int i = 0 ; i < samplings ; ++i )
				{
					random_permutations();
					_current_sat_error = 0.;
					for( unsigned int constraint_id = 0; constraint_id < _number_constraints; ++constraint_id )
						_current_sat_error += call_cost( constraint_id );
			
					if( best_sat_error > _current_sat_error )
						update_better_configuration( best_sat_error, _current_sat_error, best_values );
			
					if( _current_sat_error == 0. )
						break;
				}
		
				for( int i = 0; i < _number_variables; ++i )
					_variables[ i ].set_value( best_values[ i ] );
			}
		}

		//! Sample an configuration
		void monte_carlo_sampling()
		{
			for( auto& v : _variables )
				v.pick_random_value();
		}


		//! Sample an configuration for permutation problems
		void random_permutations()
		{
			for( unsigned int i = 0; i < _variables.size() - 1; ++i )
				for( unsigned int j = i + 1; j < _variables.size(); ++j )
				{
					// About 50% to do a swap for each couple (var_i, var_j)
					if( _rng.uniform( 0, 1 ) == 0 )
					{
						std::swap( _variables[i]._index, _variables[j]._index );
						std::swap( _variables[i]._current_value, _variables[j]._current_value );
					}
				}
		}

		void restart()
		{
			_best_sat_error = std::numeric_limits<double>::max();
			_best_opt_cost = std::numeric_limits<double>::max();
			_best_sat_error_opt_loop = std::numeric_limits<double>::max();

			// Reset weak tabu list
			std::fill( _weak_tabu_list.begin(), _weak_tabu_list.end(), 0 ); 

			// start from a random configuration, if no_random_starting_point is false
			if( !_no_random_starting_point )
				set_initial_configuration( 10 );
			else
				// From the second turn in this loop, start from a random configuration
				// TODO: What if users REALLY want to start searching from their own starting point?
				_no_random_starting_point = false;

			// Reset constraints costs
			std::fill( _error_constraints.begin(), _error_constraints.end(), 0.0 ); 
					
			// Send the current variables assignment to the constraints.
			for( unsigned int variable_id = 0; variable_id < _number_variables; ++variable_id )
				for( unsigned int constraint_id : _map_var_ctr[ variable_id ] )
					std::visit( [&](Constraint& ctr){ return ctr.update_variable( variable_id, _variables[ variable_id ].get_value() ); }, _constraints[ constraint_id ] );
			
			// Recompute constraints costs and get the total current cost
			_current_sat_error = compute_constraints_errors();

			// Reset variables costs
			std::fill( _error_variables.begin(), _error_variables.end(), 0.0 ); 
			// Recompute it
			compute_variables_errors();
			
			for( unsigned int variable_id = 0; variable_id < _number_variables; ++variable_id )
				if( _weak_tabu_list[ variable_id ] == 0 )
					_error_non_tabu_variables[ variable_id ] = _error_variables[ variable_id ];
				else
					_error_non_tabu_variables[ variable_id ] = 0.0;

			_free_variables = decay_weak_tabu_list();

		}

		inline double call_cost( unsigned int constraint_id )
		{
			return std::visit( [&](Constraint& ctr){ return ctr.error(); }, _constraints[ constraint_id ] );
		}
		
		//! Decreasing values in tabuList
		/*!
		 * \return True iff there is at least one free variable, ie, untabu. 
		 */
		bool decay_weak_tabu_list()
		{
			bool free_variables = false;
			std::for_each( _weak_tabu_list.begin(),
			               _weak_tabu_list.end(),
			               [&](auto& t){ t == 0 ? free_variables : --t; assert( t >= 0); } );
			return free_variables;
		}

		//! To factorize code like if (best > current) then best=current and update configuration
		void update_better_configuration( double& best, const double current, std::vector<int>& configuration )
		{
			best = current;

			for( int i = 0; i < _number_variables; ++i )
				configuration[ i ] = _variables[ i ].get_value();
		}

#if !defined(EXPERIMENTAL)
		//! To compute the vector of variables which are principal culprits for not satisfying the problem
		void compute_worst_variables()
		{
			// Here, we look at neighbor configurations with the lowest cost.
			double _worst_variableCost = 0.;
			_number_worst_variables = 0;
  
			for( int id = 0; id < _number_variables; ++id )
			{
				if( !_free_variables || _weak_tabu_list[ id ] == 0 )
				{
					if( _worst_variableCost < _error_variables[ id ] )
					{
						_worst_variableCost = _error_variables[ id ];
						_number_worst_variables = 0;
						_worst_variables_list[ _number_worst_variables++ ] = id;
					}
					else 
						if( _worst_variableCost == _error_variables[ id ] )
							_worst_variables_list[ _number_worst_variables++ ] = id;
				}
			}
		}
#endif
    
		//! Compute the cost of each constraints and fill up the _error_constraints map
		double compute_constraints_errors()
		{
			double satisfaction_cost = 0.;
			double cost;
  
			for( unsigned int constraint_id = 0; constraint_id < _number_constraints; ++constraint_id )
			{
				cost = call_cost( constraint_id );
				//cost = _constraints[ id ].cost();
				_error_constraints[ constraint_id ] = cost;
				satisfaction_cost += cost;    
			}

			return satisfaction_cost;
		}

		//! Compute the variable cost of each variables and fill up maps _error_variables and _error_non_tabu_variables 
		void compute_variables_errors()
		{
			for( unsigned int variable_id = 0; variable_id < _number_variables; ++variable_id )
				for( unsigned int constraint_id : _map_var_ctr[ variable_id ] )
					_error_variables[ variable_id ] += _error_constraints[ constraint_id ];
		}

		//! Compute incrementally the now satisfaction cost IF we change the value of 'variable' by 'value' with a local move.
		double simulate_local_move_error( unsigned int variable_id,
		                                 int value,
		                                 double	_current_sat_error )
		{
			double new__current_sat_error = _current_sat_error;

			_variables[ variable_id ].set_value( value );
			for( unsigned int constraint_id : _map_var_ctr[ variable_id ] )
				new__current_sat_error += ( call_cost( constraint_id ) - _error_constraints[ constraint_id ] );

			return new__current_sat_error;
		}


		//! Compute incrementally the now satisfaction cost IF we swap values of 'variable' with another variable.
		double simulate_permutation_error( unsigned int _worst_variable,
		                                  unsigned int other_variable,
		                                  double _current_sat_error )
		{
			double new__current_sat_error = _current_sat_error;
			std::vector<bool> done( _number_constraints, false );
	
			std::swap( _variables[ _worst_variable ]._index, _variables[ other_variable ]._index );
			std::swap( _variables[ _worst_variable ]._current_value, _variables[ other_variable ]._current_value );
    
			for( unsigned int constraint_id : _map_var_ctr[ _worst_variable ] )
			{
		
				new__current_sat_error += ( call_cost( constraint_id ) - _error_constraints[ constraint_id ] );
				done[ constraint_id ] = true;
			}

			// The following was commented to avoid branch misses, but it appears to be slower than
			// the commented block that follows.
			for( unsigned int constraint_id : _map_var_ctr[ other_variable ] )
				if( !done[ constraint_id ] )
					new__current_sat_error += ( call_cost( constraint_id ) - _error_constraints[ constraint_id ] );

			// vector< shared_ptr<Constraint> > diff;
			// std::set_difference( _map_var_ctr[ other_variable ].begin(), _map_var_ctr[ other_variable ].end(),
			//                      _map_var_ctr[ *_worst_variable ].begin(), _map_var_ctr[ *_worst_variable ].end(),
			//                      std::inserter( diff, diff.begin() ) );

			// for( auto& c : diff )
			// 	new__current_sat_error += ( c->cost() - _error_constraints[ c->get_id() - _ctr_offset ] );

			// We must roll back to the previous state before returning the new cost value. 
			std::swap( _variables[ _worst_variable ]._index, _variables[ other_variable ]._index );
			std::swap( _variables[ _worst_variable ]._current_value, _variables[ other_variable ]._current_value );

			return new__current_sat_error;
		}

		//! Function to make a local move, ie, to assign a given
		void local_move( unsigned int variable_id,
		                 double& _current_sat_error )
		{
			// Here, we look at values in the variable domain
			// leading to the lowest satisfaction cost.
			double new__current_sat_error = 0.0;
			std::vector<int> best_values_list;
			int best_value;
			double best_cost = std::numeric_limits<double>::max();
  
			for( auto& val : _variables[ variable_id ].get_full_domain() )
			{
				new__current_sat_error = simulate_local_move_error( variable_id, val, _current_sat_error );
				if( best_cost > new__current_sat_error )
				{
					best_cost = new__current_sat_error;
					best_values_list.clear();
					best_values_list.push_back( val );
				}
				else 
					if( best_cost == new__current_sat_error )
						best_values_list.push_back( val );	  
			}

			// If several values lead to the same best satisfaction cost,
			// call Objective::heuristic_value has a tie-break.
			// By default, Objective::heuristic_value returns the value
			// improving the most the optimization cost, or a random value
			// among values improving the most the optimization cost if there
			// are some ties.
			if( best_values_list.size() > 1 )
				best_value = _objective->heuristic_value( _variables, _variables[ variable_id ], best_values_list );
			else
				best_value = best_values_list[0];

			_variables[ variable_id ].set_value( best_value );
			_current_sat_error = best_cost;
			// for( auto& c : _map_var_ctr[ *variable ] )
			//   _error_constraints[ c->get_id() - _ctr_offset ] = c->cost();

			// compute_variables_errors( _current_sat_error );
		}

		//! Function to make a permutation move, ie, to assign a given
		void permutation_move( unsigned int variable_id,
		                       double& _current_sat_error )
		{
			// Here, we look at values in the variable domain
			// leading to the lowest satisfaction cost.
			double new__current_sat_error = 0.0;
			std::vector< unsigned int > best_var_to_swap_list;
			unsigned int best_var_to_swap;
			double best_cost = std::numeric_limits<double>::max();

#if defined(TRACE)
			std::cout << "Current error before permutation: " << _current_sat_error << "\n";
#endif

			for( unsigned int other_variable_id = 0; other_variable_id < _number_variables; ++other_variable_id )
			{
				// Next line is replaced by a simpler conditional since there were A LOT of branch misses!
				//if( other_variable._id == variable->_id || other_variable._index == variable->_index )
				if( other_variable_id == variable_id )
					continue;
    
				new__current_sat_error = simulate_permutation_error( variable_id, other_variable_id, _current_sat_error );

#if defined(TRACE)
				std::cout << "Error if permutation between " << variable_id << " and " << other_variable_id << ": " << new__current_sat_error << "\n";
#endif

				if( best_cost > new__current_sat_error )
				{
#if defined(TRACE)
					std::cout << "This is a new best error.\n";
#endif
			
					best_cost = new__current_sat_error;
					best_var_to_swap_list.clear();
					best_var_to_swap_list.push_back( other_variable_id );
				}
				else 
					if( best_cost == new__current_sat_error )
					{
						best_var_to_swap_list.push_back( other_variable_id );
#if defined(TRACE)
						std::cout << "Tie error with the best one.\n";
#endif
					}
			}

			// // If the best cost found so far leads to a plateau,
			// // then we have 10% of chance to escapte from the plateau
			// // by picking up a random variable (giving a worst global cost)
			// // to permute with.
			// if( best_cost == _current_sat_error && _rng.uniform( 0, 99 ) < 10 )
			// {
			// 	do
			// 	{
			// 		best_var_to_swap = _rng.pick( _variables );
			// 	} while( best_var_to_swap._id == variable->_id || std::find_if( best_var_to_swap_list.begin(), best_var_to_swap_list.end(), [&](auto& v){ return v._id == best_var_to_swap._id; } ) != best_var_to_swap_list.end() );
			// }
			// else
			// {
			// If several values lead to the same best satisfaction cost,
			// call Objective::heuristic_value has a tie-break.
			// By default, Objective::heuristic_value returns the value
			// improving the most the optimization cost, or a random value
			// among values improving the most the optimization cost if there
			// are some ties.
			if( best_var_to_swap_list.size() > 1 )
				best_var_to_swap = _objective->heuristic_value( best_var_to_swap_list );
			else
				best_var_to_swap = best_var_to_swap_list[0];
			// }

#if defined(TRACE)
			std::cout << "Permutation will be done between " << variable_id << " and " << best_var_to_swap << ".\n";
#endif

			std::swap( _variables[ variable_id ]._index, _variables[ best_var_to_swap ]._index );
			std::swap( _variables[ variable_id ]._current_value, _variables[ best_var_to_swap ]._current_value );

			_current_sat_error = best_cost;
			// vector<bool> compted( _error_constraints.size(), false );
  
			// for( auto& c : _map_var_ctr[ *variable ] )
			// {
			// 	new__current_sat_error += ( c->cost() - _error_constraints[ c->get_id() - _ctr_offset ] );
			// 	compted[ c->get_id() - _ctr_offset ] = true;
			// }
  
			// for( auto& c : _map_var_ctr[ best_var_to_swap ] )
			// 	if( !compted[ c->get_id() - _ctr_offset ] )
			// 		new__current_sat_error += ( c->cost() - _error_constraints[ c->get_id() - _ctr_offset ] );

			// compute_variables_errors( new__current_sat_error );
		}
    
	public:
		//! Solver's regular constructor
		/*!
		 * \param variables A reference to the vector of Variables.
		 * \param constraints A reference to the vector of shared pointer of Constraints.
		 * \param obj A shared pointer to the Objective.
		 * \param permutation_problem A boolean indicating if we work on a permutation problem. False by default.
		 */
		Solver( const std::vector<Variable>& variables, 
		        const std::vector<std::variant<ConstraintType ...>>&	constraints,
		        std::unique_ptr<Objective> objective,
		        bool permutation_problem = false )
			: _variables ( variables ), 
			  _constraints ( constraints ),
			  _objective ( std::move( objective ) ),
			  _is_optimization ( _objective == nullptr ? false : true ),
			  _no_random_starting_point( false ),
			  _free_variables( true ),
			  _number_variables ( static_cast<unsigned int>( variables.size() ) ),
			  _number_constraints ( static_cast<unsigned int>( constraints.size() ) ),
			  _map_var_ctr ( std::vector<std::vector<unsigned int> >( _number_variables ) ),
			  _weak_tabu_list( std::vector<int>( _number_variables, 0 ) ),
			  _worst_variables_list( std::vector<unsigned int>( _number_variables, 0 ) ),
			  _number_worst_variables( 0 ),
			  _error_variables( std::vector<double>( _number_variables, 0.0 ) ),
			  _error_constraints( std::vector<double>( _number_constraints, 0.0 ) ),
			  _error_non_tabu_variables( std::vector<double>( _number_variables, 0.0 ) ),
			  _worst_variable( 0 ),
			  _best_sat_error( std::numeric_limits<double>::max() ),
			  _best_sat_error_opt_loop( std::numeric_limits<double>::max() ),
			  _best_opt_cost( std::numeric_limits<double>::max() ),
			  _current_sat_error( std::numeric_limits<double>::max() ),
			  _current_opt_cost( std::numeric_limits<double>::max() ),
			  _cost_before_postprocess( std::numeric_limits<double>::max() ),
			  _neighborhood ( { 1, 1.0, permutation_problem, 1.0 } )
		{
			if( !_is_optimization )
				_objective = std::make_unique<NullObjective>( _variables );

			// Set the id of each constraint object to be their index in the _constraints vector
			for( unsigned int constraint_id = 0; constraint_id < _number_constraints; ++constraint_id )
				std::visit( [&](Constraint& ctr){ ctr._id = constraint_id; }, _constraints[ constraint_id ] );			
		
			for( unsigned int variable_id = 0; variable_id < _number_variables; ++variable_id )
			{
				// Get the original id of each variable to make the mapping between the variable's id in the solver and its id within constraints (i.e., its original id)
				unsigned int original_variable_id = _variables[ variable_id ]._id;
				
				// Set the id of each variable object to be their index in the _variables vector
				_variables[ variable_id ]._id = variable_id;

				// Save the id of each constraint the current variable appears in.
				for( unsigned int constraint_id = 0; constraint_id < _number_constraints; ++constraint_id )
					if(	std::visit( [&](Constraint& ctr){ return ctr.has_variable( _variables[ variable_id ] ); }, _constraints[ constraint_id ] ) )
					{
						_map_var_ctr[ variable_id ].push_back( constraint_id );
						std::visit( [&](Constraint& ctr){ ctr.make_variable_id_mapping( variable_id, original_variable_id ); }, _constraints[ constraint_id ] );
					}

				_objective->make_variable_id_mapping( variable_id, original_variable_id );
			}
		}

		//! Second Solver's constructor, without Objective
		/*!
		 * \param variables A reference to the vector of Variables.
		 * \param constraints A reference to the vector of shared pointer of Constraints.
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
		 * problem constraint (computated by Constraint::required_cost). For an optimization problem, the cost is the value outputed
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
		 * \param no_random_starting_point A Boolean to indicate if the solver should not start from a random starting point. This is necessary in particular to use the resume feature.
		 * \return True iff a solution has been found.
		 */
		bool solve( double& final_cost,
		            std::vector<int>& final_solution,
		            double timeout,
		            bool no_random_starting_point = false )
		{
			_no_random_starting_point = no_random_starting_point;
			
			// The only parameter of Solver<Variable, Constraint>::solve outside timeouts
			int tabu_time_local_min = std::max( static_cast<unsigned int>( 1 ), _number_variables / 2); // _number_variables - 1;
			int tabu_time_selected = std::max( 1, tabu_time_local_min / 2);

			std::chrono::duration<double,std::micro> elapsed_time( 0 );
			std::chrono::time_point<std::chrono::steady_clock> start( std::chrono::steady_clock::now() );
			std::chrono::time_point<std::chrono::steady_clock> start_postprocess;

			std::chrono::duration<double,std::micro> timer_postprocess_sat( 0 );
			std::chrono::duration<double,std::micro> timer_postprocess_opt( 0 );
			
			int search_iterations = 0;
			int restarts = 0;

			// In case final_solution is not a vector of the correct size,
			// ie, equals to the number of variables.
			final_solution.resize( _number_variables );

			// Call restart to initialize data structures.
			restart();

			elapsed_time = std::chrono::steady_clock::now() - start;

			// While timeout is not reached, and the solver didn't satisfied
			// all constraints OR it is working on an optimization problem, continue the search.
			while( elapsed_time.count() < timeout
			       && ( _best_sat_error > 0. || ( _best_sat_error == 0. && _is_optimization ) ) )
			{
				++search_iterations;
				
				// Estimate which variables need to be changed
#if !defined(EXPERIMENTAL)
				compute_worst_variables();

				if( _number_worst_variables > 1 )
					_worst_variable = _rng.pick( _worst_variables_list );
				else
					_worst_variable = _worst_variables_list[ 0 ];
#else
				if( _free_variables )
					_worst_variable = _rng.variate<int, std::discrete_distribution>( _error_non_tabu_variables );
				else
					_worst_variable = _rng.variate<int, std::discrete_distribution>( _error_variables );
#endif
				
				// Compute neighbors
				// 1. First, get all subdomains to consider from variables to change.
				std::vector< std::vector<int> > domains( _worst_variables_list.size() );
				int domains_index = 0;
				int cartesian_size = 1;
				for( auto& variable_id : _worst_variables_list )
				{
					int neighborhood_range = std::max( 1, static_cast<int>( std::round( _variables[ variable_id ].get_domain_size() * neighborhood.get_domain_span() ) ) );
					domains[ domains_index ] = _variables[ variable_id ].get_partial_domain( neighborhood_range );
					cartesian_size *= domains[ domains_index++ ].size();
				}

				// 2. Then compute their Cartesian product.
				std::vector<int> current_configuration( _number_variables );
				std::transform( _variables.begin(), _variables.end(), current_configuration.begin(), [&]( const auto& var ){ return var.value; } );
				std::vector< std::vector<int> > candidates( cartesian_size, current_configuration );
				int intermediate_cartesian = 1;
				
				for( int domains_iteration = 0 ; domains_iteration < domains_index ; ++domains_iteration )
				{
					int candidates_index = 0;
					
					for( int repeat_domain = 0 ; repeat_domain < ( cartesian_size / static_cast<int>( domains[ domains_iteration ].size() ) ) ; repeat_domain += intermediate_cartesian )
						for( auto& value : domains[ domains_iteration ] )
							for( int repeat_value = 0 ; repeat_value < intermediate_cartesian ; ++repeat_value )
								candidates[candidates_index++][ _worst_variables_list[domains_iteration] ] = value;

					intermediate_cartesian *= static_cast<int>( domains[ domains_iteration ].size() );
				}
						
				// 3. And finally, randomly extract a Neighborhood::exploration_rate percentage of them
				if( neighborhood.get_exploration_rate() == 1.0 )
					_neighbors = candidates;
				else
				{
					int size_to_explore = std::max( 1, static_cast<int>( std::round( candidates.size() * neighborhood.get_exploration_rate() ) ) );
					_rng.shuffle( candidates );
					_neighbors = std::vector<int>( candidates.begin(), candidates.begin() + size_to_explore );
				}				
				
				// Simulate delta errors (or errors is not Constraint::expert_delta_error method
				// is defined) for each neighbor

				// Select the next current configuration (local move)

				// Apply local changes (or global changes is not Constraint::expert_delta_error
				// method is defined)

				// Update constraint/variables errors and the objective function cost

				// Restart mechanism to escape local minima

				
				elapsed_time = std::chrono::steady_clock::now() - start;
			}

			
			do // optimization loop
			{
				start_opt_loop = std::chrono::steady_clock::now();
				++opt_loop;

#if defined(TRACE)
				std::cout << "Generate new config: ";
				for( auto& v : _variables )
					std::cout << v.get_value() << " ";
				std::cout << "\n";
#endif

				do // satisfaction loop 
				{
					++sat_loop;


#if defined(TRACE)
					std::cout << "Error of constraints: ";
					for( int i = 0; i < _error_constraints.size(); ++i )
						std::cout << "c[" << i << "]=" << _error_constraints[i] << ", ";
					std::cout << "\n";

					std::cout << "Error of variables: ";
					for( int i = 0; i < _error_variables.size(); ++i )
						std::cout << _variables[i].get_name() << "=" << _error_variables[i] << ", ";
					std::cout << "\n";

					std::cout << "Tabu list: ";
					for( unsigned int variable_id = 0; variable_id < _number_variables; ++variable_id )
						if( _weak_tabu_list[ variable_id ] > 0 )
							std::cout << _variables[ variable_id ].get_name() + ":" + _weak_tabu_list[ variable_id ] + " ";
					std::cout << "\n";

					std::cout << "Candidate variables: ";
					for( auto& w : _worst_variables_list )
						std::cout << _variables[ w ].get_name() << " ";
					std::cout << "\n";

					std::cout << "Picked variable: " << _variables[ _worst_variable ].get_name() << "\n";
#endif

			
					if( _neighborhood.is_permutation() )
						permutation_move( _worst_variable, _current_sat_error );
					else
						local_move( _worst_variable, _current_sat_error );

					if( _best_sat_error_opt_loop > _current_sat_error )
					{
						_best_sat_error_opt_loop = _current_sat_error;

#if defined(TRACE)
						std::cout << "New error: " << _current_sat_error << ", New config: ";
						for( unsigned int variable_id = 0; variable_id < _number_variables; ++variable_id )
							std::cout << _variables[ variable_id ].get_value() << " ";
						std::cout << "\n";
#endif
						if( _best_sat_error >= _best_sat_error_opt_loop && _best_sat_error > 0 )
						{
							_best_sat_error = _best_sat_error_opt_loop;
							std::transform( _variables.cbegin(),
							                _variables.cend(),
							                final_solution.begin(),
							                [](const auto& v){ return v.get_value(); } );
						}
						// freeze the variable a bit
						_weak_tabu_list[ _worst_variable ] = tabu_time_selected;
					}
					else // local minima
						// Mark _worst_variable as weak tabu for tabu_time_local_min iterations.
						_weak_tabu_list[ _worst_variable ] = tabu_time_local_min;

					// for rounding errors
					if( _best_sat_error_opt_loop  < 1.0e-10 )
					{
						_best_sat_error_opt_loop = 0;
						_best_sat_error = 0;
					}
			
					elapsed_time_opt_loop = std::chrono::steady_clock::now() - start_opt_loop;
					elapsed_time = std::chrono::steady_clock::now() - start;
				} // satisfaction loop
				while( _best_sat_error_opt_loop > 0. && elapsed_time_opt_loop.count() < sat_timeout && elapsed_time.count() < opt_timeout );

				if( _best_sat_error_opt_loop == 0. )
				{
					_current_opt_cost = _objective->cost();
					if( _best_opt_cost > _current_opt_cost )
					{
						update_better_configuration( _best_opt_cost, _current_opt_cost, final_solution );
				
						start_postprocess = std::chrono::steady_clock::now();
						_objective->postprocess_satisfaction( _variables, _best_opt_cost, final_solution );
						timer_postprocess_sat = std::chrono::steady_clock::now() - start_postprocess;
					}
				}
    
				elapsed_time = std::chrono::steady_clock::now() - start;
			} // optimization loop
			//while( elapsed_time.count() < opt_timeout );
			while( elapsed_time.count() < opt_timeout && ( _is_optimization || _best_opt_cost > 0. ) );

			if( _best_sat_error == 0. && _is_optimization )
			{
				_cost_before_postprocess = _best_opt_cost;

				start_postprocess = std::chrono::steady_clock::now();
				_objective->postprocess_optimization( _variables, _best_opt_cost, final_solution );
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

#if defined(DEBUG) || defined(TRACE) || defined(BENCH)
			std::cout << "############" << "\n";
      
			if( !_is_optimization )
				std::cout << "SATISFACTION run" << "\n";
			else
				std::cout << "OPTIMIZATION run with objective " << _objective->get_name() << "\n";

			std::cout << "Elapsed time: " << elapsed_time.count() / 1000 << "ms\n"
			          << "Satisfaction error: " << _best_sat_error << "\n"
			          << "Number of optization loops: " << opt_loop << "\n"
			          << "Number of satisfaction loops: " << sat_loop << "\n";

			if( _is_optimization )
				std::cout << "Optimization cost: " << _best_opt_cost << "\n"
				          << "Opt Cost BEFORE post-processing: " << _cost_before_postprocess << "\n";
  
			if( timer_postprocess_sat.count() > 0 )
				std::cout << "Satisfaction post-processing time: " << timer_postprocess_sat.count() / 1000 << "\n"; 

			if( timer_postprocess_opt.count() > 0 )
				std::cout << "Optimization post-processing time: " << timer_postprocess_opt.count() / 1000 << "\n"; 

			std::cout << "\n";
#endif
          
			return _best_sat_error == 0.;
		}
	};
}
