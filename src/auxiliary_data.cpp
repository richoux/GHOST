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

#include <algorithm>

#include "auxiliary_data.hpp"

using namespace ghost;

AuxiliaryData::AuxiliaryData()
	: _variables_index( std::vector<int>{0} )
{ }

AuxiliaryData::AuxiliaryData( const std::vector<int>& variables_index )
	: _variables_index( variables_index )
{ }

AuxiliaryData::AuxiliaryData( const std::vector<Variable>& variables )
	: _variables_index( std::vector<int>( variables.size() ) )
{
	std::transform( variables.begin(),
	                variables.end(),
	                _variables_index.begin(),
	                [&](const auto& v){ return v.get_id(); } );
}

void AuxiliaryData::update( int index, int new_value )
{
	if( _variables_position.count( index) > 0 )
		update( _variables, _variables_position.at( index ), new_value );
}

void AuxiliaryData::update()
{
	for( int i = 0 ; i < static_cast<int>( _variables.size() ) ; ++i )
		update( _variables, i, _variables[i]->get_value() );
}