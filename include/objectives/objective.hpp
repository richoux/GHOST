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

#include <set>
#include <vector>
#include <memory>
#include <algorithm>
#include <limits>

#include "../variables/building.hpp"
#include "../domains/grid.hpp"
#include "../misc/random.hpp"

namespace ghost
{
  class Objective
  {
  public:
    Objective( std::string );
    virtual double cost( const std::vector< std::shared_ptr<Building> > &vecBuildings, const Grid& ) const = 0;
    virtual int heuristicVariable( const std::vector< int > &vecVariables, const std::vector< std::shared_ptr<Building> > &vecBuildings, const Grid& ) = 0;
    virtual void setHelper( const Building&, const std::vector< std::shared_ptr<Building> >&, const Grid& );
    void initHelper( int );
    void resetHelper();
    int heuristicValue( const std::vector< double > &vecPositions, double&, int&, const Grid& ) const;
    inline  std::string getName() { return name; }
  protected:
    Random randomVar;
    std::string name;
    std::vector<double> heuristicValueHelper; 
  };

  class NoneObj : public Objective
  {
  public:
    NoneObj( std::string );
    double cost( const std::vector< std::shared_ptr<Building> > &vecBuildings, const Grid& ) const;
    int heuristicVariable( const std::vector< int > &vecVariables, const std::vector< std::shared_ptr<Building> > &vecBuildings, const Grid& );
    void setHelper( const Building&, const std::vector< std::shared_ptr<Building> >&, const Grid& );
  };

  class GapObj : public Objective
  {
  public:
    GapObj( std::string );
    double cost( const std::vector< std::shared_ptr<Building> > &vecBuildings, const Grid& ) const;
    int heuristicVariable( const std::vector< int > &vecVariables, const std::vector< std::shared_ptr<Building> > &vecBuildings, const Grid& );
    void setHelper( const Building&, const std::vector< std::shared_ptr<Building> >&, const Grid& );
  private:
    int gapSize( const Building&, const std::vector< std::shared_ptr<Building> >&, const Grid& ) const;
  };

  class BuildingObj : public Objective
  {
  public:
    BuildingObj( std::string );
    double cost( const std::vector< std::shared_ptr<Building> > &vecBuildings, const Grid& ) const;
    int heuristicVariable( const std::vector< int > &vecVariables, const std::vector< std::shared_ptr<Building> > &vecBuildings, const Grid& );
  };

  class TechTreeObj : public Objective
  {
  public:
    TechTreeObj( std::string );
    double cost( const std::vector< std::shared_ptr<Building> > &vecBuildings, const Grid& ) const;
    int heuristicVariable( const std::vector< int > &vecVariables, const std::vector< std::shared_ptr<Building> > &vecBuildings, const Grid& );
  };

  class FactoryObj
  {
  public:
    std::shared_ptr<Objective> makeObjective( const std::string& ) const;
  };
}
