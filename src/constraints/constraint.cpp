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


#include "../include/constraints/constraint.hpp"

namespace ghost
{
  
  Constraint::Constraint(const std::vector< std::shared_ptr<Building> >& variables, const Grid& grid) 
  : variables( variables ),
    grid( grid )
  { }
  
  std::vector<double> Constraint::simulateCost( Building& oldBuilding, const std::vector<int>& newPosition, int sizeGrid, std::vector< std::vector<double> >& vecVarSimCosts, std::shared_ptr<Objective> &objective )
  {
    std::vector<double> simCosts( sizeGrid, -1. );
    int backup = oldBuilding.getPosition();
    int previousPos;
    
    if( objective->getName().compare("fake") != 0 )
      objective->resetHelper();

    for( auto pos : newPosition )
    {
      if( pos >= 1 && pos == previousPos + 1 )
      {
	grid.quickShift( oldBuilding );
      }
      else
      { 
	grid.clear( oldBuilding );
	oldBuilding.setPos( pos );
	grid.add( oldBuilding );
      }

      simCosts[pos+1] = cost( vecVarSimCosts[pos+1] );
      if( objective->getName().compare("fake") != 0 )
	objective->setHelper( oldBuilding, variables, grid );
      previousPos = pos;
    }

    grid.clear( oldBuilding );
    oldBuilding.setPos( backup );
    grid.add( oldBuilding );
    
    return simCosts;
  }

  std::vector<double> Constraint::simulateCost( Building& oldBuilding, const std::vector<int>& newPosition, int sizeGrid, std::vector< std::vector<double> >& vecVarSimCosts )
  {
    shared_ptr<Objective> fake = make_shared<NoneObj>("fake");
    return simulateCost( oldBuilding, newPosition, sizeGrid, vecVarSimCosts, fake );
  }

  double Constraint::simulateCost( Building& oldBuilding, const int newPosition, std::vector<double>& varSimCost )
  {
    int backup = oldBuilding.getPosition();
    grid.clear( oldBuilding );

    oldBuilding.setPos( newPosition );
    grid.add( oldBuilding );

    double simCost = cost( varSimCost );

    grid.clear( oldBuilding );

    oldBuilding.setPos( backup );
    grid.add( oldBuilding );

    return simCost;
  }

  bool Constraint::isWall() const
  {
    auto startingBuildings = grid.buildingsAt( grid.getStartingTile() );
    if( startingBuildings.size() != 1)
      return false;

    auto targetBuildings = grid.buildingsAt( grid.getTargetTile() );
    if( targetBuildings.size() != 1)
      return false;

    // if same building on both the starting and target tile
    if( *startingBuildings.begin() == *targetBuildings.begin() )
      return true;

    int nberTarget = *( targetBuildings.begin() );

    int nberCurrent = *( startingBuildings.begin() );
    shared_ptr<Building> current = variables[ nberCurrent ];
    set< shared_ptr<Building> > toVisit = grid.getBuildingsAround( *current, variables );
    set< shared_ptr<Building> > visited;
    set< shared_ptr<Building> > neighbors;
    
    visited.insert( current );

    if( toVisit.find( variables[nberTarget] ) != toVisit.end() )
      return true;
    
    while( !toVisit.empty() )
    {
      auto first = *( toVisit.begin() );
      current = first;
      toVisit.erase( first );
      neighbors = grid.getBuildingsAround( *current, variables );
      
      for( auto n : neighbors )
      {
	if( n->getId() == nberTarget )
	  return true;
	if( visited.find( n ) == visited.end() )
	  toVisit.insert( n );
      }

      visited.insert( current );
    }

    return false;
  }

  std::ostream& operator<<( std::ostream& os, const Constraint& c )
  {
    os << "Constraint type: " <<  typeid(c).name() << std::endl;

    // for(auto v : c.variables)
    //   os << (*v) << std::endl;

    std::vector<double> fake(c.variables.size(), 0.);

    // return os << c.grid << std::endl
    // 	      << "Cost: " << c.cost( fake ) << std::endl;
    return os << "Cost: " << c.cost( fake ) << std::endl;
  }

