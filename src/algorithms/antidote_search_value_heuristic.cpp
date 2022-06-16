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

#include "algorithms/antidote_search_value_heuristic.hpp"
#include "thirdparty/randutils.hpp"

namespace ghost
{
	int AntidoteSearchValueHeuristic::select_value_candidates( int variable_to_change, const SearchUnitData& data, const Model& model, const std::map<int, std::vector<double>>& delta_errors, double& min_conflict ) const
	{
		randutils::mt19937_rng rng; // to optimize
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
		return cumulated_delta_errors_variable_index_correspondance[ index ];		
	}
}
