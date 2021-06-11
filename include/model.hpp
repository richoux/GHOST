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
#include <algorithm>

#include "variable.hpp"
#include "constraint.hpp"
#include "objective.hpp"
#include "auxiliary_data.hpp"

namespace ghost
{
	/***********/
	/** Model **/
	/***********/
	struct Model final
	{
		std::vector<Variable> variables; 
		std::vector<std::shared_ptr<Constraint>> constraints; 
		std::shared_ptr<Objective> objective;
		std::shared_ptr<AuxiliaryData> auxiliary_data;

		Model( std::vector<Variable>&& variables, 
		       const std::vector<std::shared_ptr<Constraint>>&	constraints,
		       const std::shared_ptr<Objective>& objective,
		       const std::shared_ptr<AuxiliaryData>& auxiliary_data );
	};

	/******************/
	/** FactoryModel **/
	/******************/
	class FactoryModel
	{
		template<typename FactoryModelType> friend class Solver;

		std::vector<Variable> _variables_origin; 
		std::vector<Variable> _variables_copy; 
		
		Model make_model();
		inline int get_number_variables() { return static_cast<int>( _variables_origin.size() ); }

	protected:
		std::vector<Variable*> ptr_variables; 
		std::vector<std::shared_ptr<Constraint>> constraints; 
		std::shared_ptr<Objective> objective;
		std::shared_ptr<AuxiliaryData> auxiliary_data;

	public:
		FactoryModel( const std::vector<Variable>& variables );
		virtual ~FactoryModel() = default;

		virtual void declare_constraints() = 0;
		virtual void declare_objective();
		virtual void declare_auxiliary_data();
	};
}
