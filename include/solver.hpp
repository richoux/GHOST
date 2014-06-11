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
#include <set>
#include <memory>
#include <cmath>
#include <chrono>
#include <ctime>
#include <limits>
#include <algorithm>
#include <functional>
#include <cassert>
#include <typeinfo>

#include "variables/variable.hpp"
#include "constraints/constraint.hpp"
#include "domains/domain.hpp"
#include "misc/tools.hpp"
#include "misc/random.hpp"
#include "objectives/objective.hpp"

using namespace std;

namespace ghost
{
  template <typename TypeVariable, typename TypeDomain, typename TypeConstraint>
  class Solver
  {
  public:
    Solver( const vector< shared_ptr<TypeConstraint> > &vecConstraints, 
	    const vector< TypeVariable > &vecVariables, 
	    const TypeDomain &domain,
	    const string &obj )
      : Solver(vecConstraints, vecVariables, domain, 0, obj){  }

    Solver( const vector< shared_ptr<TypeConstraint> > &vecConstraints, 
	    const vector< TypeVariable > &vecVariables, 
	    const TypeDomain &domain,
	    const int loops,
	    const string &obj )
      : vecConstraints(vecConstraints), 
	vecVariables(vecVariables), 
	variableCost( vecVariables.size() ),
	domain(domain),
	loops(loops),
	tabuList( vecVariables.size() ),
	factory(FactoryObj()),
	objective(factory.makeObjective( obj )),
	bestSolution(vecVariables.size())
      { 
	reset();
      }

    
    double solve( double );
    
  private:
    void reset();

    void move( TypeVariable& building, int newPosition )
      {
	domain.clear( building );
	building.setValue( newPosition );
	domain.add( building );
	updateConstraints( vecConstraints, domain );
      }

    set< TypeVariable > getNecessaryVariables() const
      {
	// find all buildings accessible from the starting building and remove all others
	int nberCurrent = *( domain->buildingsAt( domain->getStartingTile() ).begin() );
	TypeVariable current = vecVariables[ nberCurrent ];
	set< TypeVariable > toVisit = domain->getVariablesAround( *current, vecVariables );
	set< TypeVariable > visited;
	set< TypeVariable > neighbors;
    
	visited.insert( current );
    
	while( !toVisit.empty() )
	{
	  auto first = *( toVisit.begin() );
	  current = first;
	  toVisit.erase( first );
	  neighbors = domain.getVariablesAround( *current, vecVariables );

	  visited.insert( current );
      
	  for( auto n : neighbors )
	    if( visited.find( n ) == visited.end() )
	      toVisit.insert( n );
	}

	return visited;
      }

    vector< shared_ptr<TypeConstraint> >	vecConstraints;
    vector< TypeVariable >			vecVariables;
    vector<double>				variableCost;
    TypeDomain					domain;
    int						loops;
    vector<int>					tabuList;
    Random					randomVar;
    FactoryObj					factory;
    shared_ptr<Objective>			objective;
    double					bestCost;
    vector<int>					bestSolution;
    multimap<int, TypeVariable>			buildingSameSize;
  };
}
