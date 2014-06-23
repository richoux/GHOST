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
#include <algorithm>
#include <numeric>

#include "../misc/random.hpp"

using namespace std;

namespace ghost
{
  template <typename TypeVariable>
  class Domain
  {
  public:
    Domain( int size, int numberVariables )
      : size(size),
	domains(vector< vector<int> >( numberVariables )),
	initialDomain(vector<int>( size ))
    {
      std::iota( begin(initialDomain), end(initialDomain), -1 );
      for( int i = 0; i < numberVariables; ++i )
      {
	domains[i] = vector<int>( size );
	std::iota( begin(domains[i]), end(domains[i]), -1 );
      }
    }

    Domain( int size, int numberVariables, const vector< int > &initialDomain )
      : size(size),
	domains(vector< vector<int> >( numberVariables )),
	initialDomain(initialDomain)
    {
      for( int i = 0; i < numberVariables; ++i )
      {
	domains[i] = vector<int>( size );	  
	std::copy( begin(initialDomain), end(initialDomain), domains[i].begin() );
      }
    }
        

    inline int randomValue( const TypeVariable& variable )
    {
      vector<int> possibilities = domains[ variable.getId() ];
      return possibilities[ random.getRandNum( possibilities.size() ) ];
    }
      
    inline vector<int> possibleValues( const TypeVariable& variable ) const
    {
      return domains[ variable.getId() ];
    }
      
    inline void	resetDomain( const TypeVariable& variable )
    {
      domains[ variable.getId() ] = initialDomain;
    }
      
    inline void	resetAllDomains()
    {
      for( auto& d : domains )
	d = initialDomain;
    }

    inline int getSize() const { return size; }

    inline void add( const TypeVariable& variable ) { }      
    inline void	clear( const TypeVariable& variable ) { }

    friend ostream& operator<<( ostream& os, const Domain<TypeVariable>& domain )
    {
      os << "Domain type: " <<  typeid(domain).name() << endl
	 << "Size: " <<  domain.size << endl;
      return os;
    }

  protected:
    int size;
    vector< vector< int > > domains;
    vector< int > initialDomain;
    Random random;
  };
}
