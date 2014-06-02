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


#include "../include/objective.hpp"

namespace wallin
{
  /*************/
  /* Objective */
  /*************/
  Objective::Objective( std::string name ) : name(name) { }

  int Objective::heuristicValue( const std::vector< double > &vecGlobalCosts, 
				 double &bestEstimatedCost,
				 int &bestPosition,
				 const Grid &grid ) const
  {
    int best = 0;
    double bestHelp = numeric_limits<int>::max();

    for( int i = 0; i < vecGlobalCosts.size(); ++i )
    {
      if(      vecGlobalCosts[i] < bestEstimatedCost
	  || ( vecGlobalCosts[i] == bestEstimatedCost
	       && vecGlobalCosts[i] < numeric_limits<int>::max()
	       && ( i == 0 || heuristicValueHelper.at( i ) < bestHelp ) ) )
      {
	bestEstimatedCost = vecGlobalCosts[i];
	bestPosition = i - 1;
	if( heuristicValueHelper.at( i ) < bestHelp )
	  bestHelp = heuristicValueHelper.at( i );
	best = i;
      }
    }

    return best;
  }

  void Objective::initHelper( int size )
  {
    heuristicValueHelper = std::vector<double>( size, numeric_limits<int>::max() );
  }

  void Objective::resetHelper()
  {
    std::fill( heuristicValueHelper.begin(), heuristicValueHelper.end(), numeric_limits<int>::max() );
  }

  void Objective::setHelper( const Building &b, const std::vector< std::shared_ptr<Building> > &vecBuildings, const Grid &grid )
  {
    if( b.isOnGrid() )
    {
      int pos = b.getPosition();
      heuristicValueHelper.at( pos ) = grid.distanceToTarget( pos );
    }
  }

  /***********/
  /* NoneObj */
  /***********/
  NoneObj::NoneObj( std::string name ) : Objective( name ) { }

  double NoneObj::cost( const std::vector< std::shared_ptr<Building> > &vecBuildings, const Grid &grid ) const
  {
    // return std::count_if( vecBuildings.begin(), 
    // 			  vecBuildings.end(), 
    // 			  []( const std::shared_ptr<Building> &b ){ return b->isOnGrid(); });
    return 0.;
  }

  int NoneObj::heuristicVariable( const std::vector< int > &vecVariables, const std::vector< std::shared_ptr<Building> > &vecBuildings, const Grid &grid )
  {
    return vecVariables[ randomVar.getRandNum( vecVariables.size() ) ];
  }

  void NoneObj::setHelper( const Building &b, const std::vector< std::shared_ptr<Building> > &vecBuildings, const Grid &grid )
  {
    if( b.isOnGrid() )
      heuristicValueHelper.at( b.getPosition() ) = 0;
  }

  /**********/
  /* GapObj */
  /**********/
  GapObj::GapObj( std::string name ) : Objective( name ) { }

  double GapObj::cost( const std::vector< std::shared_ptr<Building> > &vecBuildings, const Grid &grid ) const
  {
    int gaps = 0;
    
    std::vector< std::shared_ptr<Building> > toVisit = vecBuildings;

    while( !toVisit.empty() )
    {
      auto b = *(toVisit.begin());
      gaps += gapSize( *b, toVisit, grid );
      toVisit.erase( toVisit.begin() );
    }

    return gaps;
  }

  int GapObj::heuristicVariable( const std::vector< int > &vecVariables, const std::vector< std::shared_ptr<Building> > &vecBuildings, const Grid &grid )
  {
    auto worst =  std::max_element(vecVariables.begin(),
				   vecVariables.end(),
				   [&](int v1, int v2)
				   {return gapSize( *vecBuildings[v1], vecBuildings, grid ) < gapSize( *vecBuildings[v2], vecBuildings, grid );} );

    return *(worst);
  }

  void GapObj::setHelper( const Building &b, const std::vector< std::shared_ptr<Building> > &vecBuildings, const Grid &grid )
  {
    if( b.isOnGrid() )
      heuristicValueHelper.at( b.getPosition() ) = gapSize( b, vecBuildings, grid );
  }

  int GapObj::gapSize( const Building &b, const std::vector< std::shared_ptr<Building> > &vecBuildings, const Grid &grid ) const
  {
    if( !b.isOnGrid() )
      return 0;

    int gaps = 0;
    std::set< std::shared_ptr<Building> > neighbors = grid.getBuildingsAbove( b, vecBuildings );

    // std::cout << "ABOVE " << b->getId() << std::endl;
    // for( auto n : neighbors )
    //   std::cout << n->getId() << " ";
    // std::cout << std::endl;

    gaps += std::count_if( neighbors.begin(), 
			   neighbors.end(), 
			   [&](const std::shared_ptr<Building> &n){return b.getGapTop() + n->getGapBottom() >= 16;});
    
    neighbors = grid.getBuildingsOnRight( b, vecBuildings );
    // std::cout << "RIGHT " << b->getId() << std::endl;
    // for( auto n : neighbors )
    //   std::cout << n->getId() << " ";
    // std::cout << std::endl;
    gaps += std::count_if( neighbors.begin(), 
			   neighbors.end(), 
			   [&](const std::shared_ptr<Building> &n){return b.getGapRight() + n->getGapLeft() >= 16;});
    
    neighbors = grid.getBuildingsBelow( b, vecBuildings );
    // std::cout << "BELOW " << b->getId()  << std::endl;
    // for( auto n : neighbors )
    //   std::cout << n->getId() << " ";
    // std::cout << std::endl;
    gaps += std::count_if( neighbors.begin(), 
			   neighbors.end(), 
			   [&](const std::shared_ptr<Building> &n){return b.getGapBottom() + n->getGapTop() >= 16;});
    
    neighbors = grid.getBuildingsOnLeft( b, vecBuildings );
    // std::cout << "LEFT " << b->getId()  << std::endl;
    // for( auto n : neighbors )
    //   std::cout << n->getId() << " ";
    // std::cout << std::endl;
    gaps += std::count_if( neighbors.begin(), 
			   neighbors.end(), 
			   [&](const std::shared_ptr<Building> &n){return b.getGapLeft() + n->getGapRight() >= 16;});

    return gaps;
  }

