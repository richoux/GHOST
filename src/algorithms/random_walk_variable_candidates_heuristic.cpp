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

#include "algorithms/random_walk_variable_candidates_heuristic.hpp"

using ghost::algorithms::RandomWalkVariableCandidatesHeuristic;

RandomWalkVariableCandidatesHeuristic::RandomWalkVariableCandidatesHeuristic()
	: VariableCandidatesHeuristic( "Random Walk" )
{ }
		
std::vector<double> RandomWalkVariableCandidatesHeuristic::compute_variable_candidates( const SearchUnitData& data ) const
{
	std::vector<double> full_variables_list(data.number_variables);
	std::iota( full_variables_list.begin(), full_variables_list.end(), 0.0 );
	return full_variables_list;
}
