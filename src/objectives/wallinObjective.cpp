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
#include <memory>
#include <algorithm>
#include <limits>
#include <cmath>
#include <chrono>
#include <ctime>

#include "../../include/objective/wallinObjective.hpp"
#include "../../include/variable/building.hpp"
#include "../../include/constraint/wallinConstraint.hpp"

using namespace std;

namespace ghost
{
  int WallinObjective::sizeWall = numeric_limits<int>::max();
  
  /*******************/
  /* WallinObjective */
  /*******************/
  WallinObjective::WallinObjective( string name ) : Objective<Building, WallinGrid>( name ) { }

  void WallinObjective::v_setHelper( const Building &b, const vector< Building > &vecVariables, const WallinGrid &grid )
  {
    if( b.isSelected() )
    {
      int pos = b.getValue();
      heuristicValueHelper.at( pos ) = grid.distanceToTarget( pos );
    }
  }

  void WallinObjective::v_postprocessSatisfaction( const vector< Building > &vecVariables,
						   const WallinGrid &domain,
						   double &bestCost,
						   vector<int> &bestSolution ) const
  {
    bool change;
    double cost;
    NoGaps ng( vecVariables, domain );

    // find all buildings accessible from the starting building and remove all others
    int nberCurrent = *( domain.buildingsAt( domain.getStartingTile() ).begin() );
    Building current = vecVariables[ nberCurrent ];
    set< Building > toVisit = domain.getVariablesAround( current, vecVariables );
    set< Building > visited;
    set< Building > neighbors;
    
    visited.insert( current );
    
    while( !toVisit.empty() )
    {
      auto first = *( toVisit.begin() );
      current = first;
      toVisit.erase( first );
      neighbors = domain.getVariablesAround( current, vecVariables );
      
      visited.insert( current );
      
      for( auto n : neighbors )
	if( visited.find( n ) == visited.end() )
	  toVisit.insert( n );
    }
    
    // remove all unreachable buildings from the starting building out of the domain
    for( auto b : vecVariables )
      if( visited.find( b ) == visited.end() )
      {
	domain.clear( b );
	b.setValue( -1 );
      }

    vector<double> varSimCost( vecVariables.size() );

    // clean wall from unnecessary buildings.
    do
    {
      for( auto b : vecVariables )
	if( ! domain.isStartingOrTargetTile( b.getId() ) )
	{
	  change = false;
	  if( b.isSelected() )
	  {
	    cost = 0.;
	    fill( varSimCost.begin(), varSimCost.end(), 0. );
	      
	    cost = ng.simulateCost( b, -1, varSimCost );
	      
	    if( cost == 0. )
	    {
	      domain.clear( b );
	      b.setValue( -1 );
	      ng.update( domain );
	      change = true;
	    }	  
	  }
	}
    } while( change );

    double objectiveCost = objective->cost( vecVariables, domain );
    int currentSizeWall = countSelectedVariables( vecVariables );

    if( objectiveCost < bestCost || ( objectiveCost == bestCost && currentSizeWall < sizeWall ) )
    {
      sizeWall = currentSizeWall;
      bestCost = objectiveCost;
      for( int i = 0; i < vecVariables.size(); ++i )
	bestSolution[i] = vecVariables[i]->getValue();
    }
  }

