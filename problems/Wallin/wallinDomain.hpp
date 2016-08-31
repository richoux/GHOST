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


#pragma once

#include <vector>
#include <map>
#include <set>
#include <string>
#include <iostream>

#include "../../src/domain.hpp"
#include "../../src/misc/random.hpp"
#include "building.hpp"

using namespace std;

namespace ghost
{
  using mapFail = map<pair<int, int>, string>;

  class WallinDomain : public Domain<Building>
  {
  public:
    // To counter method hiding
    using Domain<Building>::add;
    using Domain<Building>::clear;

    WallinDomain( int, int, int, int, int, int, int ) ;
    WallinDomain( int,
		  int,
		  const vector< pair<int, int> >&,
		  const vector< Building >*,
		  int,
		  int,
		  int,
		  int ) ;

    pair<int, int>	shift( Building& );
    void		quickShift( Building& );
    void		swap( Building&, Building& );	  
    void		add( const Building& );
    void		clear( const Building& );
    
    set< Building > getBuildingsAround( const Building &, const vector< Building >* )	const;
    set< Building > getBuildingsAbove( const Building &, const vector< Building >* )	const;
    set< Building > getBuildingsOnRight( const Building &, const vector< Building >* )	const;
    set< Building > getBuildingsBelow( const Building &, const vector< Building >* )	const;
    set< Building > getBuildingsOnLeft( const Building &, const vector< Building >* )	const;

    inline int		 distanceTo( int source, int target )	const { return distanceTo( source, lin2mat( target ) ); }
    inline int		 distanceToTarget( int source )		const { return distanceTo( source, targetTile ); }
	   int		 distanceTo( int, pair<int, int> )	const;
    
    inline void		 unbuildable( int row, int col )		{ matrixType_[row][col].assign(3, '#'); }
	   void		 unbuildable( vector< pair<int, int> > );
    
    inline set<int>	 buildingsAt( int row, int col )	const { return matrixId_[row][col]; }
    inline set<int>	 buildingsAt( pair<int, int> p )	const { return buildingsAt(p.first, p.second); }
    inline set<int>	 buildingsAt( int p )			const { return buildingsAt( lin2mat( p ) ); }

    inline pair<int, int> getStartingTile()	const { return startingTile; }
    inline pair<int, int> getTargetTile()	const { return targetTile; }
           
    inline int		 getNberRows()	const { return nRow_; }
    inline int		 getNberCols()	const { return mCol_; }
    inline bool		 hasFailure()	const { return !failures_.empty(); }
    inline mapFail	 failures()	const { return failures_; }

    inline pair<int, int> lin2mat( int p )	      const {return make_pair(p / mCol_, p % mCol_);}
    inline int		  mat2lin( int row, int col ) const {return row * mCol_ + col;}
    inline int		  mat2lin( pair<int, int> p ) const {return p.first * mCol_ + p.second;}

    bool	isStartingOrTargetTile( int ) const;
    bool	isNeightborOfSTTBuildings( const Building &, vector< Building > ) const;
    int		countAround( const Building &, const vector< Building >* ) const;  
    vector<int>	possiblePos( const Building& ) const;
    
    friend ostream& operator<<( ostream&, const WallinDomain& );

  private:
    void add(int, int, string, int);
    void clear(int, int, string, int);

    void v_restart( vector<Building> *variables );
    void v_wipe( vector<Building> *variables );
    void v_rebuild( vector<Building> *variables );
    
    int mCol_;
    int nRow_;
    vector< vector<string> > matrixType_;
    vector< vector< set<int> > > matrixId_;
    pair<int, int> startingTile;
    pair<int, int> targetTile;
    mapFail failures_;
  };
}
