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
#include <memory>

#include "../variables/variable.hpp"
#include "../domains/domain.hpp"
#include "../objectives/objective.hpp"

using namespace std;

namespace ghost
{
  class Constraint
  {
  public:
    virtual double cost( vector<double>& ) const = 0;
    virtual vector<double> simulateCost( Variable&, const vector<int>&, vector< vector<double> >&, shared_ptr<Objective>& ) = 0;
    
    virtual void update( const shared_ptr<Domain> &d );

    friend ostream& operator<<( ostream&, const Constraint& );
    
  protected:
    vector< shared_ptr<Variable> > variables;
    shared_ptr<Domain> domain;
  };  
}
