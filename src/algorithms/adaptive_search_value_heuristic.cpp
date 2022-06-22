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

#include <algorithm>

#include "algorithms/adaptive_search_value_heuristic.hpp"
#include "thirdparty/randutils.hpp"

namespace ghost
{
	AdaptiveSearchValueHeuristic::AdaptiveSearchValueHeuristic()
		: ValueHeuristic( "Adaptive Search" )
	{ }
		
	int AdaptiveSearchValueHeuristic::select_value_candidates( int variable_to_change, const SearchUnitData& data, const Model& model, const std::map<int, std::vector<double>>& delta_errors, double& min_conflict ) const
	{
		randutils::mt19937_rng rng; // to optimize
		std::vector<int> candidate_values;
		std::map<int, double> cumulated_delta_errors;
		for( const auto& deltas : delta_errors )
		{
			cumulated_delta_errors[ deltas.first ] = std::accumulate( deltas.second.begin(), deltas.second.end(), 0.0 );
// #if defined GHOST_TRACE
// 			if( model.permutation_problem )
// 				COUT << "Error for switching var[" << variable_to_change << "]=" << model.variables[ variable_to_change ].get_value()
// 				     << " with var[" << deltas.first << "]=" << model.variables[ deltas.first ].get_value()
// 				     << ": " << cumulated_delta_errors[ deltas.first ] << "\n";
// 			else
// 				COUT << "Error for the value " << deltas.first << ": " << cumulated_delta_errors[ deltas.first ] << "\n";
// #endif
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

// #if defined GHOST_TRACE
// 		COUT << "Min conflict value candidates list: " << candidate_values[0];
// 		for( int i = 1 ; i < static_cast<int>( candidate_values.size() ); ++i )
// 			COUT << ", " << candidate_values[i];
// #endif
		
		// if we deal with an optimization problem, find the value minimizing to objective function
		if( data.is_optimization )
		{
			if( model.permutation_problem )
				return static_cast<int>( model.objective->heuristic_value_permutation( variable_to_change, candidate_values ) );
			else
				return model.objective->heuristic_value( variable_to_change, candidate_values );
		}
		else
			return rng.pick( candidate_values );
	}
}
