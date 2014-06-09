/*
 * GHOST (General meta-Heuristic Optimization Solving Tool) is a C++ library 
 * designed for StarCraft: Brood war. 
 * GHOST is a meta-heuristic solver aiming to solve any kind of combinatorial 
 * and optimization RTS-related problems represented by a CSP. 
 * It is an extension of the project Wall-in.
 * Please visit https://github.com/richoux/GHOST for further information.
 * 
 * Copyright (C) 2014 Florian Richoux
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
#include <iostream>
#include <typeinfo>

// #include "../variables/variable.hpp"
#include "../misc/random.hpp"

using namespace std;

namespace ghost
{
  template <typename TypeVariable>
  class Domain
  {
  public:
    Domain( int, int );
    Domain( int, int, const vector< int >& );
        

    inline int		randomValue( const TypeVariable& var )		const	{ return v_randomValue(var); }
    inline vector<int>	possibleValues( const TypeVariable& var )	const	{ return v_possibleValues(var); }
    inline void		resetDomain( const TypeVariable& var );			{ v_resetDomain(var); }
    inline void		resetAllDomains()					{ v_resetAllDomains(); }
    
    inline int getSize() const { return size; }

    friend ostream& operator<<( ostream& os, const Domain<TypeVariable>& domain )
      {
	os << "Domain type: " <<  typeid(domain).name() << endl
	   << "Size: " <<  domain.size << endl;
	for( int i = 0; i < domains.size; ++i )
	{
	  os << "Domain[" << i << "]: ";
	  for( auto& e : domains[i] )
	    os << e << " ";
	  os << endl;
	}
	os << endl;
	return os;
      }

  protected:
    virtual int v_randomValue( const TypeVariable& variable ) const
      {
	vector<int> possibilities = domains[ variable.getId() ];
	return possibilities[ random.getRandNum( possibilities.size() ) ];
      }

    virtual vector<int> v_possibleValues( const TypeVariable& variable ) const
      {
	return domains[ variable.getId() ];
      }

    virtual void v_resetDomain( const TypeVariable& variable )
      {
	domains[ variable.getId() ] = initialDomain;
      }

    virtual void v_resetAllDomains();
    
    int size;
    vector< vector< int > > domains;
    vector< int > initialDomain;
    Random random;
  };
}
