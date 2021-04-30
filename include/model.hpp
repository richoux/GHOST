/*
 * GHOST (General meta-Heuristic Optimization Solving Tool) is a C++ library
 * designed to help developers to model and implement optimization problem
 * solving. It contains a meta-heuristic solver aiming to solve any kind of
 * combinatorial and optimization real-time problems represented by a CSP/COP/CFN.
 *
 * GHOST has been first developped to help making AI for the RTS game
 * StarCraft: Brood war, but can be used for any kind of applications where
 * solving combinatorial and optimization problems within some tenth of
 * milliseconds is needed. It is a generalization of the Wall-in project.
 * Please visit https://github.com/richoux/GHOST for further information.
 *
 * Copyright (C) 2014-2021 Florian Richoux
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

#include "variable.hpp"
#include "constraint.hpp"
#include "objective.hpp"

namespace ghost
{
	class NullObjective;

	template<typename ObjectiveType = NullObjective, typename ... ConstraintType>
	struct Model
	{
		std::vector<Variable> variables; 
		std::vector<std::shared_ptr<Constraint>> constraints; 
		ObjectiveType objective;

		Model( const std::vector<Variable>& variables, 
		       const std::vector<std::shared_ptr<Constraint>>&	constraints,
		       ObjectiveType objective )
			: variables( variables ),
			  constraints( constraints ),
			  objective( objective )
		{ }

		Model( const Model& other ) = default;
		Model( Model&& other ) = default;	
	
		Model& operator=( const Model& other ) = default;
		Model& operator=( Model&& other ) = default;	

		virtual ~Model() = default;
	};

	template<typename ObjectiveType = NullObjective, typename ... ConstraintType>
	class FactoryModel
	{
	public:
		virtual std::unique_ptr< Model<ObjectiveType, ConstraintType...> > make_model() = 0;
	};
}
