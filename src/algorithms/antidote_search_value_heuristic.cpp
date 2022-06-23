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

#include <algorithm>
#include <numeric>

#include "algorithms/antidote_search_value_heuristic.hpp"
#include "thirdparty/randutils.hpp"

using ghost::algorithms::AntidoteSearchValueHeuristic;

AntidoteSearchValueHeuristic::AntidoteSearchValueHeuristic()
	: ValueHeuristic( "Antidote Search" )
{ }
		
int AntidoteSearchValueHeuristic::select_value_candidates( int variable_to_change,
                                                           const SearchUnitData& data,
                                                           const Model& model,
                                                           const std::map<int, std::vector<double>>& delta_errors,
                                                           double& min_conflict,
                                                           randutils::mt19937_rng& rng ) const
{
	std::vector<double> cumulated_delta_errors( delta_errors.size() );
	std::vector<double> cumulated_delta_errors_for_distribution( delta_errors.size() );
	std::vector<int> cumulated_delta_errors_variable_index_correspondance( delta_errors.size() ); // longest variable name ever

	int index = 0;

	for( const auto& deltas : delta_errors )
	{
		cumulated_delta_errors[ index ] = std::accumulate( deltas.second.begin(), deltas.second.end(), 0.0 );
		cumulated_delta_errors_variable_index_correspondance[ index ] = deltas.first;

// #if defined GHOST_TRACE
// 			double transformed = cumulated_delta_errors[ index ] >= 0 ? 0.0 : -cumulated_delta_errors[ index ];
// 			if( model.permutation_problem )
// 				COUT << "Error for switching var[" << variable_to_change << "]=" << model.variables[ variable_to_change ].get_value()
// 				     << " with var[" << deltas.first << "]=" << model.variables[ deltas.first ].get_value()
// 				     << ": " << cumulated_delta_errors[ index ] << ", transformed: " << transformed << "\n";
// 			else
// 				COUT << "Error for the value " << deltas.first << ": " << cumulated_delta_errors[ index ] << "\n";
// #endif
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

// #if defined GHOST_TRACE
// 		auto domain_to_explore = model.variables[ variable_to_change ].get_full_domain();
// 		// Remove the current value
// 		domain_to_explore.erase( std::find( domain_to_explore.begin(), domain_to_explore.end(), model.variables[ variable_to_change ].get_value() ) );

// 		auto distrib_value = std::discrete_distribution<int>( cumulated_delta_errors_for_distribution.begin(), cumulated_delta_errors_for_distribution.end() );
// 		std::vector<int> vec_value( domain_to_explore.size(), 0 );
// 		for( int n = 0 ; n < 10000 ; ++n )
// 			++vec_value[ rng.variate<int, std::discrete_distribution>( distrib_value ) ];
// 		std::vector<std::pair<int,int>> vec_value_pair( domain_to_explore.size() );
// 		for( int n = 0 ; n < domain_to_explore.size() ; ++n )
// 			vec_value_pair[n] = std::make_pair( cumulated_delta_errors_variable_index_correspondance[n], vec_value[n] );
// 		std::sort( vec_value_pair.begin(), vec_value_pair.end(), [&](std::pair<int, int> &a, std::pair<int, int> &b){ return a.second > b.second; } );
// 		COUT << "Cumulated delta error distribution (normalized):\n";
// 		for( int n = 0 ; n < domain_to_explore.size() ; ++n )
// 			COUT << "val[" <<  vec_value_pair[ n ].first << "]=" << model.variables[ vec_value_pair[ n ].first ].get_value() << " => " << std::fixed << std::setprecision(3) << static_cast<double>( vec_value_pair[ n ].second ) / 10000 << "\n";
// #endif
		
	return cumulated_delta_errors_variable_index_correspondance[ index ];		
}
