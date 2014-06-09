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
#include <memory>
#include <algorithm>

#include "../../include/objective/wallinObjective.hpp"

using namespace std;

namespace ghost
{
  /***********/
  /* NoneObj */
  /***********/
  NoneObj::NoneObj( string name ) : Objective<Building, WallinGrid>( name ) { }

  double NoneObj::cost( const vector< Building > &vecBuildings, const WallinGrid &grid ) const
  {
    return count_if( vecBuildings.begin(), 
		     vecBuildings.end(), 
		     []( const Building &b ){ return b.isOnGrid(); });
    //return 0.;
  }

  int NoneObj::heuristicVariable( const vector< int > &vecVariables, const vector< Building > &vecBuildings, const WallinGrid &grid )
  {
    return vecVariables[ randomVar.getRandNum( vecVariables.size() ) ];
  }

  void NoneObj::setHelper( const Building &b, const vector< Building > &vecBuildings, const WallinGrid &grid )
  {
    if( b.isOnGrid() )
      heuristicValueHelper.at( b.getValue() ) = 0;
  }

  /**********/
  /* GapObj */
  /**********/
  GapObj::GapObj( string name ) : Objective<Building, WallinGrid>( name ) { }

  double GapObj::cost( const vector< Building > &vecBuildings, const WallinGrid &grid ) const
  {
    int gaps = 0;
    
    vector< Building > toVisit = vecBuildings;

    while( !toVisit.empty() )
    {
      auto b = *(toVisit.begin());
      gaps += gapSize( b, toVisit, grid );
      toVisit.erase( toVisit.begin() );
    }

    return gaps;
  }

  int GapObj::heuristicVariable( const vector< int > &vecVariables, const vector< Building > &vecBuildings, const WallinGrid &grid )
  {
    auto worst =  max_element(vecVariables.begin(),
			      vecVariables.end(),
			      [&](int v1, int v2)
			      {return gapSize( *vecBuildings[v1], vecBuildings, grid ) < gapSize( *vecBuildings[v2], vecBuildings, grid );} );

    return *(worst);
  }

  void GapObj::setHelper( const Building &b, const vector< Building > &vecBuildings, const WallinGrid &grid )
  {
    if( b.isOnGrid() )
      heuristicValueHelper.at( b.getValue() ) = gapSize( b, vecBuildings, grid );
  }

  int GapObj::gapSize( const Building &b, const vector< Building > &vecBuildings, const WallinGrid &grid ) const
  {
    if( !b.isOnGrid() )
      return 0;

    int gaps = 0;
    set< Building > neighbors = grid.getBuildingsAbove( b, vecBuildings );

    // cout << "ABOVE " << b->getId() << endl;
    // for( auto n : neighbors )
    //   cout << n->getId() << " ";
    // cout << endl;

    gaps += count_if( neighbors.begin(), 
		      neighbors.end(), 
		      [&](const Building &n){return b.getGapTop() + n.getGapBottom() >= 16;});
    
    neighbors = grid.getBuildingsOnRight( b, vecBuildings );
    // cout << "RIGHT " << b->getId() << endl;
    // for( auto n : neighbors )
    //   cout << n->getId() << " ";
    // cout << endl;
    gaps += count_if( neighbors.begin(), 
		      neighbors.end(), 
		      [&](const Building &n){return b.getGapRight() + n.getGapLeft() >= 16;});
    
    neighbors = grid.getBuildingsBelow( b, vecBuildings );
    // cout << "BELOW " << b->getId()  << endl;
    // for( auto n : neighbors )
    //   cout << n->getId() << " ";
    // cout << endl;
    gaps += count_if( neighbors.begin(), 
		      neighbors.end(), 
		      [&](const Building &n){return b.getGapBottom() + n.getGapTop() >= 16;});
    
    neighbors = grid.getBuildingsOnLeft( b, vecBuildings );
    // cout << "LEFT " << b->getId()  << endl;
    // for( auto n : neighbors )
    //   cout << n->getId() << " ";
    // cout << endl;
    gaps += count_if( neighbors.begin(), 
		      neighbors.end(), 
		      [&](const Building &n){return b.getGapLeft() + n.getGapRight() >= 16;});

    return gaps;
  }

  /***************/
  /* BuildingObj */
  /***************/
  BuildingObj::BuildingObj( string name ) : Objective<Building, WallinGrid>( name ) { }

