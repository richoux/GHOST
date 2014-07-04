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


#include <numeric>
#include <iostream>
#include <typeinfo>

#include "../../domains/buildorderDomain.hpp"

using namespace std;

namespace ghost
{
  BuildOrderDomain::BuildOrderDomain( int numberVariables, const vector<Action> *variables )
    : Domain( computeSize( variables ), numberVariables, -1 )
  {
    for( const auto &v : *variables )
      domains[ v.getId() ] = possibleFrames( v );
  }
  
  BuildOrderDomain::BuildOrderDomain( int numberVariables, const vector<Action> *variables, int sizeSample )
    : BuildOrderDomain( numberVariables, variables )
  { makeMonteCarloSample( sizeSample ); }
  
  BuildOrderDomain::BuildOrderDomain( int numberVariables, const vector<Action> *variables, double ratioSample )
    : BuildOrderDomain( numberVariables, variables )
  { makeMonteCarloSample( ratioSample ); }

  vector<int> BuildOrderDomain::possibleFrames( const Action &action ) const
  {
    vector<int> vecFrames( size - action.getFrameRequired() );
    std::iota( begin( vecFrames ), end( vecFrames ), -1 );
    return vecFrames;
  }
  
  void BuildOrderDomain::makeMonteCarloSample( int numberSamples )
  {
    while( sample.size() < numberSamples )
      sample.insert( random.getRandNum( size ) - 1 );
  }
  
  void BuildOrderDomain::makeMonteCarloSample( double ratio )
  {
    while( sample.size() < ratioSample * size )
      sample.insert( random.getRandNum( size ) - 1 );    
  }

  void BuildOrderDomain::makeMonteCarloSample( int numberSamples, int varId )
  {
    while( sample.size() < numberSamples )
      sample.insert( random.getRandNum( domains.at( varId ).size() ) - 1 );    
  }
  
  void BuildOrderDomain::makeMonteCarloSample( double ratio, int varId )
  {
    while( sample.size() < ratioSample * size )
      sample.insert( random.getRandNum( domains.at( varId ).size() ) - 1 );        
  }

  int BuildOrderDomain::computeSize( const vector<Action> *variables )
  {
    int sum = 0;

    for( const auto &v : *variables )
      sum += v.getFrameRequired();

    return 2*sum;
  }

  // friend ostream& operator<<( ostream &os, const BuildOrderDomain &b )
  // {
  // }
}
