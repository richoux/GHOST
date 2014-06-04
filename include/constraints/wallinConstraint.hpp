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
#include <map>

#include "constraint.hpp"
#include "../variables/building.hpp"
#include "../domains/wallinGrid.hpp"
#include "../objectives/objective.hpp"

using namespace std;

namespace ghost
{
  class WallinConstraint : public Constraint
  {
  public:
    WallinConstraint( const vector< shared_ptr<Building> >&, const shared_ptr<WallinGrid>& );

    vector<double> simulateCost( Building&, const vector<int>&, vector< vector<double> >&, shared_ptr<Objective>& );
    virtual vector<double> simulateCost( Building&, const vector<int>&, int, vector< vector<double> >& );

  protected:
    bool isWall() const;
  };  

  //Overlap
  class Overlap : public WallinConstraint
  {
  public:
    Overlap( const vector< shared_ptr<Building> >&, const shared_ptr<WallinGrid>& );
    double cost( vector<double>& ) const;
    vector<double> simulateCost( Building&, const vector<int>&, int, vector< vector<double> >& );
  };

  //Buildable
  class Buildable : public WallinConstraint
  {
  public:
    Buildable( const vector< shared_ptr<Building> >&, const shared_ptr<WallinGrid>& );
    double cost( vector<double>& ) const;
    vector<double> simulateCost( Building&, const vector<int>&, int, vector< vector<double> >& );
  };

  //NoGaps
  class NoGaps : public WallinConstraint
  {
  public:
    NoGaps( const vector< shared_ptr<Building> >&, const shared_ptr<WallinGrid>& );
    double cost( vector<double>& ) const;
  };

  //StartingTargetTiles
  class StartingTargetTiles : public WallinConstraint
  {
  public:
    StartingTargetTiles( const vector< shared_ptr<Building> >&, const shared_ptr<WallinGrid>& );
    double cost( vector<double>& ) const;
  private:
    map<int, shared_ptr<Building> > mapBuildings;
  };
}