  /***********/
  /* Overlap */
  /***********/
  Overlap::Overlap(const std::vector< std::shared_ptr<Building> >& variables, const Grid& grid) 
    : Constraint(variables, grid)
  { }

  double Overlap::cost( std::vector<double>& varCost ) const
  {
    // version 1: 1 failure = 1 cost
    // return double( grid.failures().size() );

    // version 2: 1 conflict = 1 cost (may have several conflicts into one failure)
    double conflicts = 0.;

    for( auto failures : grid.failures() )
    {
      int nbConflict = failures.second.size() - 1;
      if( nbConflict > 0 && failures.second.find( "###" ) == std::string::npos )
      {
	conflicts += nbConflict;
	std::set<int> setBuildings = grid.buildingsAt( failures.first );
	for( auto id : setBuildings )
	  varCost[ id ] += nbConflict;
      }
    }

    return conflicts;    
  }

  std::vector<double> Overlap::simulateCost( Building& oldBuilding, const std::vector<int>& newPosition, int sizeGrid, std::vector< std::vector<double> >& vecVarSimCosts )
  {
    std::vector<double> simCosts( sizeGrid, -1. );
    int backup = oldBuilding.getPosition();
    int previousPos;
    int diff;

    for( auto pos : newPosition )
    {
      if( pos >= 1 && pos == previousPos + 1 )
      {
	vecVarSimCosts[pos + 1] = vecVarSimCosts[pos];
	
	diff = grid.shift( oldBuilding ).first;
	if( diff != 0 )
	{
	  std::set<int> setBuildings = grid.buildingsAt( pos + 1 );
	  for( auto id : setBuildings )
	    vecVarSimCosts[pos + 1][ id ] += diff;
	}

	simCosts[pos + 1] = simCosts[pos] + diff;
      }
      else
      { 
	grid.clear( oldBuilding );
	oldBuilding.setPos( pos );
	grid.add( oldBuilding );
	
	simCosts[pos + 1] = cost( vecVarSimCosts[pos + 1] );
      }

      previousPos = pos;
    }

    grid.clear( oldBuilding );
    oldBuilding.setPos( backup );
    grid.add( oldBuilding );
    
    return simCosts;
  }

  /*************/
  /* Buildable */
  /*************/
  Buildable::Buildable(const std::vector< std::shared_ptr<Building> >& variables, const Grid& grid) 
    : Constraint(variables, grid)
  { }

  double Buildable::cost( std::vector<double>& varCost ) const
  {
    // count number of buildings misplaced on unbuildable tiles (denoted by ###)
    double conflicts = 0.;
    int nbConflict;

    for( auto failures : grid.failures() )
    {
      if( failures.second.find( "###" ) != std::string::npos )
      {
	nbConflict = failures.second.size() - 3;
	conflicts += nbConflict;
	std::set<int> setBuildings = grid.buildingsAt( failures.first );
	for( auto id : setBuildings )
	  varCost[ id ] += nbConflict;
      }
    }

    return conflicts;    
  }

  std::vector<double> Buildable::simulateCost( Building& oldBuilding, const std::vector<int>& newPosition, int sizeGrid, std::vector< std::vector<double> >& vecVarSimCosts )
  {
    std::vector<double> simCosts( sizeGrid, -1. );
    int backup = oldBuilding.getPosition();
    int previousPos;
    int diff;

    for( auto pos : newPosition )
    {
      if( pos >= 1 && pos == previousPos + 1 )
      {
	vecVarSimCosts[pos + 1] = vecVarSimCosts[pos];
	
	diff = grid.shift( oldBuilding ).second;
	if( diff != 0 )
	{
	  std::set<int> setBuildings = grid.buildingsAt( pos + 1 );
	  for( auto id : setBuildings )
	    vecVarSimCosts[pos + 1][ id ] += diff;
	}

	simCosts[pos + 1] = simCosts[pos] + diff;
      }
      else
      { 
	grid.clear( oldBuilding );
	oldBuilding.setPos( pos );
	grid.add( oldBuilding );
	
	simCosts[pos + 1] = cost( vecVarSimCosts[pos + 1] );
      }

      previousPos = pos;
    }

    grid.clear( oldBuilding );
    oldBuilding.setPos( backup );
    grid.add( oldBuilding );
    
    return simCosts;
  }

