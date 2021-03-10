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

#include <limits>

#include "variable.hpp"

using namespace ghost;

unsigned int Variable::NBER_VAR = 0;

Variable::Variable( const std::string& name, const std::vector<int>& domain, int index )
	: _name( name ),
	  _domain( domain ),
	  _current_value( domain.at( index ) ),
	  _min_value( *( std::min_element( _domain.begin(), _domain.end() ) ) ),
	  _max_value( *( std::max_element( _domain.begin(), _domain.end() ) ) )
{
	if( NBER_VAR < std::numeric_limits<unsigned int>::max() )
		_id = NBER_VAR++;
	else
		_id = NBER_VAR = 0;
}

Variable::Variable( const std::string& name, int startValue, std::size_t size, int index )
	: _name( name ),
	  _domain( std::vector<int>( size ) ),
	  _min_value( startValue ),
	  _max_value( startValue + static_cast<int>( size ) - 1 )
{
	if( NBER_VAR < std::numeric_limits<unsigned int>::max() )
		_id = NBER_VAR++;
	else
		_id = NBER_VAR = 0;

	std::iota( _domain.begin(), _domain.end(), startValue );
	_current_value = _domain.at( index );
}

std::vector<int> Variable::get_partial_domain( int range ) const
{
	if( range == static_cast<int>( _domain.size() ) )
		return _domain;
	else
	{
		std::vector<int> partial_domain( range );
		
		// [---xxxIxxx-]
		//        |
		//        ^
		//      index
		int index = std::distance( _domain.cbegin(), std::find( _domain.cbegin(), _domain.cend(), _current_value ) );
		int start_position = index - static_cast<int>( range / 2 );

		if( start_position >= 0 )
		{
			// [---xxxIxxx-]
			//     |
			//     ^
			// start_position
			if( index + ( range - static_cast<int>( range / 2 ) ) <= static_cast<int>( _domain.size() ) )
			{
				std::copy( _domain.begin() + start_position,
				           _domain.begin() + start_position + range,
				           partial_domain.begin() );
			}
			// [xx----xxxIx]
			//   |
			//   ^
			// end_position
			else
			{
				int end_position = index + ( range - static_cast<int>( range / 2 ) ) - static_cast<int>( _domain.size() );
				std::copy( _domain.begin(),
				           _domain.begin() + end_position,
				           partial_domain.begin() );
				
				std::copy( _domain.begin() + start_position,
				           _domain.end(),
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
			start_position += static_cast<int>( _domain.size() );
			std::copy( _domain.begin(),
			           _domain.begin() + end_position,
			           partial_domain.begin() );

			std::copy( _domain.begin() + start_position,
			           _domain.end(),
			           partial_domain.begin() + end_position );
		}
		
		return partial_domain;
	}
}
