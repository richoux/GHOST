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

#include "algorithms/adaptive_search_value_heuristic.hpp"

using ghost::algorithms::AdaptiveSearchValueHeuristic;
using ghost::SearchUnitData;
using ghost::Model;

AdaptiveSearchValueHeuristic::AdaptiveSearchValueHeuristic()
	: ValueHeuristic( "Adaptive Search" )
{ }
		
int AdaptiveSearchValueHeuristic::select_value( int variable_to_change,
                                                const SearchUnitData& data,
                                                const Model& model,
                                                randutils::mt19937_rng& rng ) const
{
	std::vector<int> candidates; // variable indexes for permutation problems, variable values otherwise
	std::map<int, double> cumulated_delta_errors;
	for( const auto& deltas : data.delta_errors )
		cumulated_delta_errors[ deltas.first ] = std::accumulate( deltas.second.begin(), deltas.second.end(), 0.0 );

	for( const auto& deltas : cumulated_delta_errors )
	{
		if( data.min_conflict > deltas.second )
		{
			candidates.clear();
			candidates.push_back( deltas.first );
			data.update_min_conflict( deltas.second );
		}
		else
			if( data.min_conflict == deltas.second )
				candidates.push_back( deltas.first );
	}

	if( candidates.empty() )
		return variable_to_change;

	// if we deal with an optimization problem, find the value minimizing to objective function
	if( data.is_optimization )
	{
		if( model.permutation_problem )
			return static_cast<int>( model.objective->heuristic_value_permutation( variable_to_change, candidates, rng ) );
		else
			return model.objective->heuristic_value( variable_to_change, candidates, rng );
	}
	else
		return rng.pick( candidates );
}
