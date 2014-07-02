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
#include <set>

#include "domain.hpp"
#include "../variables/action.hpp"

using namespace std;

namespace ghost
{
  class BuildOrderDomain : public Domain<Action>
  {
  public:
    BuildOrderDomain( int, int, const vector<Action>* );
    BuildOrderDomain( int, int, const vector<Action>*, int );
    BuildOrderDomain( int, int, const vector<Action>*, double );

    inline set<int>	getMonteCarloSample() const { return sample; }
    
    vector<int>	possibleFrames( const Action& ) const;

    void makeMonteCarloSample( int numberSamples );
    void makeMonteCarloSample( double ratio );
    void makeMonteCarloSample( int numberSamples, int varId );
    void makeMonteCarloSample( double ratio, int varId );

    // friend ostream& operator<<( ostream&, const BuildOrderDomain& );

  private:
    int computeSize( const vector<Action>* );
    
    set<int> sample;
  };
}