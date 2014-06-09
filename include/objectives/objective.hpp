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

#include "../misc/random.hpp"

using namespace std;

namespace ghost
{
  template <typename TypeVariable, typename TypeDomain>
  class Objective
  {
  public:
    Objective<TypeVariable, TypeDomain>( const string &name ) : name(name) { }

    
    inline double cost( const vector< TypeVariable > &vecBuildings,
			const TypeDomain &domain ) const
      { return v_cost(vecBuildings, domain); }
    
    inline int heuristicVariable( const vector< int > &vecVariables,
				  const vector< TypeVariable > &vecBuildings,
				  const TypeDomain &domain )
      { return v_heuristicVariable(vecVariables, vecBuildings, domain); }
    
    inline void setHelper( const TypeVariable &variable,
			   const vector< TypeVariable > &vecBuildings,
			   const TypeDomain &domain )
      { v_setHelper(variable, vecBuildings, domain); }

    inline string getName() { return name; }

    int		heuristicValue( const vector< double >&, double&, int& ) const;
    void	initHelper( int );
    void	resetHelper();

  protected:
    virtual double	v_cost( const vector< TypeVariable > &vecBuildings,
				const TypeDomain& ) const = 0;

    virtual int		v_heuristicVariable( const vector< int > &vecVariables,
					     const vector< TypeVariable > &vecBuildings,
					     const TypeDomain& ) = 0;

    virtual void	v_setHelper( const TypeVariable &b,
				     const vector< TypeVariable > &vecBuildings,
				     const TypeDomain &domain ) = 0;

    Random randomVar;
    string name;
    vector<double> heuristicValueHelper; 
  };
}
