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

#pragma once

#include <vector>
#include <map>

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
		class ValueHeuristic
		{
		protected:
			// Protected string variable for the heuristic name. Used for debug/trace purposes.
			std::string name;

		public:
			ValueHeuristic( std::string&& name )
				: name( std::move( name ) )
			{ }

			// Default virtual destructor.
			virtual ~ValueHeuristic() = default;

			// Inline function returning the heuristic name.
			inline std::string get_name() const { return name; }

			/*
			 * Function to select a value to assign to the variable currently selected by the search algorithm to make a local move.
			 * \param variable_to_change The index of the variable currently selected by the search algorithm.
			 * \param data A reference to the SearchUnitData object containing data about the problem instance and the search state, such as the number of variables and constraints, the current projected error on each variable, etc.
			 * \param model A reference to the problem model, to get access to the objective function for instance.
			 * \param delta_errors A reference to a map to have the list of delta errors on each variable.
			 * \param min_conflict A non-constant reference to get the minimal conflict value after calling this function.
			 * \param rng A reference to the pseudo-random generator, to avoid recreating such object.
			 * \return The selected value to be assigned to variable_to_change (or the index of a variable in case of permutation moves).
			 */
			virtual int select_value( int variable_to_change,
			                          const SearchUnitData& data,
			                          const Model& model,
			                          const std::map<int, std::vector<double>>& delta_errors,
			                          double& min_conflict,
			                          randutils::mt19937_rng& rng ) const = 0;
		};
	}
}
