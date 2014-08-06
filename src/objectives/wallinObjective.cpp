/*
 * GHOST (General meta-Heuristic Optimization Solving Tool) is a C++ library 
 * designed for StarCraft: Brood war. 
 * GHOST is a meta-heuristic solver aiming to solve any kind of combinatorial 
 * and optimization RTS-related problems represented by a CSP/COP. 
 * It is a generalization of the project Wall-in.
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


#include <vector>
#include <map>
#include <memory>
#include <algorithm>
#include <limits>
#include <cmath>
#include <chrono>
#include <ctime>
#include <numeric>

#include "../../include/objectives/wallinObjective.hpp"
#include "../../include/variables/building.hpp"
#include "../../include/constraints/wallinConstraint.hpp"

using namespace std;

namespace ghost
{
  int WallinObjective::sizeWall = numeric_limits<int>::max();
  
  /*******************/
  /* WallinObjective */
  /*******************/
  WallinObjective::WallinObjective( const string &name ) : Objective<Building, WallinDomain>( name ) { }

  void WallinObjective::v_setHelper( const Building &b, const vector< Building > *vecVariables, const WallinDomain *domain )
  {
    if( b.isSelected() )
    {
      int pos = b.getValue();
      heuristicValueHelper.at( pos ) = domain->distanceToTarget( pos );
    }
  }

  double WallinObjective::v_postprocessSatisfaction( vector< Building > *vecVariables,
						     WallinDomain *domain,
						     double &bestCost,
						     vector< Building > &bestSolution ) const 
  {
    chrono::time_point<chrono::high_resolution_clock> startPostprocess = chrono::high_resolution_clock::now(); 

    bool change;
    double cost;
    NoHoles nh( vecVariables, domain );

    // find all buildings accessible from the starting building and remove all others
    int nberCurrent = *( domain->buildingsAt( domain->getStartingTile() ).begin() );
    Building current = vecVariables->at( nberCurrent );
    set< Building > toVisit = domain->getBuildingsAround( current, vecVariables );
    set< Building > visited;
    set< Building > neighbors;
    
    visited.insert( current );
    
    while( !toVisit.empty() )
    {
      auto first = *( toVisit.begin() );
      current = first;
      toVisit.erase( first );
      neighbors = domain->getBuildingsAround( current, vecVariables );
      
      visited.insert( current );
      
      for( const auto &n : neighbors )
	if( visited.find( n ) == visited.end() )
	  toVisit.insert( n );
    }
    
    // remove all unreachable buildings from the starting building out of the domain
    for( auto &b : *vecVariables )
      if( visited.find( b ) == visited.end() )
      {
	domain->clear( b );
	b.setValue( -1 );
      }

    vector<double> varSimCost( vecVariables->size() );

    // clean wall from unnecessary buildings.
    do
    {
      for( auto &b : *vecVariables )
	if( ! domain->isStartingOrTargetTile( b.getId() ) )
	{
	  change = false;
	  if( b.isSelected() )
	  {
	    cost = 0.;
	    fill( varSimCost.begin(), varSimCost.end(), 0. );
	      
	    cost = nh.postprocess_simulateCost( b, -1, varSimCost );
	      
	    if( cost == 0. )
	    {
	      domain->clear( b );
	      b.setValue( -1 );
	      change = true;
	    }	  
	  }
	}
    } while( change );

    double objectiveCost = this->cost( vecVariables, domain );
    int currentSizeWall = std::count_if( vecVariables->begin(), vecVariables->end(), []( const Building &b ){ return b.isSelected(); });

    if( objectiveCost < bestCost || ( objectiveCost == bestCost && currentSizeWall < sizeWall ) )
    {
      sizeWall = currentSizeWall;
      bestCost = objectiveCost;
      for( int i = 0; i < vecVariables->size(); ++i )
	bestSolution[i] = vecVariables->at(i);
    }

    return (chrono::high_resolution_clock::now() - startPostprocess).count();
  }

  double WallinObjective::v_postprocessOptimization( vector< Building > *vecVariables, WallinDomain *domain, double &bestCost ) 
  {
    chrono::time_point<chrono::high_resolution_clock> startPostprocess = chrono::high_resolution_clock::now(); 
    chrono::duration<double,micro> postprocesstimer(0);

    vector<int> tabuList( vecVariables->size() );
    std::fill( tabuList.begin(), tabuList.end(), 0 );

    multimap<int, Building> buildingSameSize;
    
    for( const auto &v : *vecVariables )
      buildingSameSize.insert( make_pair( v.getSurface(), v ) );

    Building *oldVariable;
    vector<int> goodVar;
    Building *toSwap;
    bool mustSwap;
    
    bestCost = v_cost( vecVariables, domain );
    double currentCost = bestCost;

    int postprocessTimeLimit = std::max( 1, static_cast<int>( ceil( static_cast<double>(OPT_TIME) / 100) ) );

    while( (postprocesstimer = chrono::high_resolution_clock::now() - startPostprocess).count() < postprocessTimeLimit && bestCost > 0 )
    {
      goodVar.clear();

      for( int i = 0; i < tabuList.size(); ++i )
      {
	if( tabuList[i] <= 1 )
	  tabuList[i] = 0;
	else
	  --tabuList[i];
      }

      for( int i = 0; i < vecVariables->size(); ++i )
      {
	if( tabuList[i] == 0 )
	  goodVar.push_back( i );
      }

      if( goodVar.empty() )
	for( int i = 0; i < vecVariables->size(); ++i )
	  goodVar.push_back( i );	

      int index = v_heuristicVariable( goodVar, vecVariables, domain );
      oldVariable = &vecVariables->at( index );
      auto surface = buildingSameSize.equal_range( oldVariable->getSurface() );
	
      for( auto &it = surface.first; it != surface.second && bestCost != 0; ++it )
      {
	mustSwap = false;
	if( it->second.getId() != oldVariable->getId() )
	{
	  domain->swap( vecVariables->at(it->second.getId()), *oldVariable );
	  currentCost = v_cost( vecVariables, domain );
	  if( currentCost < bestCost )
	  {
	    bestCost = currentCost;
	    toSwap = &( vecVariables->at( it->second.getId() ) );
	    mustSwap = true;
	  }

	  domain->swap( vecVariables->at( it->second.getId() ), *oldVariable );
	}
	  
	if( mustSwap )
	  domain->swap( *toSwap, *oldVariable );
      }

      tabuList[ index ] = 2;
    }

    return postprocesstimer.count();
  }

  // /***********/
  // /* NoneObj */
  // /***********/
  // NoneObj::NoneObj() : WallinObjective( "wallinNone" ) { }

  // double NoneObj::v_cost( const vector< Building > *vecVariables, const WallinDomain *domain ) const
  // {
  //   return count_if( vecVariables->begin(), 
  // 		     vecVariables->end(), 
  // 		     []( const Building &b ){ return b.isSelected(); });
  // }

  // int NoneObj::v_heuristicVariable( const vector< int > &vecId, const vector< Building > *vecVariables, WallinDomain *domain )
  // {
  //   return vecId[ randomVar.getRandNum( vecId.size() ) ];
  // }

  // void NoneObj::v_setHelper( const Building &b, const vector< Building > *vecVariables, const WallinDomain *domain )
  // {
  //   if( b.isSelected() )
  //     heuristicValueHelper.at( b.getValue() ) = 0;
  // }

  // double NoneObj::v_postprocessOptimization( vector< Building > *vecVariables, WallinDomain *domain, double &bestCost ) { return 0.; }
  
  /**********/
  /* GapObj */
  /**********/
  GapObj::GapObj() : WallinObjective( "wallinGap" ) { }

  double GapObj::v_cost( const vector< Building > *vecVariables, const WallinDomain *domain ) const
  {
    int gaps = 0;
    
    vector< Building > toVisit = *vecVariables;

    while( !toVisit.empty() )
    {
      auto b = *(toVisit.begin());
      gaps += gapSize( b, &toVisit, domain );
      toVisit.erase( toVisit.begin() );
    }

    return gaps;
  }

  int GapObj::v_heuristicVariable( const vector< int > &vecId, const vector< Building > *vecVariables, WallinDomain *domain )
  {
    vector<int> worstVec;
    
    auto worst =  max_element(vecId.begin(),
			      vecId.end(),
			      [&](int v1, int v2)
			      {return gapSize( vecVariables->at(v1), vecVariables, domain ) < gapSize( vecVariables->at(v2), vecVariables, domain );} );
    
    int worstGap = gapSize( vecVariables->at(*worst), vecVariables, domain );
    
    for( const auto v : vecId )
      if( gapSize( vecVariables->at(v), vecVariables, domain ) == worstGap )
	worstVec.push_back(v);
    
    int index;
    
    if( worstVec.size() > 1 )
      index = worstVec[ randomVar.getRandNum( worstVec.size() ) ];
    else
      index = *(worst);
    
    return index; 
  }

  void GapObj::v_setHelper( const Building &b, const vector< Building > *vecVariables, const WallinDomain *domain )
  {
    if( b.isSelected() )
      heuristicValueHelper.at( b.getValue() ) = gapSize( b, vecVariables, domain );
  }

  int GapObj::gapSize( const Building &b, const vector< Building > *vecVariables, const WallinDomain *domain ) const
  {
    if( !b.isSelected() )
      return 0;

    int gaps = 0;
    set< Building > neighbors = domain->getBuildingsAbove( b, vecVariables );

    gaps += count_if( neighbors.begin(), 
		      neighbors.end(), 
		      [&](const Building &n){return b.getGapTop() + n.getGapBottom() >= 16;});
    
    neighbors = domain->getBuildingsOnRight( b, vecVariables );
    gaps += count_if( neighbors.begin(), 
		      neighbors.end(), 
		      [&](const Building &n){return b.getGapRight() + n.getGapLeft() >= 16;});
    
    neighbors = domain->getBuildingsBelow( b, vecVariables );
    gaps += count_if( neighbors.begin(), 
		      neighbors.end(), 
		      [&](const Building &n){return b.getGapBottom() + n.getGapTop() >= 16;});
    
    neighbors = domain->getBuildingsOnLeft( b, vecVariables );
    gaps += count_if( neighbors.begin(), 
		      neighbors.end(), 
		      [&](const Building &n){return b.getGapLeft() + n.getGapRight() >= 16;});

    
    return gaps;
  }

  /***************/
  /* BuildingObj */
  /***************/
  BuildingObj::BuildingObj() : WallinObjective( "wallinBuilding" ) { }

  double BuildingObj::v_cost( const vector< Building > *vecVariables, const WallinDomain *domain ) const
  {
    return count_if( vecVariables->begin(), 
		     vecVariables->end(), 
		     []( const Building &b ){ return b.isSelected(); });
  }

  int BuildingObj::v_heuristicVariable( const vector< int > &vecId, const vector< Building > *vecVariables, WallinDomain *domain ) 
  {
    vector< int > varOnDomain( vecId.size() );
    
    auto it = copy_if( vecId.begin(),
		       vecId.end(),
		       varOnDomain.begin(),
		       [&](int b){return vecVariables->at(b).isSelected();} );

    int size = distance( varOnDomain.begin(), it );

    if( it == varOnDomain.begin() )
    {
      varOnDomain = vecId;
      size = vecId.size();
    }

    return varOnDomain[ randomVar.getRandNum( size ) ];    
  }

  double BuildingObj::v_postprocessOptimization( vector< Building > *vecVariables, WallinDomain *domain, double &bestCost ) { return 0.; }

  /***************/
  /* TechTreeObj */
  /***************/
  TechTreeObj::TechTreeObj() : WallinObjective( "wallinTechtree" ) { }

  double TechTreeObj::v_cost( const vector< Building > *vecVariables, const WallinDomain *domain ) const
  {
    vector< Building > onDomain( vecVariables->size() );
    
    auto it = copy_if( vecVariables->begin(),
		       vecVariables->end(), 
		       onDomain.begin(),
		       [](const Building &b){ return b.isSelected(); } );
    onDomain.resize( distance( onDomain.begin(), it ) );

    auto max =  max_element( onDomain.begin(), 
			     onDomain.end(), 
			     [](const Building &b1, const Building &b2)
			     {return b1.getTreedepth() < b2.getTreedepth();} );

    return max->getTreedepth();
  }

  int TechTreeObj::v_heuristicVariable( const vector< int > &vecId, const vector< Building > *vecVariables, WallinDomain *domain )
  {
    auto min =  min_element( vecId.begin(), 
			     vecId.end(), 
			     [&](int b1, int b2)
			     { return vecVariables->at(b1).getTreedepth() < vecVariables->at(b2).getTreedepth(); } );

    vector< int > varMinTech( vecId.size() );
    
    auto it = copy_if( vecId.begin(),
		       vecId.end(),
		       varMinTech.begin(),
		       [&](int b){return vecVariables->at(b).getTreedepth() == *min;} );
    
    int size = distance( varMinTech.begin(), it );

    if( it == varMinTech.begin() )
    {
      varMinTech = vecId;
      size = vecId.size();
    }
    
    return varMinTech[ randomVar.getRandNum( size ) ];    
  }
}
