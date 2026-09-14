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

#pragma once

#include <vector>
#include <memory>
#include <algorithm>

#include "variable.hpp"
#include "constraint.hpp"
#include "objective.hpp"
#include "auxiliary_data.hpp"

namespace ghost
{
	struct Model final
	{
	private:
		std::map<int, int> _domain_of_variable; // map<var id, domain index>

	public:
		std::vector<Variable> variables; 
		std::vector<std::vector<int>> domains;
		std::vector<std::shared_ptr<Constraint>> constraints;
		std::shared_ptr<Objective> objective;
		std::shared_ptr<AuxiliaryData> auxiliary_data;
		bool permutation_problem;

		Model() = default;

		Model( std::vector<Variable>&& variables,
		       std::vector<std::vector<int>>&& domains,
		       std::map<int, int>&& domain_of_variable,
		       const std::vector<std::shared_ptr<Constraint>>&	constraints,
		       const std::shared_ptr<Objective>& objective,
		       const std::shared_ptr<AuxiliaryData>& auxiliary_data,
		       bool permutation_problem );

		// Assign to the variable a random values from its domain.
		inline void set_random_value_to_variable( int var_id, randutils::mt19937_rng& rng ) {	variables[var_id].set_value( rng.pick( domains[ _domain_of_variable[ var_id ] ] ) ); }

		/*
		 * Inline method returning the domain.
		 *
		 * \return The vector of integers composing the domain.
		 */
		inline std::vector<int> get_full_domain_of_variable( int var_id ) const { return domains[ _domain_of_variable.at( var_id ) ]; }

		/*
		 * Method returning the range of values
		 * [current_value - range/2 [mod domain_size], current_value + range/2 [mod domain_size]]
		 * from the domain.
		 *
		 * \return A vector containing these integers.
		 */
		std::vector<int> get_partial_domain_of_variable( int var_id, int range ) const;

		/*
		 * Inline method returning the size of the domain of the variable.
		 *
		 * \return A size_t equals to size of the domain of the variable.
		 */
		inline std::size_t get_domain_size_of_variable( int var_id ) const { return domains[ _domain_of_variable.at( var_id ) ].size(); }

		/*
		 * Inline method returning the minimal value in the variable's domain.
		 *
		 * \return The minimal value in the domain, in constant time.
		 */
		inline int get_domain_min_value_of_variable( int var_id ) const { return *domains[ _domain_of_variable.at( var_id ) ].begin(); }

		/*
		 * Inline method returning the maximal value in the variable's domain.
		 *
		 * \return The maximal value in the domain, in constant time.
		 */
		inline int get_domain_max_value_of_variable( int var_id ) const { return *domains[ _domain_of_variable.at( var_id ) ].rend(); }
	};
}
