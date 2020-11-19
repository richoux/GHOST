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

#include "solver.hpp"

using namespace ghost;

Solver::Solver( const std::vector<Variable>&	variables, 
                const std::vector<std::shared_ptr<Constraint>>&	constraints,
                std::unique_ptr<Objective>	objective,
                bool permutation_problem )
	: _variables ( variables ), 
	  _constraints ( constraints ),
	  _objective ( std::move( objective ) ),
	  _is_optimization ( _objective == nullptr ? false : true ),
	  _permutation_problem ( permutation_problem ),
	  _number_variables ( variables.size() )
{
	for( const auto& var : variables )
		for( auto& ctr : constraints )
			if( ctr->has_variable( var ) )
				_map_var_ctr[ var._id ].push_back( ctr );
}

Solver::Solver( const std::vector<Variable>&	variables, 
                const std::vector<std::shared_ptr<Constraint>>&	constraints,
                bool permutation_problem )
	: Solver( variables, constraints, nullptr, permutation_problem )
{ }

bool Solver::solve( double&	final_cost,
                    std::vector<int>& final_solution,
                    double sat_timeout,
                    double opt_timeout,
                    bool no_random_starting_point )
{
	//sat_timeout *= 1000; // timeouts in microseconds
	if( opt_timeout == 0 )
		opt_timeout = sat_timeout * 10;
	//else
	//opt_timeout *= 1000;

	// The only parameter of Solver<Variable, Constraint>::solve outside timeouts
	int tabu_time_local_min = std::max( 1, _number_variables / 2); // _number_variables - 1;
	int tabu_time_selected = std::max( 1, tabu_time_local_min / 2);

	std::chrono::duration<double,std::micro> elapsed_time(0);
	std::chrono::duration<double,std::micro> elapsed_time_opt_loop(0);
	std::chrono::time_point<std::chrono::steady_clock> start;
	std::chrono::time_point<std::chrono::steady_clock> start_opt_loop;
	std::chrono::time_point<std::chrono::steady_clock> start_postprocess;
	start = std::chrono::steady_clock::now();

	std::chrono::duration<double,std::micro> timer_postprocess_sat(0);
	std::chrono::duration<double,std::micro> timer_postprocess_opt(0);

	if( _objective == nullptr )
		_objective = std::make_unique<NullObjective>( _variables );
    
	int opt_loop = 0;
	int sat_loop = 0;

	double cost_before_postprocess = std::numeric_limits<double>::max();
  
	Variable* worst_variable;
	double current_sat_cost;
	double current_opt_cost;
	std::map< int, double > cost_constraints;
	std::map< int, double > cost_variables;
	std::map< int, double > cost_non_tabu_variables;

	// In case final_solution is not a vector of the correct size,
	// ie, equals to the number of variables.
	final_solution.resize( _number_variables );
  
	_best_sat_cost = std::numeric_limits<double>::max();
	_best_opt_cost = std::numeric_limits<double>::max();
  
	do // optimization loop
	{
		start_opt_loop = std::chrono::steady_clock::now();
		++opt_loop;

		// start from a random configuration, if no_random_starting_point is false
		if( !no_random_starting_point )
			set_initial_configuration( 10 );
		else
			// From the second turn in this loop, start from a random configuration
			// TODO: What if the user REALLY wants to start searching from his/her own starting point?
			no_random_starting_point = false;

#if defined(TRACE)
		std::cout << "Generate new config: ";
		for( auto& v : _variables )
			std::cout << v.get_value() << " ";
		std::cout << "\n";
#endif
		
		// Reset weak tabu list
		_weak_tabu_list.clear();

		// Reset the best satisfaction cost
		_best_sat_cost_opt_loop = std::numeric_limits<double>::max();

		do // satisfaction loop 
		{
			++sat_loop;

			// Reset variables and constraints costs
			for( auto& c : _constraints )
				cost_constraints.at( c->_id ) = 0.0;

			for( auto& v : _variables )
				cost_variables.at( v._id ) = 0.0;

			current_sat_cost = compute_constraints_costs( cost_constraints );

#if defined(TRACE)
			std::cout << "Cost of constraints: ";
			for( int i = 0; i < cost_constraints.size(); ++i )
				std::cout << "c[" << i << "]=" << cost_constraints[i] << ", ";
			std::cout << "\n";
#endif

			compute_variables_costs( cost_constraints, cost_variables, cost_non_tabu_variables, current_sat_cost );

#if defined(TRACE)
			std::cout << "Cost of variables: ";
			for( int i = 0; i < cost_variables.size(); ++i )
				std::cout << _variables[i].get_name() << "=" << cost_variables[i] << ", ";
			std::cout << "\n";
#endif
			
			bool free_variables = false;
			decay_weak_tabu_list( free_variables );

#if defined(TRACE)
			std::cout << "Tabu list: ";
			std::for_each( _weak_tabu_list.cbegin(),
			               _weak_tabu_list.cend(),
			               [](const auto& t){std::cout << " " << t.second;} );
			std::cout << "\n";
#endif

#if !defined(EXPERIMENTAL)
			auto worst_variable_list = compute_worst_variables( free_variables, cost_variables );

#if defined(TRACE)
			std::cout << "Candidate variables: ";
			for( auto& w : worst_variable_list )
				std::cout << w->get_name() << " ";
			std::cout << "\n";
#endif

			if( worst_variable_list.size() > 1 )
				worst_variable = _rng.pick( worst_variable_list );
			else
				worst_variable = worst_variable_list[0];

#if defined(TRACE)
			std::cout << "Picked variable: " << worst_variable->get_name() << "\n";
#endif

#else
			if( free_variables )
			{
				// discrete_distribution<int> distribution { cost_non_tabu_variables.begin(), cost_non_tabu_variables.end() };
				// worst_variable = &(_variables[ distribution( rng ) ]);
				worst_variable = &(_variables[ _rng.variate<int, std::discrete_distribution>( cost_non_tabu_variables ) ]);
			}
			else
			{
				// discrete_distribution<int> distribution { cost_variables.begin(), cost_variables.end() };
				// worst_variable = &(_variables[ distribution( rng ) ]);
				worst_variable = &(_variables[ _rng.variate<int, std::discrete_distribution>( cost_variables ) ]);
			}      
#endif
			
			if( _permutation_problem )
				permutation_move( worst_variable, cost_constraints, cost_variables, cost_non_tabu_variables, current_sat_cost );
			else
				local_move( worst_variable, cost_constraints, cost_variables, cost_non_tabu_variables, current_sat_cost );

			if( _best_sat_cost_opt_loop > current_sat_cost )
			{
				_best_sat_cost_opt_loop = current_sat_cost;

#if defined(TRACE)
				std::cout << "New cost: " << current_sat_cost << ", New config: ";
				for( auto& v : _variables )
					std::cout << v.get_value() << " ";
				std::cout << "\n";
#endif
				if( _best_sat_cost >= _best_sat_cost_opt_loop && _best_sat_cost > 0 )
				{
					_best_sat_cost = _best_sat_cost_opt_loop;
					std::transform( _variables.cbegin(),
					                _variables.cend(),
					                final_solution.begin(),
					                [](const auto& v){ return v.get_value(); } );
				}
				// freeze the variable a bit
				_weak_tabu_list.at( worst_variable->_id ) = tabu_time_selected;
			}
			else // local minima
				// Mark worst_variable as weak tabu for tabu_time_local_min iterations.
				_weak_tabu_list.at( worst_variable->_id ) = tabu_time_local_min;

			// for rounding errors
			if( _best_sat_cost_opt_loop  < 1.0e-10 )
			{
				_best_sat_cost_opt_loop = 0;
				_best_sat_cost = 0;
			}
			
			elapsed_time_opt_loop = std::chrono::steady_clock::now() - start_opt_loop;
			elapsed_time = std::chrono::steady_clock::now() - start;
		} // satisfaction loop
		while( _best_sat_cost_opt_loop > 0. && elapsed_time_opt_loop.count() < sat_timeout && elapsed_time.count() < opt_timeout );

		if( _best_sat_cost_opt_loop == 0. )
		{
			current_opt_cost = _objective->cost( _variables );
			if( _best_opt_cost > current_opt_cost )
			{
				update_better_configuration( _best_opt_cost, current_opt_cost, final_solution );
				
				start_postprocess = std::chrono::steady_clock::now();
				_objective->postprocess_satisfaction( _variables, _best_opt_cost, final_solution );
				timer_postprocess_sat = std::chrono::steady_clock::now() - start_postprocess;
			}
		}
    
		elapsed_time = std::chrono::steady_clock::now() - start;
	} // optimization loop
	//while( elapsed_time.count() < opt_timeout );
	while( elapsed_time.count() < opt_timeout && ( _is_optimization || _best_opt_cost > 0. ) );

	if( _best_sat_cost == 0. && _is_optimization )
	{
		cost_before_postprocess = _best_opt_cost;

		start_postprocess = std::chrono::steady_clock::now();
		_objective->postprocess_optimization( _variables, _best_opt_cost, final_solution );
		timer_postprocess_opt = std::chrono::steady_clock::now() - start_postprocess;							     
	}

	if( _is_optimization )
	{
		if( _best_opt_cost < 0 )
		{
			_best_opt_cost = -_best_opt_cost;
			cost_before_postprocess = -cost_before_postprocess;
		}
    
		final_cost = _best_opt_cost;
	}
	else
		final_cost = _best_sat_cost;

	// Set the variables to the best solution values.
	// Useful if the user prefer to directly use the vector of Variables
	// to manipulate and exploit the solution.
	for( int i = 0 ; i < _number_variables; ++i )
		_variables[ i ].set_value( final_solution[ i ] );

#if defined(DEBUG) || defined(TRACE) || defined(BENCH)
	std::cout << "############" << "\n";
      
	if( !_is_optimization )
		std::cout << "SATISFACTION run" << "\n";
	else
		std::cout << "OPTIMIZATION run with objective " << _objective->get_name() << "\n";

	std::cout << "Elapsed time: " << elapsed_time.count() / 1000 << "ms\n"
	          << "Satisfaction cost: " << _best_sat_cost << "\n"
	          << "Number of optization loops: " << opt_loop << "\n"
	          << "Number of satisfaction loops: " << sat_loop << "\n";

	if( _is_optimization )
		std::cout << "Optimization cost: " << _best_opt_cost << "\n"
		          << "Opt Cost BEFORE post-processing: " << cost_before_postprocess << "\n";
  
	if( timer_postprocess_sat.count() > 0 )
		std::cout << "Satisfaction post-processing time: " << timer_postprocess_sat.count() / 1000 << "\n"; 

	if( timer_postprocess_opt.count() > 0 )
		std::cout << "Optimization post-processing time: " << timer_postprocess_opt.count() / 1000 << "\n"; 

	std::cout << "\n";
#endif
          
	return _best_sat_cost == 0.;
}