  /**********/
  /* NoGaps */
  /**********/
  NoGaps::NoGaps(const std::vector< std::shared_ptr<Building> >& variables, const Grid& grid) 
    : Constraint(variables, grid)
  { }

  double NoGaps::cost( std::vector<double>& varCost ) const
  {
    // cost = |buildings with one neighbor| - 1 + |buildings with no neighbors|
    double conflicts = 0.;

    if( !isWall() )
    {
      int nberNeighbors;
      std::vector<int> oneNeighborBuildings;

      for( auto building : variables )
      {
	if( building->isOnGrid() )
	{
	  // if we don't have a wall, penalise all buildings on the grid.
	  ++conflicts;
	  ++varCost[ building->getId() ];
	  
	  nberNeighbors = grid.countAround( *building, variables );

	  if( nberNeighbors == 0 || nberNeighbors > 2 ) // to change with Protoss and pylons
	  {
	    ++conflicts;
	    ++varCost[ building->getId() ];
	  }
	  else
	  {
	    if( nberNeighbors == 1 )
	      oneNeighborBuildings.push_back( building->getId() );
	  }
	}
      }

      if( oneNeighborBuildings.size() > 2 ) // for latter: pylons can be alone, or have 1 neighbor only
      {
	for( auto b : oneNeighborBuildings )
	  if( ! grid.isStartingOrTargetTile( b ) )
	  {
	    ++conflicts;
	    ++varCost[ b ];
	  }
      }
    }
    
    return conflicts;    
  }

  /***********************/
  /* StartingTargetTiles */
  /***********************/
  StartingTargetTiles::StartingTargetTiles(const std::vector< std::shared_ptr<Building> >& variables, const Grid& grid) 
    : Constraint(variables, grid)
  {
    for( auto b : variables )
      mapBuildings[b->getId()] = b;
  }

  double StartingTargetTiles::cost( std::vector<double>& varCost ) const
  {
    // no building on one of these two tiles: cost of the tile = 6
    // a building with no or with 2 or more neighbors: cost of the tile = 3
    // two or more buildings on one of these tile: increasing penalties.
    double conflicts = 0.;

    std::set<int> startingBuildings = grid.buildingsAt( grid.getStartingTile() );
    std::set<int> targetBuildings = grid.buildingsAt( grid.getTargetTile() );

    std::shared_ptr<Building> b;
    int neighbors;

    // if same building on both the starting and target tile
    if( startingBuildings.size() == 1 && targetBuildings.size() == 1 && *startingBuildings.begin() == *targetBuildings.begin() )
      return 0.;

    if( startingBuildings.empty() )
    {
      // penalize buildings not placed on the grid
      for( auto v : variables )
	if( !v->isOnGrid() )
	{
	  varCost[ v->getId() ] += 2;
	  conflicts += 2;
	}
    }
    else
    {
      //int penalty = 0;
      for( int bId : startingBuildings )
      {
	b = mapBuildings.at(bId);
	neighbors = grid.countAround( *b, variables );

	if( neighbors != 1 )
	{
	  conflicts += 2;
	  varCost[ bId ] += 2;
	}

	//conflicts += penalty++;
      }
    }

    if( targetBuildings.empty() )
    {      
      // penalize buildings not placed on the grid
      for( auto v : variables )
	if( !v->isOnGrid() )
	{
	  varCost[ v->getId() ] += 2;
	  conflicts += 2;
	}
    }
    else
    {
      //int penalty = 0;
      for( int bId : targetBuildings )
      {
	b = mapBuildings.at(bId);
	neighbors = grid.countAround( *b, variables );

	if( neighbors != 1 )
	{
	  conflicts += 2;
	  varCost[ bId ] += 2;
	}

	//conflicts += penalty++;	
      }
      
    }

    return conflicts;    
  }

}
