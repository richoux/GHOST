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

#include "model.hpp"

using namespace ghost;

Model::Model( std::vector<Variable>&& moved_variables, 
              const std::vector<std::shared_ptr<Constraint>>&	constraints,
              const std::shared_ptr<Objective>& objective,
              const std::shared_ptr<AuxiliaryData>& auxiliary_data )
	: variables( std::move( moved_variables ) ),
	  constraints( constraints ),
	  objective( objective ),
	  auxiliary_data ( auxiliary_data )
{ }

FactoryModel::FactoryModel( const std::vector<Variable>& variables ) 
	: _variables_origin( variables ),
	  ptr_variables( std::vector<Variable*>( variables.size() ) )
{	}

Model FactoryModel::make_model()
{
	_variables_copy = std::vector<Variable>( _variables_origin.size() );
	std::copy( _variables_origin.begin(), _variables_origin.end(), _variables_copy.begin() );
	std::transform( _variables_copy.begin(),
	                _variables_copy.end(),
	                ptr_variables.begin(),
	                [&](Variable& var){ return &var; } );
	
	constraints.clear();
	declare_auxiliary_data();
	declare_constraints();
	declare_objective();

	return Model( std::move( _variables_copy ), constraints, objective, auxiliary_data );
}

void FactoryModel::declare_objective()
{
	objective = std::make_shared<NullObjective>( ptr_variables );
}

void FactoryModel::declare_auxiliary_data()
{
	auxiliary_data = std::make_shared<NullAuxiliaryData>( ptr_variables );
}
