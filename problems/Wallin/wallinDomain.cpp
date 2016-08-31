/*
 * GHOST (General meta-Heuristic Optimization Solving Tool) is a C++ library 
 * designed to help developers to model and implement optimization problem 
 * solving. 
 * GHOST is a meta-heuristic solver aiming to solve any kind of combinatorial 
 * and optimization real-time problems represented by a CSP/COP. 
 * GHOST has been first developped to help making AI for the RTS game
 * StarCraft: Brood war, but can be used for any kind of applications where 
 * solving combinatorial and optimization problems within some tenth of 
 * milliseconds is needed. It is a generalization of the Wall-in project.
 * Please visit https://github.com/richoux/GHOST for further information.
 * 
 * Copyright (C) 2014-2016 Florian Richoux
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


#include <iomanip>
#include <algorithm>

#include "wallinDomain.hpp"

namespace ghost
{
  WallinDomain::WallinDomain( int col,
			      int row,
			      int nbVar,
			      int sRow,
			      int sCol,
			      int tRow,
			      int tCol ) 
    : Domain(col*row+1, nbVar, -1),
      mCol_(col),
      nRow_(row),
      matrixType_(vector< vector<string> >(nRow_, vector<string>(mCol_, "") ) ),
      matrixId_(vector< vector< set<int> > >(nRow_, vector< set<int> >(mCol_, set<int>() ) ) ),
      startingTile( make_pair( sRow, sCol ) ),
      targetTile( make_pair( tRow, tCol ) )
  { 
    matrixType_[sRow][sCol] += "@s";
    matrixType_[tRow][tCol] += "@t";
  }

  WallinDomain::WallinDomain( int col,
			      int row,
			      const vector< pair<int, int> > &unbuildables,
			      const vector< Building > *variables,
			      int sRow,
			      int sCol,
			      int tRow,
			      int tCol ) 
    : WallinDomain( col, row, variables->size(), sRow, sCol, tRow, tCol )
  {
    for( const auto &u : unbuildables )
      matrixType_[u.first][u.second].assign(3, '#');

    for( const auto &v : *variables )
      domains[ v.getId() ] = possiblePos( v );
  }

  void WallinDomain::add( const Building& building )
  {
    if( building.isSelected() )
    {
      pair<int, int> pos = lin2mat( building.getValue() );
      int row = pos.first;
      int col = pos.second;
     
      for( int x = row; x < row + building.getHeight(); ++x )
	for( int y = col; y < col + building.getLength(); ++y )
	  add(x, y, building.getName(), building.getId() );
    }
  }

  void WallinDomain::add( int row, int col, string b_short, int b_id )
  {
    bool fail = ! ( matrixType_[row][col].empty() 
		    || ( matrixType_[row][col].find("@") != string::npos && matrixType_[row][col].size() <= 3) );

    matrixType_[row][col] += b_short;
    matrixId_[row][col].insert( b_id );
    if( fail )
    {
      pair<int, int> key(row, col);
      if( failures_.find( key ) == failures_.end() )
	failures_.emplace( key, matrixType_[row][col] );
      else
	failures_.at( key ) += b_short;
    }
  }
  
  void WallinDomain::clear( const Building& building )
  {
    if( building.isSelected() )
    {
      pair<int, int> pos = lin2mat( building.getValue() );
      int row = pos.first;
      int col = pos.second;
      
      for( int x = row; x < row + building.getHeight(); ++x )
	for( int y = col; y < col + building.getLength(); ++y )
	  clear(x, y, building.getName(), building.getId() );
    }
  }

  void WallinDomain::clear( int row, int col, string b_short, int b_id )
  {
    auto it = matrixType_[row][col].find( b_short );
    if( it != string::npos )
    {
      matrixType_[row][col].replace( it,
				     b_short.length(),
				     "" );
      matrixId_[row][col].erase( b_id );
      
      pair<int, int> key(row, col);
      mapFail::iterator it = failures_.find( key );
      
      if( it != failures_.end() )
      {
	if( matrixType_[row][col].size() < 2 
	    || matrixType_[row][col].compare("###") == 0 
	    || ( matrixType_[row][col].size() == 2 && matrixType_[row][col].find("@") != string::npos ) )
	  failures_.erase( it );
	else
	  failures_.at( key ) = matrixType_[row][col];
      }
    }
  }

  pair<int, int> WallinDomain::shift( Building &building )
  {
    int overlaps = 0;
    int unbuildables = 0;
    
    if( building.isSelected() )
    {
      pair<int, int> pos = lin2mat( building.getValue() );
      int row = pos.first;
      int col = pos.second;

      int row_shift = row + building.getHeight();
      int col_shift = col + building.getLength();

      pair<int, int> key;

      for( int x = row; x < row_shift; ++x )
      {
	add(x, col_shift, building.getName(), building.getId() );	

	key = make_pair( x, col_shift );
	if( failures_.find( key ) != failures_.end() )
	{
	  if( failures_.at( key ).find( "###" ) == std::string::npos )
	    ++overlaps;
	  else
	    ++unbuildables;
	}

	key = make_pair( x, col );
	if( failures_.find( key ) != failures_.end() )
	{
	  if( failures_.at( key ).find( "###" ) == std::string::npos )
	    --overlaps;
	  else
	    --unbuildables;
	}

	clear(x, col, building.getName(), building.getId() );
      }
      
      building.shiftValue();
    }

    return make_pair( overlaps, unbuildables );
  }

  void WallinDomain::quickShift( Building &building )
  {
    if( building.isSelected() )
    {
      pair<int, int> pos = lin2mat( building.getValue() );
      int row = pos.first;
      int col = pos.second;

      int row_shift = row + building.getHeight();
      int col_shift = col + building.getLength();

      for( int x = row; x < row_shift; ++x )
      {
	add(x, col_shift, building.getName(), building.getId() );	
	clear(x, col, building.getName(), building.getId() );
      }
      
      building.shiftValue();
    }
  }

  void WallinDomain::swap( Building &first, Building &second )
  {
    clear( first );
    clear( second );
    first.swapValue( second );
    add( first );
    add( second );
  }  

  set< Building > WallinDomain::getBuildingsAround ( const Building &b, const vector< Building > *variables ) const
  {
    set< Building > myNeighbors;

    if( b.isSelected() )
    {
      pair<int, int> coordinates = lin2mat( b.getValue() );

      int top = coordinates.first;
      int right = coordinates.second + b.getLength() - 1;
      int bottom = coordinates.first + b.getHeight() - 1;
      int left = coordinates.second;

      for( const auto &other : *variables )
      {
  	if( other.getId() != b.getId() && other.isSelected() )
  	{
  	  pair<int, int> xyOther = lin2mat( other.getValue() );
  	  int otherTop = xyOther.first;
  	  int otherRight = xyOther.second + other.getLength() - 1;
  	  int otherBottom = xyOther.first + other.getHeight() - 1;
  	  int otherLeft = xyOther.second;

  	  if(  ( top == otherBottom + 1 && ( otherRight >= left && otherLeft <= right ) )
  	       || ( right == otherLeft - 1 && ( otherBottom >= top - 1 && otherTop <= bottom + 1 ) )
  	       || ( bottom == otherTop - 1 && ( otherRight >= left && otherLeft <= right ) )
  	       || ( left == otherRight + 1 && ( otherBottom >= top - 1 && otherTop <= bottom + 1 ) ) )
  	  {
  	    myNeighbors.insert( other );
  	  }
  	}
      }
    }
    
    return myNeighbors;
  }

  set< Building > WallinDomain::getBuildingsAbove ( const Building &b, const vector< Building > *variables ) const
  {
    set< Building > myNeighbors;

    if( b.isSelected() )
    {
      pair<int, int> coordinates = lin2mat( b.getValue() );

      int top = coordinates.first;
      int right = coordinates.second + b.getLength() - 1;
      int left = coordinates.second;

      for( const auto &other : *variables )
      {
	if( other.getId() != b.getId() && other.isSelected() )
	{
	  pair<int, int> xyOther = lin2mat( other.getValue() );
	  int otherRight = xyOther.second + other.getLength() - 1;
	  int otherBottom = xyOther.first + other.getHeight() - 1;
	  int otherLeft = xyOther.second;

	  if( top == otherBottom + 1 && otherRight >= left && otherLeft <= right )
	    myNeighbors.insert( other );
	}
      }
    }
    
    return myNeighbors;
  }

  set< Building > WallinDomain::getBuildingsOnRight ( const Building &b, const vector< Building > *variables ) const
  {
    set< Building > myNeighbors;

    if( b.isSelected() )
    {
      pair<int, int> coordinates = lin2mat( b.getValue() );

      int top = coordinates.first;
      int right = coordinates.second + b.getLength() - 1;
      int bottom = coordinates.first + b.getHeight() - 1;

      for( const auto &other : *variables )
      {
	if( other.getId() != b.getId() && other.isSelected() )
	{
	  pair<int, int> xyOther = lin2mat( other.getValue() );
	  int otherTop = xyOther.first;
	  int otherBottom = xyOther.first + other.getHeight() - 1;
	  int otherLeft = xyOther.second;

	  if( right == otherLeft - 1 && otherBottom >= top - 1 && otherTop <= bottom + 1 )
	    myNeighbors.insert( other );
	}
      }
    }
    
    return myNeighbors;
  }

  set< Building > WallinDomain::getBuildingsBelow ( const Building &b, const vector< Building > *variables ) const
  {
    set< Building > myNeighbors;

    if( b.isSelected() )
    {
      pair<int, int> coordinates = lin2mat( b.getValue() );

      int right = coordinates.second + b.getLength() - 1;
      int bottom = coordinates.first + b.getHeight() - 1;
      int left = coordinates.second;

      for( const auto &other : *variables )
      {
	if( other.getId() != b.getId() && other.isSelected() )
	{
	  pair<int, int> xyOther = lin2mat( other.getValue() );
	  int otherTop = xyOther.first;
	  int otherRight = xyOther.second + other.getLength() - 1;
	  int otherLeft = xyOther.second;

	  if( bottom == otherTop - 1 && otherRight >= left && otherLeft <= right )
	    myNeighbors.insert( other );
	}
      }
    }
    
    return myNeighbors;
  }

  set< Building > WallinDomain::getBuildingsOnLeft ( const Building &b, const vector< Building > *variables ) const
  {
    set< Building > myNeighbors;

    if( b.isSelected() )
    {
      pair<int, int> coordinates = lin2mat( b.getValue() );

      int top = coordinates.first;
      int bottom = coordinates.first + b.getHeight() - 1;
      int left = coordinates.second;

      for( const auto &other : *variables )
      {
	if( other.getId() != b.getId() && other.isSelected() )
	{
	  pair<int, int> xyOther = lin2mat( other.getValue() );
	  int otherTop = xyOther.first;
	  int otherRight = xyOther.second + other.getLength() - 1;
	  int otherBottom = xyOther.first + other.getHeight() - 1;

	  if( left == otherRight + 1 && otherBottom >= top - 1 && otherTop <= bottom + 1 )
	    myNeighbors.insert( other );
	}
      }
    }
    
    return myNeighbors;
  }

  int WallinDomain::distanceTo( int source, pair<int, int> target ) const
  {
    pair<int, int> sourcePair = lin2mat( source );
    return abs( target.first - sourcePair.first ) + abs( target.second - sourcePair.second );
  }

  void WallinDomain::unbuildable( vector< pair<int, int> > unbuildables )
  {
    for( const auto &u : unbuildables )
      this->unbuildable( u.first, u.second );    
  }

  bool WallinDomain::isStartingOrTargetTile( int id ) const
  {
    auto startingBuildings = buildingsAt( getStartingTile() );
    auto targetBuildings = buildingsAt( getTargetTile() );

    return startingBuildings.find( id ) != startingBuildings.end()
      || targetBuildings.find( id ) != targetBuildings.end();
  }

  bool WallinDomain::isNeightborOfSTTBuildings( const Building &building, vector< Building > others  ) const
  {
    auto startingBuildings = buildingsAt( getStartingTile() );
    auto targetBuildings = buildingsAt( getTargetTile() );

    remove_if( begin(others), end(others), [&](Building b)
	       {
		 return ( find( begin(startingBuildings), end(startingBuildings), b.getId() ) == end(startingBuildings) )
		   &&
		   ( find( begin(targetBuildings), end(targetBuildings), b.getId() ) == end(targetBuildings) );
	       }
	       );

    return getBuildingsAround( building, &others ).size() != 0;
  }
  
  int WallinDomain::countAround( const Building &b, const vector< Building > *variables ) const
  {
    if( b.isSelected() )
      return getBuildingsAround( b, variables ).size();
    else
      return 0;
  }

  vector<int> WallinDomain::possiblePos( const Building& b ) const
  {
    vector<int> possiblePositions;

    possiblePositions.push_back( -1 );

    for( int row = 0; row <= nRow_ - b.getHeight(); ++row )
      for( int col = 0; col <= mCol_ - b.getLength(); ++col )
	if( matrixType_[row][col].compare("###") != 0
	    &&
	    matrixType_[row][col+b.getLength()-1].compare("###") != 0 )
	{
	  possiblePositions.push_back( mat2lin(row, col) );
	}
    
    return possiblePositions;
  }

  void WallinDomain::v_restart( vector<Building> *variables )
  {
    for( const auto &v : *variables )
      clear( v );
    
    for( auto &v : *variables )
    {
      // 1 chance over 3 to be placed on the domain
      if( random.getRandNum(3) == 0)
      {
	v.setValue( randomValue( v ) );
	add( v );
      }
      else
	v.setValue( -1 );
    }
  }

  void WallinDomain::v_wipe( vector<Building> *variables )
  {
    for( const auto &v : *variables )
      clear( v );
  }
  
  void WallinDomain::v_rebuild( vector<Building> *variables )
  {
    for( const auto &v : *variables )
      add( v );
  }
  
  ostream& operator<<( ostream& os, const WallinDomain& g )
  {
    os << "#rows: " <<  g.nRow_ << endl
       << "#columns: " <<  g.mCol_ << endl
       << "Matrix Id:" << endl;

    string bar = "";
    for( int i=0; i<g.matrixType_[0].size(); ++i )
      bar += "------";

    for( const auto &vec : g.matrixId_ )
    {
      os << bar << endl << "| ";
      for( const auto &setId : vec )
      {
	if( setId.empty() )
	  os << setw(3) << "    | ";
	else
	{
	  for( const auto &id : setId )
	    os << setw(3) << to_string( id ) << " | ";
	}
      }
      os << endl;
    }
    os << bar << endl << endl;

    os << "Matrix Type:" << endl;
    for( const auto &vec : g.matrixType_ )
    {
      os << bar << endl << "| ";
      for( const auto &str : vec )
	os << setw(3) << (str.empty() ? " " : str) << " | ";

      os << endl;
    }
    os << bar << endl;
    
    os << "Failures:" << endl;
    for( const auto &m : g.failures_ )
      os << "(" << m.first.first << "," << m.first.second << "):" << m.second << endl;

    return os;
  }

}
