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

#include <cassert>
#include "algorithms/space_policy.hpp"

#include "algorithms/error_projection_algorithm_adaptive_search.hpp"

using ghost::algorithms::SpacePolicy;

SpacePolicy::SpacePolicy( std::string&& name,
                          std::unique_ptr<algorithms::ErrorProjection> error_projection,
													bool switch_space_instead_reset,
                          int index_space_pool )
	: name( std::move( name ) ),
	  error_projection( std::move( error_projection ) ),
		switch_space_instead_reset( switch_space_instead_reset ),
	  index_space_pool( index_space_pool )
{ }

SpacePolicy::SpacePolicy( std::string&& name,
													bool switch_space_instead_reset )
	: SpacePolicy( std::move( name ),
	               std::make_unique<algorithms::ErrorProjectionAdaptiveSearch>(),
								 switch_space_instead_reset )
{ }

void SpacePolicy::initialize_data_structures( const SearchUnitData& data ) const
{
	assert(error_projection);
	error_projection->initialize_data_structures( data );
}

void SpacePolicy::compute_variable_errors( const std::vector<Variable>& variables,
                                           const std::vector<std::shared_ptr<Constraint>>& constraints,
                                           SearchUnitData& data ) const
{
	error_projection->compute_variable_errors( variables,
	                                           constraints,
	                                           data );
}

void SpacePolicy::switch_space() { }