  void WallinObjective::v_postprocessOptimization( const vector< Building > &vecVariables, const WallinGrid &domain, double &bestCost ) const
  {
    vector<int> tabuList( vecVariables.size() );
    std::fill( tabuList.begin(), tabuList.end(), 0 );

    multimap<int, Building> buildingSameSize;
    
    for( auto v : vecVariables )
      buildingSameSize.insert( make_pair( v.getSurface(), v ) );

    vector<int> goodVar;
    shared_ptr<Variable> toSwap;
    bool mustSwap;

    chrono::time_point<chrono::system_clock> startPostprocess = chrono::system_clock::now(); 
    chrono::duration<double,milli> postprocessGap(0);
    
    bestCost = objective->cost( vecVariables, domain );
    double currentCost = bestCost;
    beforePostProc = bestCost;

    while( (postprocessGap = chrono::system_clock::now() - startPostprocess).count() < static_cast<int>( ceil(OPT_TIME / 100) ) && bestCost > 0 )
    {
      goodVar.clear();

      for( int i = 0; i < tabuList.size(); ++i )
      {
	if( tabuList[i] <= 1 )
	  tabuList[i] = 0;
	else
	  --tabuList[i];
      }

      for( int i = 0; i < vecVariables.size(); ++i )
      {
	if( tabuList[i] == 0 )
	  goodVar.push_back( i );
      }

      if( goodVar.empty() )
	for( int i = 0; i < vecVariables.size(); ++i )
	  goodVar.push_back( i );	

      int index = heuristicVariable( goodVar, vecVariables, domain );
      oldVariable = vecVariables[ index ];
      auto surface = buildingSameSize.equal_range( oldVariable.getSurface() );
	
      for( auto it = surface.first; it != surface.second; ++it )
      {
	mustSwap = false;
	if( it->second.getId() != oldVariable.getId() )
	{
	  domain.swap( it->second, oldVariable );
	    
	  currentCost = objective.cost( vecVariables, domain );
	  if( currentCost < bestCost )
	  {
	    bestCost = currentCost;
	    toSwap = it->second;
	    mustSwap = true;
	  }

	  domain.swap( it->second, oldVariable );
	}
	  
	if( mustSwap )
	  domain.swap( toSwap, oldVariable );
      }

      tabuList[ index ] = 2;//std::max(2, static_cast<int>( ceil(TABU / 2) ) );
    }
  }

  /***********/
  /* NoneObj */
  /***********/
  NoneObj::NoneObj( string name ) : WallinObjective( name ) { }

  double NoneObj::v_cost( const vector< Building > &vecVariables, const WallinGrid &grid ) const
  {
    return count_if( vecVariables.begin(), 
		     vecVariables.end(), 
		     []( const Building &b ){ return b.isSelected(); });
    //return 0.;
  }

  int NoneObj::v_heuristicVariable( const vector< int > &vecId, const vector< Building > &vecVariables, const WallinGrid &grid )
  {
    return vecId[ randomVar.getRandNum( vecId.size() ) ];
  }

  void NoneObj::v_setHelper( const Building &b, const vector< Building > &vecVariables, const WallinGrid &grid )
  {
    if( b.isSelected() )
      heuristicValueHelper.at( b.getValue() ) = 0;
  }

  void NoneObj::v_postprocessOptimization( const vector< Building > &vecVariables, const WallinGrid &domain, double &bestCost ) const { }
  
  /**********/
  /* GapObj */
  /**********/
  GapObj::GapObj( string name ) : WallinObjective( name ) { }

  double GapObj::v_cost( const vector< Building > &vecVariables, const WallinGrid &grid ) const
  {
    int gaps = 0;
    
    vector< Building > toVisit = vecVariables;

    while( !toVisit.empty() )
    {
      auto b = *(toVisit.begin());
      gaps += gapSize( b, toVisit, grid );
      toVisit.erase( toVisit.begin() );
    }

    return gaps;
  }

  int GapObj::v_heuristicVariable( const vector< int > &vecId, const vector< Building > &vecVariables, const WallinGrid &grid )
  {
    auto worst =  max_element(vecId.begin(),
			      vecId.end(),
			      [&](int v1, int v2)
			      {return gapSize( *vecVariables[v1], vecVariables, grid ) < gapSize( *vecVariables[v2], vecVariables, grid );} );

    return *(worst);
  }

  void GapObj::v_setHelper( const Building &b, const vector< Building > &vecVariables, const WallinGrid &grid )
  {
    if( b.isSelected() )
      heuristicValueHelper.at( b.getValue() ) = gapSize( b, vecVariables, grid );
  }

