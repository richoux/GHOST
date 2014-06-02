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
#include <typeinfo>
#include <map>

#include "../variables/building.hpp"
#include "../domains/grid.hpp"
#include "../objectives/objective.hpp"

namespace ghost
{
  class Constraint
  {
  public:
    Constraint( const std::vector< std::shared_ptr<Building> >&, const Grid& );
    // Constraint(const Constraint&) = default;
    // Constraint(Constraint&&) = default;
    // Constraint& operator=(const Constraint&) = default;
    // Constraint& operator=(Constraint&&) = default;
    // virtual ~Constraint() = default;

    virtual double cost( std::vector<double>& ) const = 0;
    virtual std::vector<double> simulateCost( Building&, const std::vector<int>&, int, std::vector< std::vector<double> >&, std::shared_ptr<Objective>& );
    virtual std::vector<double> simulateCost( Building&, const std::vector<int>&, int, std::vector< std::vector<double> >& );
    virtual double simulateCost( Building&, const int, std::vector<double>& );

    inline void update( const Grid& g ) { grid = g; }

    friend std::ostream& operator<<( std::ostream&, const Constraint& );
    
  protected:
    bool isWall() const;
    
    std::vector< std::shared_ptr<Building> > variables;
    Grid grid;
  };  

  //Overlap
  class Overlap : public Constraint
  {
  public:
    Overlap( const std::vector< std::shared_ptr<Building> >&, const Grid& );
    double cost( std::vector<double>& ) const;
    std::vector<double> simulateCost( Building&, const std::vector<int>&, int, std::vector< std::vector<double> >& );
  };

  //Buildable
  class Buildable : public Constraint
  {
  public:
    Buildable( const std::vector< std::shared_ptr<Building> >&, const Grid& );
    double cost( std::vector<double>& ) const;
    std::vector<double> simulateCost( Building&, const std::vector<int>&, int, std::vector< std::vector<double> >& );
  };

  //NoGaps
  class NoGaps : public Constraint
  {
  public:
    NoGaps( const std::vector< std::shared_ptr<Building> >&, const Grid& );
    double cost( std::vector<double>& ) const;
  };

  //StartingTargetTiles
  class StartingTargetTiles : public Constraint
  {
  public:
    StartingTargetTiles( const std::vector< std::shared_ptr<Building> >&, const Grid& );
    double cost( std::vector<double>& ) const;
  private:
    std::map<int, std::shared_ptr<Building> > mapBuildings;
  };
}