  double BuildingObj::cost( const vector< Building > &vecBuildings, const WallinGrid &grid ) const
  {
    return count_if( vecBuildings.begin(), 
		     vecBuildings.end(), 
		     []( const Building &b ){ return b.isOnGrid(); });
  }

  int BuildingObj::heuristicVariable( const vector< int > &vecVariables, const vector< Building > &vecBuildings, const WallinGrid &grid )
  {
    vector< int > varOnGrid( vecVariables.size() );
    
    auto it = copy_if( vecVariables.begin(),
		       vecVariables.end(),
		       varOnGrid.begin(),
		       [&](int b){return vecBuildings[b].isOnGrid();} );

    int size = distance( varOnGrid.begin(), it );

    if( it == varOnGrid.begin() )
    {
      varOnGrid = vecVariables;
      size = vecVariables.size();
    }

    return varOnGrid[ randomVar.getRandNum( size ) ];    
  }

  void BuildingObj::setHelper( const Building &b, const vector< Building > &vecBuildings, const WallinGrid &grid )
  {
    if( b.isOnGrid() )
    {
      int pos = b.getValue();
      heuristicValueHelper.at( pos ) = grid.distanceToTarget( pos );
    }
  }

  /***************/
  /* TechTreeObj */
  /***************/
  TechTreeObj::TechTreeObj( string name ) : Objective<Building, WallinGrid>( name ) { }

  double TechTreeObj::cost( const vector< Building > &vecBuildings, const WallinGrid &grid ) const
  {
    vector< Building > onGrid( vecBuildings.size() );
    
    auto it = copy_if( vecBuildings.begin(),
		       vecBuildings.end(), 
		       onGrid.begin(),
		       [](const Building &b){ return b.isOnGrid(); } );
    onGrid.resize( distance( onGrid.begin(), it ) );

    auto max =  max_element( onGrid.begin(), 
			     onGrid.end(), 
			     [](const Building &b1, const Building &b2)
			     {return b1.getTreedepth() < b2.getTreedepth();} );

    return max->getTreedepth();
  }

  int TechTreeObj::heuristicVariable( const vector< int > &vecVariables, const vector< Building > &vecBuildings, const WallinGrid &grid )
  {
    // auto min =  min_element( vecBuildings.begin(), 
    // 				  vecBuildings.end(), 
    // 				  [](const Building &b1, const Building &b2)
    // 				  {return b1.getTreedepth() < b2.getTreedepth();} );

    // int minValue = min->getTreedepth();
    // vector< int > varMinTech( vecVariables.size() );

    // auto it = copy_if( vecVariables.begin(),
    // 			    vecVariables.end(),
    // 			    varMinTech.begin(),
    // 			    [&](int b){return vecBuildings[b].getTreedepth() == minValue;} );

    auto min =  min_element( vecVariables.begin(), 
			     vecVariables.end(), 
			     [&](int b1, int b2)
			     { return vecBuildings[b1].getTreedepth() < vecBuildings[b2].getTreedepth(); } );

    vector< int > varMinTech( vecVariables.size() );
    
    auto it = copy_if( vecVariables.begin(),
		       vecVariables.end(),
		       varMinTech.begin(),
		       [&](int b){return vecBuildings[b].getTreedepth() == min;} );
    
    int size = distance( varMinTech.begin(), it );

    if( it == varMinTech.begin() )
    {
      varMinTech = vecVariables;
      size = vecVariables.size();
    }
    
    return varMinTech[ randomVar.getRandNum( size ) ];    
  }

  void TreeTechObj::setHelper( const Building &b, const vector< Building > &vecBuildings, const WallinGrid &grid )
  {
    if( b.isOnGrid() )
    {
      int pos = b.getValue();
      heuristicValueHelper.at( pos ) = grid.distanceToTarget( pos );
    }
  }

  /**************/
  /* FactoryObj */
  /**************/
  shared_ptr<Objective> FactoryObj::makeObjective( const string &obj ) const
  {
    if( obj.compare("gap") == 0 || obj.compare("g") == 0 || obj.compare("G") == 0 )
      return make_shared<GapObj>("gap");
    
    if( obj.compare("building") == 0 || obj.compare("b") == 0 || obj.compare("B") == 0 )
      return make_shared<BuildingObj>("building");
    
    if( obj.compare("techtree") == 0 || obj.compare("t") == 0 || obj.compare("T") == 0 )
      return make_shared<TechTreeObj>("techtree");
    
    return make_shared<NoneObj>("none");
  }
}
