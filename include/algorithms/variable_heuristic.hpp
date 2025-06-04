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
#include "../thirdparty/randutils.hpp"

namespace ghost
{
	namespace algorithms
	{
		/*
		 * Strategy design pattern to implement variable selection heuristics.
		 */
		class VariableHeuristic
		{
		protected:
			// Protected string variable for the heuristic name. Used for debug/trace purposes.
			std::string name;

		public:
			VariableHeuristic( std::string&& name )
				: name( std::move( name ) )
			{ }

			// Default virtual destructor.
			virtual ~VariableHeuristic() = default;

			// Inline function returning the heuristic name.
			inline std::string get_name() const { return name; }

			/*
			 * Function to select, among a vector of candidates, a variable from which the search algorithm will make a local move.
			 * \param candidates A const reference to a double vector to be more generic, allowing for instance a vector of errors, rather than a vector of ID, although it would certainly be often the case in practice.
			 * \param data A reference to the SearchUnitData object containing data about the problem instance and the search state, such as the number of variables and constraints, the current projected error on each variable, etc.
			 * \param rng A reference to the pseudo-random generator, to avoid recreating such object.
			 * \return The index of the selected variable in the candidates vector.
			 */
			virtual int select_variable( const std::vector<double>& candidates, const SearchUnitData& data, randutils::mt19937_rng& rng ) const = 0;
		};
	}
}