  int GapObj::gapSize( const Building &b, const vector< Building > &vecVariables, const WallinGrid &grid ) const
  {
    if( !b.isSelected() )
      return 0;

    int gaps = 0;
    set< Building > neighbors = grid.getBuildingsAbove( b, vecVariables );

    // cout << "ABOVE " << b->getId() << endl;
    // for( auto n : neighbors )
    //   cout << n->getId() << " ";
    // cout << endl;

    gaps += count_if( neighbors.begin(), 
		      neighbors.end(), 
		      [&](const Building &n){return b.getGapTop() + n.getGapBottom() >= 16;});
    
    neighbors = grid.getBuildingsOnRight( b, vecVariables );
    // cout << "RIGHT " << b->getId() << endl;
    // for( auto n : neighbors )
    //   cout << n->getId() << " ";
    // cout << endl;
    gaps += count_if( neighbors.begin(), 
		      neighbors.end(), 
		      [&](const Building &n){return b.getGapRight() + n.getGapLeft() >= 16;});
    
    neighbors = grid.getBuildingsBelow( b, vecVariables );
    // cout << "BELOW " << b->getId()  << endl;
    // for( auto n : neighbors )
    //   cout << n->getId() << " ";
    // cout << endl;
    gaps += count_if( neighbors.begin(), 
		      neighbors.end(), 
		      [&](const Building &n){return b.getGapBottom() + n.getGapTop() >= 16;});
    
    neighbors = grid.getBuildingsOnLeft( b, vecVariables );
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
  BuildingObj::BuildingObj( string name ) : WallinObjective( name ) { }

  double BuildingObj::v_cost( const vector< Building > &vecVariables, const WallinGrid &grid ) const
  {
    return count_if( vecVariables.begin(), 
		     vecVariables.end(), 
		     []( const Building &b ){ return b.isSelected(); });
  }

  int BuildingObj::v_heuristicVariable( const vector< int > &vecId, const vector< Building > &vecVariables, const WallinGrid &grid )
  {
    vector< int > varOnGrid( vecId.size() );
    
    auto it = copy_if( vecId.begin(),
		       vecId.end(),
		       varOnGrid.begin(),
		       [&](int b){return vecVariables[b].isSelected();} );

    int size = distance( varOnGrid.begin(), it );

    if( it == varOnGrid.begin() )
    {
      varOnGrid = vecId;
      size = vecId.size();
    }

    return varOnGrid[ randomVar.getRandNum( size ) ];    
  }

  void BuildingObj::v_postprocessOptimization( const vector< Building > &vecVariables, const WallinGrid &domain, double &bestCost ) const { }

  /***************/
  /* TechTreeObj */
  /***************/
  TechTreeObj::TechTreeObj( string name ) : WallinObjective( name ) { }

  double TechTreeObj::v_cost( const vector< Building > &vecVariables, const WallinGrid &grid ) const
  {
    vector< Building > onGrid( vecVariables.size() );
    
    auto it = copy_if( vecVariables.begin(),
		       vecVariables.end(), 
		       onGrid.begin(),
		       [](const Building &b){ return b.isSelected(); } );
    onGrid.resize( distance( onGrid.begin(), it ) );

    auto max =  max_element( onGrid.begin(), 
			     onGrid.end(), 
			     [](const Building &b1, const Building &b2)
			     {return b1.getTreedepth() < b2.getTreedepth();} );

    return max->getTreedepth();
  }

  int TechTreeObj::v_heuristicVariable( const vector< int > &vecId, const vector< Building > &vecVariables, const WallinGrid &grid )
  {
    // auto min =  min_element( vecVariables.begin(), 
    // 				  vecVariables.end(), 
    // 				  [](const Building &b1, const Building &b2)
    // 				  {return b1.getTreedepth() < b2.getTreedepth();} );

    // int minValue = min->getTreedepth();
    // vector< int > varMinTech( vecId.size() );

    // auto it = copy_if( vecId.begin(),
    // 			    vecId.end(),
    // 			    varMinTech.begin(),
    // 			    [&](int b){return vecVariables[b].getTreedepth() == minValue;} );

    auto min =  min_element( vecId.begin(), 
			     vecId.end(), 
			     [&](int b1, int b2)
			     { return vecVariables[b1].getTreedepth() < vecVariables[b2].getTreedepth(); } );

    vector< int > varMinTech( vecId.size() );
    
    auto it = copy_if( vecId.begin(),
		       vecId.end(),
		       varMinTech.begin(),
		       [&](int b){return vecVariables[b].getTreedepth() == min;} );
    
    int size = distance( varMinTech.begin(), it );

    if( it == varMinTech.begin() )
    {
      varMinTech = vecId;
      size = vecId.size();
    }
    
    return varMinTech[ randomVar.getRandNum( size ) ];    
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