double Solver::compute_constraints_costs( std::vector<double>& cost_constraints ) const
{
	double satisfaction_cost = 0.;
	double cost;
  
	for( auto& c : _constraints )
	{
		cost = c->cost();
		cost_constraints.at( c->_id ) = cost;
		satisfaction_cost += cost;    
	}

	return satisfaction_cost;
}

void Solver::compute_variables_costs( const std::vector<double>&	cost_constraints,
                                      std::vector<double>&	cost_variables,
                                      std::vector<double>&	cost_non_tabu_variables,
                                      const double current_sat_cost ) const
{
	int id;
	
	for( auto& v : _variables )
	{
		for( auto& c : _map_var_ctr[ v._id ] )
			cost_variables.at( v._id ) += cost_constraints.at( c->_id );

		if( _weak_tabu_list.at( v._id ) == 0 )
			cost_non_tabu_variables.at( v._id ) = cost_variables.at( v._id );
		else
			cost_non_tabu_variables.at( v._id ) = 0.0;
	}
}

void Solver::set_initial_configuration( int samplings )
{
	if( !_permutation_problem )
	{
		if( samplings == 1 )
		{
			monte_carlo_sampling();
		}
		else
		{
			// To avoid weird samplings numbers like 0 or -1
			samplings = std::max( 2, samplings );
    
			double best_sat_cost = std::numeric_limits<double>::max();
			double current_sat_cost;

			std::vector<int> best_values( _number_variables, 0 );
    
			for( int i = 0 ; i < samplings ; ++i )
			{
				monte_carlo_sampling();
				current_sat_cost = 0.;
				for( auto& c : _constraints )
					current_sat_cost += c->cost();
      
				if( best_sat_cost > current_sat_cost )
					update_better_configuration( best_sat_cost, current_sat_cost, best_values );

				if( current_sat_cost == 0. )
					break;
			}

			for( int i = 0; i < _number_variables; ++i )
				_variables[ i ].set_value( best_values[ i ] );
		}
	}
	else
	{
		// To avoid weird samplings numbers like 0 or -1
		samplings = std::max( 1, samplings );
		
		double best_sat_cost = std::numeric_limits<double>::max();
		double current_sat_cost;
		
		std::vector<int> best_values( _number_variables, 0 );
		
		for( int i = 0 ; i < samplings ; ++i )
		{
			random_permutations();
			current_sat_cost = 0.;
			for( auto& c : _constraints )
				current_sat_cost += c->cost();
			
			if( best_sat_cost > current_sat_cost )
				update_better_configuration( best_sat_cost, current_sat_cost, best_values );
			
			if( current_sat_cost == 0. )
				break;
		}
		
		for( int i = 0; i < _number_variables; ++i )
			_variables[ i ].set_value( best_values[ i ] );
	}
}

