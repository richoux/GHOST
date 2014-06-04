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

#include "variables/variable.hpp"
#include "constraints/constraint.hpp"
#include "domains/domain.hpp"
#include "misc/tools.hpp"
#include "misc/random.hpp"
#include "objectives/objective.hpp"

using namespace std;

namespace ghost
{
  class Solver
  {
  public:
    Solver( const vector< shared_ptr<Constraint> >&, 
    	    const vector<shared_ptr<Variable> >&, 
    	    const shared_ptr<Domain>&,
    	    const string& = "" );

    Solver( const vector< shared_ptr<Constraint> >&, 
	    const vector<shared_ptr<Variable> >&, 
	    const shared_ptr<Domain>&,
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
    void move( shared_ptr<Variable>&, int );
    set< shared_ptr<Variable> > getNecessaryVariables() const;

    vector< shared_ptr<Constraint> >	vecConstraints;
    vector< shared_ptr<Variable> >	vecVariables;
    vector<double>			variableCost;
    shared_ptr<Domain>			domain;
    int					loops;
    vector<int>				tabuList;
    Random				randomVar;
    FactoryObj				factory;
    shared_ptr<Objective>		objective;
    double				bestCost;
    vector<int>				bestSolution;
    multimap<int, shared_ptr<Variable>> buildingSameSize;
  };
}
