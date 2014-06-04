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

#include "../variables/variable.hpp"
#include "../misc/random.hpp"

using namespace std;

namespace ghost
{
  class Domain
  {
  public:
    Domain( int, int );
    Domain( int, int, const vector< int >& );
        

    virtual int		randomValue( const Variable& ) const;
    virtual vector<int> possibleValues( const Variable& ) const;
    virtual void	resetDomain( const Variable& );
    virtual void	resetAllDomains();
    
    inline int getSize() const { return size; }

    friend ostream& operator<<( ostream&, const Domain& );

  protected:
    int size;
    vector< vector< int > > domains;
    vector< int > initialDomain;
    Random random;
  };
}