void Solver::monte_carlo_sampling()
{
	for( auto& v : _variables )
		v.pick_random_value();
}

void Solver::random_permutations()
{
	for( unsigned int i = 0; i < _variables.size() - 1; ++i )
		for( unsigned int j = i + 1; j < _variables.size(); ++j )
		{
			// About 50% to do a swap for each couple (var_i, var_j)
			if( _rng.uniform( 0, 1 ) == 0 )
			{
				std::swap( _variables[i]._index, _variables[j]._index );
				std::swap( _variables[i]._cache_value, _variables[j]._cache_value );
			}
		}
}

void Solver::decay_weak_tabu_list( bool& free_variables ) 
{
	std::for_each( _weak_tabu_list.begin(),
	               _weak_tabu_list.end(),
	               [](auto& t){ t.second == 0 ? free_variables : --t.second; assert( t.second >= 0); } );
}

void Solver::update_better_configuration( double&	best,
                                          const double current,
                                          std::vector<int>& configuration )
{
	best = current;

	for( int i = 0; i < _number_variables; ++i )
		configuration[ i ] = _variables[ i ].get_value();
}

#if !defined(EXPERIMENTAL)
std::vector< Variable* > Solver::compute_worst_variables( bool free_variables,
                                                          const std::vector<double>& cost_variables )
{
	// Here, we look at neighbor configurations with the lowest cost.
	std::vector< Variable* > worst_variable_list;
	double worst_variableCost = 0.;
	int id;
  
	for( auto& v : _variables )
	{
		if( !free_variables || _weak_tabu_list.at( v._id ) == 0 )
		{
			if( worst_variableCost < cost_variables[ v._id ] )
			{
				worst_variableCost = cost_variables[ v._id ];
				worst_variable_list.clear();
				worst_variable_list.push_back( &v );
			}
			else 
				if( worst_variableCost == cost_variables[ v._id ] )
					worst_variable_list.push_back( &v );	  
		}
	}
  
	return worst_variable_list;
}
#endif
  
