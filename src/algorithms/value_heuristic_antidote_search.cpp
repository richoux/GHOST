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

#include "algorithms/value_heuristic_antidote_search.hpp"
#include "thirdparty/randutils.hpp"

using ghost::algorithms::ValueHeuristicAntidoteSearch;

ValueHeuristicAntidoteSearch::ValueHeuristicAntidoteSearch()
	: ValueHeuristic( "Antidote Search" )
{ }
		
int ValueHeuristicAntidoteSearch::select_value( int variable_to_change,
                                                const SearchUnitData& data,
                                                const Model& model,
                                                randutils::mt19937_rng& rng ) const
{
	auto delta_errors_size = data.delta_errors.size();
	
	std::vector<double> cumulated_delta_errors( delta_errors_size );
	std::vector<double> cumulated_delta_errors_for_distribution( delta_errors_size );
	std::vector<int> cumulated_delta_errors_variable_index_correspondance( delta_errors_size ); // longest variable name ever

	int index = 0;

	for( const auto& deltas : data.delta_errors )
	{
		cumulated_delta_errors[ index ] = std::accumulate( deltas.second.begin(), deltas.second.end(), 0.0 );
		cumulated_delta_errors_variable_index_correspondance[ index ] = deltas.first;
		++index;
	}

	std::transform( cumulated_delta_errors.begin(),
	                cumulated_delta_errors.end(),
	                cumulated_delta_errors_for_distribution.begin(),
	                []( auto delta ){ if( delta >= 0) return 0.0; else return -delta; } );

	if( *std::max_element( cumulated_delta_errors_for_distribution.begin(), cumulated_delta_errors_for_distribution.end() ) == 0.0 )
		index = rng.uniform( 0, static_cast<int>( delta_errors_size ) - 1 );
	else
		index = rng.variate<int, std::discrete_distribution>( cumulated_delta_errors_for_distribution.begin(), cumulated_delta_errors_for_distribution.end() );

	data.update_min_conflict( cumulated_delta_errors[ index ] );
		
	return cumulated_delta_errors_variable_index_correspondance[ index ];		
}
