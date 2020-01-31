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
 * Copyright (C) 2014-2020 Florian Richoux
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


#include <typeinfo>
#include <algorithm>
#include <numeric>
#include <exception>

#include "domain.hpp"

using namespace std;
using namespace ghost;

Domain::Domain( const vector< int >& domain )
	: _domain	( domain ),
	  _minValue	( *std::min_element( _domain.begin(), _domain.end() ) ),
	  _maxValue	( *std::max_element( _domain.begin(), _domain.end() ) ),
	  _size	( domain.size() )
{
	_indexes = vector<int>( _maxValue - _minValue + 1, -1 );
	for( int i = 0 ; i < (int)_size ; ++i )
		_indexes[ _domain[ i ] - _minValue ] = i ;
}

Domain::Domain( int startValue, size_t size )
	: _domain	( vector<int>( size ) ),
	  _minValue	( startValue ),
	  _maxValue	( startValue + (int)size - 1 ),
	  _size	( size )
{
	iota( begin( _domain ), end( _domain ), startValue );

	_indexes = vector<int>( _size, -1 );
	for( int i = 0 ; i < (int)_size ; ++i )
		_indexes[ _domain[ i ] - _minValue ] = i;
}

Domain::Domain( const Domain &other )
	: _domain( other._domain ),
	  _indexes( other._indexes ),
	  _minValue( other._minValue ),
	  _maxValue( other._maxValue ),
	  _size( other._size ),
	  _rng( randutils::mt19937_rng() )
{ }

Domain& Domain::operator=( Domain other )
{
	this->swap( other );
	_rng = randutils::mt19937_rng();
	return *this;
}

void Domain::swap( Domain &other )
{
	std::swap( this->_domain, other._domain );
	std::swap( this->_indexes, other._indexes );
	std::swap( this->_minValue, other._minValue );
	std::swap( this->_maxValue, other._maxValue );
	std::swap( this->_size, other._size );
}  

int Domain::get_value( int index ) const
{
	if( index >=0 && index < (int)_size )
		return _domain[ index ];
	else
		throw indexException( index, (int)_size );
}

int Domain::index_of( int value ) const
{
	if( value < _minValue || value > _maxValue )
		throw valueException( value, _minValue, _maxValue );
    
	int index = _indexes[ value - _minValue ];
	if( index == -1 )
		throw valueException( value, _minValue, _maxValue );
	else
		return index;
}
