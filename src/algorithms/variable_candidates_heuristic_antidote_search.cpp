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

#include "algorithms/variable_candidates_heuristic_antidote_search.hpp"
#include "thirdparty/randutils.hpp"

using ghost::algorithms::VariableCandidatesHeuristicAntidoteSearch;

VariableCandidatesHeuristicAntidoteSearch::VariableCandidatesHeuristicAntidoteSearch()
	: VariableCandidatesHeuristic( "Antidote Search" )
{ }

std::vector<int> VariableCandidatesHeuristicAntidoteSearch::compute_variable_candidates( const SearchUnitData& data, randutils::mt19937_rng& rng ) const
{
	data.make_error_distribution();
		
	for( int variable_id = 0; variable_id < data.number_variables; ++variable_id )
		if( data.tabu_list[ variable_id ] > data.local_moves )
			data.erase_error_at( variable_id );

	// return a (non empty) dummy vector; only computing error_distribution is important here
	// returning an empty vector would trigger a reset
	return std::vector<int>{-1};
}
