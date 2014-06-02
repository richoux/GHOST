/*
 * Wall-in is a C++ library designed for StarCraft: Brood,
 * making a wall optimizised for a given objective: minimize the
 * number of buildings, the technology needed, the number of gaps
 * between building big enough to let enter small units, etc.
 * To do so, it use some Constraint Programming techniques 
 * like meta-heuristics.
 * Please visit https://github.com/richoux/Wall-in 
 * for further information.
 * 
 * Copyright (C) 2014 Florian Richoux
 *
 * This file is part of Wall-in.
 * Wall-in is free software: you can redistribute it and/or 
 * modify it under the terms of the GNU General Public License as published 
 * by the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * Wall-in is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with Wall-in. If not, see http://www.gnu.org/licenses/.
 */


#pragma once

#include <set>
#include <vector>
#include <memory>
#include <algorithm>
#include <limits>

#include "building.hpp"
#include "grid.hpp"
#include "random.hpp"

namespace wallin
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
