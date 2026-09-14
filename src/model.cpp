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
 * Copyright (C) 2014-2026 Florian Richoux
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

#include "model.hpp"

using ghost::Model;

Model::Model( std::vector<Variable>&& moved_variables,
              std::vector<std::vector<int>>&& moved_domains,
              std::map<int, int>&& domain_of_variable,
              const std::vector<std::shared_ptr<Constraint>>&	constraints,
              const std::shared_ptr<Objective>& objective,
              const std::shared_ptr<AuxiliaryData>& auxiliary_data,
              bool permutation_problem )
	: _domain_of_variable( std::move( domain_of_variable ) ),
	  variables( std::move( moved_variables ) ),
	  domains( std::move( moved_domains ) ),
	  constraints( constraints ),
	  objective( objective ),
	  auxiliary_data( auxiliary_data ),
	  permutation_problem( permutation_problem )
{ }

std::vector<int> Model::get_partial_domain_of_variable( int var_id, int range ) const
{
	auto domain = get_full_domain_of_variable( var_id );
	
	if( range >= static_cast<int>( domain.size() ) )
		return domain;
	else
		if( range <= 0 )
			return std::vector<int>{};
		else
		{
			std::vector<int> partial_domain( range );

			// [---xxxIxxx-]
			//        |
			//        ^
			//      index
			int index = std::distance( domain.cbegin(), std::find( domain.cbegin(), domain.cend(), variables[ var_id ].get_value() ) );
			int start_position = index - static_cast<int>( range / 2 );

			if( start_position >= 0 )
			{
				// [---xxxIxxx-]
				//     |
				//     ^
				// start_position
				if( index + ( range - static_cast<int>( range / 2 ) ) <= static_cast<int>( domain.size() ) )
				{
					std::copy( domain.begin() + start_position,
					           domain.begin() + start_position + range,
					           partial_domain.begin() );
				}
				// [xx----xxxIx]
				//   |
				//   ^
				// end_position
				else
				{
					int end_position = index + ( range - static_cast<int>( range / 2 ) ) - static_cast<int>( domain.size() );
					std::copy( domain.begin(),
					           domain.begin() + end_position,
					           partial_domain.begin() );

					std::copy( domain.begin() + start_position,
					           domain.end(),
					           partial_domain.begin() + end_position );
				}
			}
			// [xIxxx----xx]
			//      |    |
			//      |    ^
			//      | start_position
			//      ^
			//  end_position
			else
			{
				int end_position = index + ( range - static_cast<int>( range / 2 ) );
				// Remember: start_position is negative here
				start_position += static_cast<int>( domain.size() );
				std::copy( domain.begin(),
				           domain.begin() + end_position,
				           partial_domain.begin() );

				std::copy( domain.begin() + start_position,
				           domain.end(),
				           partial_domain.begin() + end_position );
			}

			return partial_domain;
		}
}