// NO VALUE BACKED-UP!
double Solver::simulate_local_move_cost( Variable* variable,
                                         double	value,
                                         const std::vector<double>& cost_constraints,
                                         double	current_sat_cost ) const
{
	double new_current_sat_cost = current_sat_cost;

	variable->set_value( value );
	for( auto& c : _map_var_ctr[ variable._id ] )
		new_current_sat_cost += ( c->cost() - cost_constraints.at( c->_id ) );

	return new_current_sat_cost;
}

double Solver::simulate_permutation_cost( Variable*	worst_variable,
                                          Variable&	other_variable,
                                          const std::vector<double>&	cost_constraints,
                                          double current_sat_cost ) const
{
	double new_current_sat_cost = current_sat_cost;
	std::map<int, bool> done;
	for( auto& c : _constraints )
		done.at( c->_id ) = false;

	std::swap( worst_variable->_index, other_variable._index );
	std::swap( worst_variable->_cache_value, other_variable._cache_value );
    
	for( auto& c : _map_var_ctr[ worst_variable._id ] )
	{
		
		new_current_sat_cost += ( c->cost() - cost_constraints.at( c->_id ) );
		done.at( c->_id ) = true;
	}

	// The following was commented to avoid branch misses, but it appears to be slower than
	// the commented block that follows.
	for( auto& c : _map_var_ctr[ other_variable._id ] )
		if( !done.at( c->_id ) )
			new_current_sat_cost += ( c->cost() - cost_constraints.at( c->_id ) );

	// vector< shared_ptr<Constraint> > diff;
	// std::set_difference( _map_var_ctr[ other_variable ].begin(), _map_var_ctr[ other_variable ].end(),
	//                      _map_var_ctr[ *worst_variable ].begin(), _map_var_ctr[ *worst_variable ].end(),
	//                      std::inserter( diff, diff.begin() ) );

	// for( auto& c : diff )
	// 	new_current_sat_cost += ( c->cost() - cost_constraints[ c->get_id() - _ctr_offset ] );

	// We must roll back to the previous state before returning the new cost value. 
	std::swap( worst_variable->_index, other_variable._index );
	std::swap( worst_variable->_cache_value, other_variable._cache_value );

	return new_current_sat_cost;
}

