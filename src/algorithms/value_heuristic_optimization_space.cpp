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

#include "algorithms/value_heuristic_optimization_space.hpp"

using ghost::algorithms::ValueHeuristicOptimizationSpace;
using ghost::SearchUnitData;
using ghost::Model;

ValueHeuristicOptimizationSpace::ValueHeuristicOptimizationSpace()
	: ValueHeuristic( "Optimization Space" )
{ }

int ValueHeuristicOptimizationSpace::select_value( int variable_to_change,
                                                   const SearchUnitData& data,
                                                   const Model& model,
                                                   randutils::mt19937_rng& rng ) const
{
	// The goal is to find the neighbor with the lowest objective cost, while keeping track of the constraint delta errors
	
	std::vector<int> candidates; // variable indexes for permutation problems, variable values otherwise
	std::map<int, double> cumulated_delta_errors;
	double min_cost = std::numeric_limits<double>::max();
	double simulated_cost = 0.0;
	auto vars = model.objective->_variables;
	int backup = vars[ variable_to_change ]->get_value();

	for( const auto& deltas : data.delta_errors )
		cumulated_delta_errors[ deltas.first ] = std::accumulate( deltas.second.begin(), deltas.second.end(), 0.0 );

	if( model.permutation_problem )
	{
		int tmp;
		auto head = cumulated_delta_errors.begin();
		auto tail = cumulated_delta_errors.begin();
		
		while( head != cumulated_delta_errors.end() )
		{
			tmp = vars[ variable_to_change ]->get_value();
			vars[ variable_to_change ]->set_value( vars[ head->first ]->get_value() );
			vars[ head->first ]->set_value( backup );
			vars[ tail->first ]->set_value( tmp );

			simulated_cost = model.objective->cost();

			if( model.objective->_is_maximization )
				simulated_cost = -simulated_cost;

			if( min_cost > simulated_cost )
			{
				min_cost = simulated_cost;
				candidates.clear();
				candidates.push_back( head->first );
			}
			else
				if( min_cost == simulated_cost )
					candidates.push_back( head->first );

			tail = head; // first iteration, tail remains at index 0
			++head;
		}
		vars[ tail->first ]->set_value( vars[ variable_to_change ]->get_value() );
		vars[ variable_to_change ]->set_value( backup );
	}
	else // not a permutation problem
	{
		for( const auto& deltas : cumulated_delta_errors )
		{
			vars[ variable_to_change ]->set_value( deltas.first );
			simulated_cost = model.objective->cost();
			if( model.objective->_is_maximization )
				simulated_cost = -simulated_cost;
			if( min_cost > simulated_cost )
			{
				min_cost = simulated_cost;
				candidates.clear();
				candidates.push_back( deltas.first );
			}
			else
				if( min_cost == simulated_cost )
					candidates.push_back( deltas.first );
		}

		vars[ variable_to_change ]->set_value( backup );
	}

	data.update_delta_cost( min_cost - data.current_opt_cost );

	auto candidate = rng.pick( candidates );
	data.update_min_conflict( cumulated_delta_errors[candidate] );
	
	return candidate;
}
