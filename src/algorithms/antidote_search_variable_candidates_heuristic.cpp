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

#include "algorithms/antidote_search_variable_candidates_heuristic.hpp"
#include "thirdparty/randutils.hpp"

namespace ghost
{
	AntidoteSearchVariableCandidatesHeuristic::AntidoteSearchVariableCandidatesHeuristic()
		: VariableCandidatesHeuristic( "Antidote Search" )
	{ }

	std::vector<double> AntidoteSearchVariableCandidatesHeuristic::compute_variable_candidates( const SearchUnitData& data ) const
	{
		auto error_variables = data.error_variables;
		
		for( int variable_id = 0; variable_id < data.number_variables; ++variable_id )
			if( data.tabu_list[ variable_id ] > data.local_moves )
				error_variables[ variable_id ] = 0.0;

// #if defined GHOST_TRACE
// 		auto distrib = std::discrete_distribution<int>( error_variables.begin(), error_variables.end() );
// 		std::vector<int> vec( data.number_variables, 0 );
// 		for( int n = 0 ; n < 10000 ; ++n )
// 			++vec[ rng.variate<int, std::discrete_distribution>( distrib ) ];
// 		std::vector<std::pair<int,int>> vec_pair( data.number_variables );
// 		for( int n = 0 ; n < data.number_variables ; ++n )
// 			vec_pair[n] = std::make_pair( n, vec[n] );
// 		std::sort( vec_pair.begin(), vec_pair.end(), [&](std::pair<int, int> &a, std::pair<int, int> &b){ return a.second > b.second; } );
// 		COUT << "\nVariable errors (normalized):\n";
// 		for( auto &v : vec_pair )
// 			COUT << "v[" << v.first << "]: " << std::fixed << std::setprecision(3) << static_cast<double>( v.second ) / 10000 << "\n";
// #endif

		return error_variables;
	}
}
