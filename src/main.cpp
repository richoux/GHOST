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


#include <iostream>
#include <vector>
#include <memory>
#include <algorithm>
#include <type_traits>

#include "../include/variables/building.hpp"
#include "../include/constraints/wallinConstraint.hpp"
#include "../include/domains/wallinGrid.hpp"
#include "../include/misc/tools.hpp"
#include "../include/misc/wallinTerran.hpp"
#include "../include/solver.hpp"

using namespace ghost;
using namespace std;

int main(int argc, char **argv)
{
  vector< pair<int, int> > unbuildables 
  { 
    make_pair(7, 12), 
    make_pair(7, 13), 
    make_pair(7, 14), 
    make_pair(7, 15), 
    make_pair(8, 10), 
    make_pair(8, 11), 
    make_pair(8, 12), 
    make_pair(8, 13), 
    make_pair(8, 14), 
    make_pair(8, 15), 
    make_pair(9, 10), 
    make_pair(9, 11), 
    make_pair(9, 12), 
    make_pair(9, 13), 
    make_pair(9, 14), 
    make_pair(9, 15), 
    make_pair(10, 8), 
    make_pair(10, 9), 
    make_pair(10, 10), 
    make_pair(10, 11), 
    make_pair(10, 12), 
    make_pair(10, 13), 
    make_pair(10, 14), 
    make_pair(10, 15), 
    make_pair(11, 8), 
    make_pair(11, 9), 
    make_pair(11, 10), 
    make_pair(11, 11), 
    make_pair(11, 12), 
    make_pair(11, 13), 
    make_pair(11, 14), 
    make_pair(11, 15) 
  };
  
  // Please write the name of the objective here!
  string objective = "g";

  // Define variables
  vector<shared_ptr<Variable> >	vec = makeTerranBuildings();

  // Define domain
  shared_ptr<Domain> grid = make_shared<WallinGrid>( 16, 12, unbuildables, vec, 11, 7, 6, 15 );

  // Define constraints
  vector< shared_ptr<Constraint> > vecConstraints = makeTerranConstraints( vec, grid );

  Solver solver( vecConstraints, vec, grid, objective );

#ifndef NDEBUG
  cout << boolalpha << "Building movable: " << is_nothrow_move_constructible<Building>::value << endl;
  cout << boolalpha << "Barracks movable: " << is_nothrow_move_constructible<Barracks>::value << endl;
  cout << boolalpha << "Grid movable: " << is_nothrow_move_constructible<WallinGrid>::value << endl;
  cout << boolalpha << "Solver movable: " << is_nothrow_move_constructible<Solver>::value << endl;
  cout << boolalpha << "Random movable: " << is_nothrow_move_constructible<Random>::value << endl;
  cout << boolalpha << "Constraint movable: " << is_nothrow_move_constructible<Constraint>::value << endl;
  cout << boolalpha << "Overlap movable: " << is_nothrow_move_constructible<Overlap>::value << endl;
  cout << boolalpha << "Objective movable: " << is_nothrow_move_constructible<Objective>::value << endl;
  cout << boolalpha << "GapObj movable: " << is_nothrow_move_constructible<GapObj>::value << endl;
#endif

  solver.solve( 20 );    
}