  /***************/
  /* BuildingObj */
  /***************/
  BuildingObj::BuildingObj( std::string name ) : Objective( name ) { }

  double BuildingObj::cost( const std::vector< std::shared_ptr<Building> > &vecBuildings, const Grid &grid ) const
  {
    return std::count_if( vecBuildings.begin(), 
			  vecBuildings.end(), 
			  []( const std::shared_ptr<Building> &b ){ return b->isOnGrid(); });
  }

  int BuildingObj::heuristicVariable( const std::vector< int > &vecVariables, const std::vector< std::shared_ptr<Building> > &vecBuildings, const Grid &grid )
  {
    std::vector< int > varOnGrid( vecVariables.size() );
    
    auto it = std::copy_if( vecVariables.begin(),
    			    vecVariables.end(),
    			    varOnGrid.begin(),
    			    [&](int b){return vecBuildings[b]->isOnGrid();} );

    int size = std::distance( varOnGrid.begin(), it );

    if( it == varOnGrid.begin() )
    {
      varOnGrid = vecVariables;
      size = vecVariables.size();
    }

    return varOnGrid[ randomVar.getRandNum( size ) ];    
  }

  /***************/
  /* TechTreeObj */
  /***************/
  TechTreeObj::TechTreeObj( std::string name ) : Objective( name ) { }

  double TechTreeObj::cost( const std::vector< std::shared_ptr<Building> > &vecBuildings, const Grid &grid ) const
  {
    std::vector< std::shared_ptr<Building> > onGrid( vecBuildings.size() );
    
    auto it = std::copy_if( vecBuildings.begin(),
			    vecBuildings.end(), 
			    onGrid.begin(),
			    [](const std::shared_ptr<Building> &b){ return b->isOnGrid(); } );
    onGrid.resize( std::distance( onGrid.begin(), it ) );

    auto max =  std::max_element( onGrid.begin(), 
				  onGrid.end(), 
				  [](const std::shared_ptr<Building> &b1, const std::shared_ptr<Building> &b2)
				  {return b1->getTreedepth() < b2->getTreedepth();} );

    return (*max)->getTreedepth();
  }

  int TechTreeObj::heuristicVariable( const std::vector< int > &vecVariables, const std::vector< std::shared_ptr<Building> > &vecBuildings, const Grid &grid )
  {
    // auto min =  std::min_element( vecBuildings.begin(), 
    // 				  vecBuildings.end(), 
    // 				  [](const std::shared_ptr<Building> &b1, const std::shared_ptr<Building> &b2)
    // 				  {return b1->getTreedepth() < b2->getTreedepth();} );

    // int minValue = (*min)->getTreedepth();
    // std::vector< int > varMinTech( vecVariables.size() );

    // auto it = std::copy_if( vecVariables.begin(),
    // 			    vecVariables.end(),
    // 			    varMinTech.begin(),
    // 			    [&](int b){return vecBuildings[b]->getTreedepth() == minValue;} );

    auto min =  std::min_element( vecVariables.begin(), 
				  vecVariables.end(), 
				  [&](int b1, int b2)
				  { return vecBuildings[b1]->getTreedepth() < vecBuildings[b2]->getTreedepth(); } );

    std::vector< int > varMinTech( vecVariables.size() );

    auto it = std::copy_if( vecVariables.begin(),
			    vecVariables.end(),
			    varMinTech.begin(),
			    [&](int b){return vecBuildings[b]->getTreedepth() == *min;} );
    
    int size = std::distance( varMinTech.begin(), it );

    if( it == varMinTech.begin() )
    {
      varMinTech = vecVariables;
      size = vecVariables.size();
    }

    return varMinTech[ randomVar.getRandNum( size ) ];    
  }

  /**************/
  /* FactoryObj */
  /**************/
  std::shared_ptr<Objective> FactoryObj::makeObjective( const std::string &obj ) const
  {
    if( obj.compare("gap") == 0 || obj.compare("g") == 0 || obj.compare("G") == 0 )
      return std::make_shared<GapObj>("gap");
    
    if( obj.compare("building") == 0 || obj.compare("b") == 0 || obj.compare("B") == 0 )
      return std::make_shared<BuildingObj>("building");
    
    if( obj.compare("techtree") == 0 || obj.compare("t") == 0 || obj.compare("T") == 0 )
      return std::make_shared<TechTreeObj>("techtree");
    
    return std::make_shared<NoneObj>("none");
  }
}