void Solver::local_move( Variable* variable,
                         std::vector<double>& cost_constraints,
                         std::vector<double>& cost_variables,
                         std::vector<double>& cost_non_tabu_variables,
                         double& current_sat_cost )
{
	// Here, we look at values in the variable domain
	// leading to the lowest satisfaction cost.
	double new_current_sat_cost = 0.0;
	std::vector< int > best_values_list;
	int best_value;
	double best_cost = std::numeric_limits<double>::max();
  
	for( auto& val : variable->possible_values() )
	{
		new_current_sat_cost = simulate_local_move_cost( variable, val, cost_constraints, current_sat_cost );
		if( best_cost > new_current_sat_cost )
		{
			best_cost = new_current_sat_cost;
			best_values_list.clear();
			best_values_list.push_back( val );
		}
		else 
			if( best_cost == new_current_sat_cost )
				best_values_list.push_back( val );	  
	}

	// If several values lead to the same best satisfaction cost,
	// call Objective::heuristic_value has a tie-break.
	// By default, Objective::heuristic_value returns the value
	// improving the most the optimization cost, or a random value
	// among values improving the most the optimization cost if there
	// are some ties.
	if( best_values_list.size() > 1 )
		best_value = _objective->heuristic_value( _variables, *variable, best_values_list );
	else
		best_value = best_values_list[0];

	variable->set_value( best_value );
	current_sat_cost = best_cost;
	// for( auto& c : _map_var_ctr[ *variable ] )
	//   cost_constraints[ c->get_id() - _ctr_offset ] = c->cost();

	// compute_variables_costs( cost_constraints, cost_variables, cost_non_tabu_variables, current_sat_cost );
}

void Solver::permutation_move( Variable* variable,
                               std::vector<double>& cost_constraints,
                               std::vector<double>& cost_variables,
                               std::vector<double>& cost_non_tabu_variables,
                               double& current_sat_cost )
{
	// Here, we look at values in the variable domain
	// leading to the lowest satisfaction cost.
	double new_current_sat_cost = 0.0;
	std::vector< Variable* > best_var_to_swap_list;
	Variable* best_var_to_swap;
	double best_cost = std::numeric_limits<double>::max();

#if defined(TRACE)
	std::cout << "Current cost before permutation: " << current_sat_cost << "\n";
#endif

	
	for( auto& other_variable : _variables )
	{
		// Next line is replaced by a simpler conditional since there were A LOT of branch misses!
		//if( other_variable._id == variable->_id || other_variable._index == variable->_index )
		if( other_variable._id == variable->_id )
			continue;
    
		new_current_sat_cost = simulate_permutation_cost( variable, other_variable, cost_constraints, current_sat_cost );

#if defined(TRACE)
		std::cout << "Cost if permutation between " << variable->_id << " and " << other_variable._id << ": " << new_current_sat_cost << "\n";
#endif

		if( best_cost > new_current_sat_cost )
		{
#if defined(TRACE)
			std::cout << "This is a new best cost.\n";
#endif
			
			best_cost = new_current_sat_cost;
			best_var_to_swap_list.clear();
			best_var_to_swap_list.push_back( &other_variable );
		}
		else 
			if( best_cost == new_current_sat_cost )
			{
				best_var_to_swap_list.push_back( &other_variable );
#if defined(TRACE)
				std::cout << "Tie cost with the best one.\n";
#endif
			}
	}

	// // If the best cost found so far leads to a plateau,
	// // then we have 10% of chance to escapte from the plateau
	// // by picking up a random variable (giving a worst global cost)
	// // to permute with.
	// if( best_cost == current_sat_cost && _rng.uniform( 0, 99 ) < 10 )
	// {
	// 	do
	// 	{
	// 		best_var_to_swap = _rng.pick( _variables );
	// 	} while( best_var_to_swap._id == variable->_id || std::find_if( best_var_to_swap_list.begin(), best_var_to_swap_list.end(), [&best_var_to_swap](auto& v){ return v._id == best_var_to_swap._id; } ) != best_var_to_swap_list.end() );
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
	std::cout << "Permutation will be done between " << variable->_id << " and " << best_var_to_swap->_id << ".\n";
#endif

	std::swap( variable->_index, best_var_to_swap->_index );
	std::swap( variable->_cache_value, best_var_to_swap->_cache_value );

	current_sat_cost = best_cost;
	// vector<bool> compted( cost_constraints.size(), false );
  
	// for( auto& c : _map_var_ctr[ *variable ] )
	// {
	// 	new_current_sat_cost += ( c->cost() - cost_constraints[ c->get_id() - _ctr_offset ] );
	// 	compted[ c->get_id() - _ctr_offset ] = true;
	// }
  
	// for( auto& c : _map_var_ctr[ best_var_to_swap ] )
	// 	if( !compted[ c->get_id() - _ctr_offset ] )
	// 		new_current_sat_cost += ( c->cost() - cost_constraints[ c->get_id() - _ctr_offset ] );

	// compute_variables_costs( cost_constraints, cost_variables, cost_non_tabu_variables, new_current_sat_cost );
}
