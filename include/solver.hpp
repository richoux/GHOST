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
#include <map>
#include <set>
#include <memory>
#include <cmath>
#include <chrono>
#include <ctime>
#include <limits>
#include <algorithm>
#include <functional>
#include <cassert>
#include <typeinfo>

#include "building.hpp"
#include "constraint.hpp"
#include "grid.hpp"
#include "tools.hpp"
#include "random.hpp"
#include "objective.hpp"

using namespace std;

namespace wallin
{
  class Solver
  {
  public:
    Solver( const vector< shared_ptr<Constraint> >&, 
    	    const vector<shared_ptr<Building> >&, 
    	    const Grid&,
    	    const string& = "" );

    Solver( const vector< shared_ptr<Constraint> >&, 
	    const vector<shared_ptr<Building> >&, 
	    const Grid&,
	    const int loops,
	    const string& = "" );

    Solver(const Solver&) = default;
    Solver(Solver&&) = default;
    Solver& operator=(const Solver&) = default;
    Solver& operator=(Solver&&) = default;
    ~Solver() = default;

    double solve( double );
    
  private:
    void reset();
    void move( shared_ptr<Building>&, int );
    set< shared_ptr<Building> > getNecessaryBuildings() const;

    vector< shared_ptr<Constraint> >	vecConstraints;
    vector< shared_ptr<Building> >	vecBuildings;
    vector<double>			variableCost;
    Grid				grid;
    int					loops;
    vector<int>				tabuList;
    Random				randomVar;
    FactoryObj				factory;
    shared_ptr<Objective>		objective;
    double				bestCost;
    vector< int >			bestSolution;
    multimap<int, shared_ptr<Building>> buildingSameSize;
  };
}
