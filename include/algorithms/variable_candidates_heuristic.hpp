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
 * Copyright (C) 2014-2025 Florian Richoux
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

#pragma once

#include <vector>

#include "../search_unit_data.hpp"
// #include "../macros.hpp"

namespace ghost
{
	namespace algorithms
	{
		/*
		 * Strategy design pattern to implement variable candidates selection heuristics.
		 */
		class VariableCandidatesHeuristic
		{
		protected:
			// Protected string variable for the heuristic name. Used for debug/trace purposes.
			std::string name;

		public:
			VariableCandidatesHeuristic( std::string&& name )
				: name( std::move( name ) )
			{ }

			// Default virtual destructor.
			virtual ~VariableCandidatesHeuristic() = default;

			// Inline function returning the heuristic name.	
			inline std::string get_name() const { return name; }

			/*
			 * Function to compute variable candidates, for instance regarding their projected error.
			 * \param data A reference to the SearchUnitData object containing data about the problem instance and the search state, such as the number of variables and constraints, the current projected error on each variable, etc.
			 * \param rng A reference to the pseudo-random generator, to avoid recreating such object.
			 * \param number_variables_to_sample The number of variables to consider during the search (it will be options.number_variables_to_sample)
			 * \return A vector of ID.
			 */
			virtual std::vector<int> compute_variable_candidates( const SearchUnitData& data,
																														randutils::mt19937_rng& rng,
																														int number_variables_to_sample ) const = 0;
		};
	}
}
