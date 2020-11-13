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

#pragma once

#include <string>

class Neighborhood final
{
	int _number_variables;
	double _domain_span;
	bool _is_permutation;
	double _randomness_rate;

	struct rangeException : std::exception
	{
		double percent;
		std::string message;
		
		rangeException( double percent ) : percent( percent )
		{
			message = "The given rate " + std::to_string( percent ) + " should be a real value within the range [0, 1].\n";
		}
		const char* what() const noexcept { return message.c_str(); }
	};

public:
	Neighborhood( int number_variables,
	              double domain_span,
	              bool is_permutation,
	              double randomness_rate )
		: _number_variables ( number_variables ),
		  _is_permutation ( is_permutation )
	{
		if( domain_span < 0 || domain_span > 1 )
			throw rangeException( domain_span );
		
		if( randomness_rate < 0 || randomness_rate > 1 )
			throw rangeException( randomness_rate );

		_domain_span = domain_span;
		_randomness_rate = randomness_rate;
	}

	inline int get_number_variables() { return _number_variables; }
	inline double get_domain_span() { return _domain_span; }
	inline bool is_permutation() { return _is_permutation; }
	inline double get_randomness_rate() { return _randomness_rate; }

	inline void set_number_variables( int number_variable ) { _number_variables = number_variable; }
	inline void set_domain_span( double domain_span )
	{
		if( domain_span < 0 || domain_span > 1 )
			throw rangeException( domain_span );
		_domain_span = domain_span;
	}
	inline void set_is_permutation( bool is_permutation ) { _is_permutation = is_permutation; }
	inline void set_randomness_rate( double randomness_rate )
	{
		if( randomness_rate < 0 || randomness_rate > 1 )
			throw rangeException( randomness_rate );
		_randomness_rate = randomness_rate;
	}	
};
