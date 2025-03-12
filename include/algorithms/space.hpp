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

#include <memory>
#include "../search_unit_data.hpp"

namespace ghost
{
	namespace algorithms
	{
		/*
		 * Strategy design pattern to implement different search spaces.
		 */
		class Space
		{
		protected:
			// Protected string variable for the space name. Used for debug/trace purposes.
			std::string name;
			bool violation_space;
			
		public:
			Space( std::string&& name,
			       bool violation_space );

			virtual ~Space() = default;

			inline std::string get_name() const { return name; }
			inline bool is_violation_space() const { return violation_space; }

			/*
			 * Function returning the fitness variation the search aims to minimize while exploring the search space.
			 * Indeed, this value depends on the value heuristics. Most of the time, we will explore
			 * the violation space, thus the value to minimize is SearchUnitData::min_conflict (value outputed by default).
			 * But when exploring the optimization space, SearchUnitData::delta_cost should be returned.
			 * \param data A reference to the SearchUnitData object containing data about the problem instance and the search state, such as the number of variables and constraints, the current projected error on each variable, etc.
			 * \return The value the search unit is trying to minimize while exploring the search space.
			 */
			virtual double get_fitness_variation( const SearchUnitData& data ) const = 0;
			
			/*
			 * Procedure updating the current fitness of the search space in data.
			 * This corresponds to current_sat_error on a violation space, and current_opt_cost on an optimization space.
			 * Those values are updated by adding min_conflict or delta_cost, respectively.
			 * \param data A reference to the SearchUnitData object containing data about the problem instance and the search state, such as the number of variables and constraints, the current projected error on each variable, etc.
			 */
			virtual void update_fitness( const SearchUnitData& data ) const = 0;
		};
	}
}
