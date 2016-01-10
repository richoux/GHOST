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


#pragma once

#include <vector>
#include <map>
#include <set>
#include <string>
#include <iostream>

#include "domain.hpp"
#include "../variables/building.hpp"
#include "../misc/random.hpp"

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

	WallinDomain(int maxX, int maxY, int nbVar, int sX, int sY, int tX, int tY);
	WallinDomain(int maxX, int maxY,
		const vector< pair<int, int> > &unbuildables, const vector< Building > *variables,
		int sX, int sY, int tX, int tY);

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

	inline int		 distanceTo(int source, int target)	const { return distanceTo(source, lin2mat(target)); }
	inline int		 distanceToTarget(int source)		const { return distanceTo(source, targetTile); }
		   int		 distanceTo(int, pair<int, int>)	const;
    
	inline void		 unbuildable(int x, int y)			{ matrixType_[x][y].assign(3, '#'); }
		   void		 unbuildable(vector< pair<int, int> >);
    
    inline set<int>	 buildingsAt( int x, int y )		const { return matrixId_[x][y]; }
    inline set<int>	 buildingsAt( pair<int, int> p )	const { return buildingsAt(p.first, p.second); }
    inline set<int>	 buildingsAt( int p )				const { return buildingsAt( lin2mat( p ) ); }

    inline pair<int, int> getStartingTile()	const { return startingTile; }
    inline pair<int, int> getTargetTile()	const { return targetTile; }
           
	inline int		 getNberRows()	const { return maxY_; }
	inline int		 getNberCols()	const { return maxX_; }
    inline bool		 hasFailure()	const { return !failures_.empty(); }
    inline mapFail	 failures()		const { return failures_; }

	inline pair<int, int> lin2mat( int p )				const { return make_pair(p % maxY_, p / maxY_); }
	inline int		      mat2lin( int x, int y )		const { return x + y * maxY_; }
	inline int		      mat2lin( pair<int, int> p )	const { return p.first + p.second * maxY_; }

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
    
    int maxX_;
    int maxY_;
    vector< vector<string> > matrixType_;
    vector< vector< set<int> > > matrixId_;
    pair<int, int> startingTile;
    pair<int, int> targetTile;
    mapFail failures_;
  };
}
