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
// #include <memory>

#include "../../include/variables/action.hpp"
#include "../../include/misc/buildorderProtoss.hpp"
// #include "../constraints/buildorderConstraint.hpp"
// #include "../domains/buildorderDomain.hpp"

using namespace std;

namespace ghost
{
  Action factoryProtossAction( const string &fullName )
  {
    if( fullName.compare("Protoss_Zealot") == 0 )
      return Action( 40, 100, 0, 2, unit, vector<string>{"Protoss_Gateway"}, "Protoss_Gateway", Protoss, "puz", "Protoss_Zealot" );
    else
      if( fullName.compare("Protoss_Dragoon") == 0 )
	return Action( 50, 125, 50, 2, unit, vector<string>{"Protoss_Gateway", "Protoss_Cybernetics_Core", "Protoss_Assimilator"}, "Protoss_Gateway", Protoss, "pud", "Protoss_Dragoon" );
      else
	if( fullName.compare("Protoss_Gateway") == 0 )
	  return Action( 60, 150, 0, 0, building, vector<string>{"Protoss_Nexus"}, "Protoss_Probe", Protoss, "pbg", "Protoss_Gateway" );
	else
	  if( fullName.compare("Protoss_Cybernetics_Core") == 0 )
	    return Action( 60, 200, 0, 0, building, vector<string>{"Protoss_Gateway"}, "Protoss_Probe", Protoss, "pbc", "Protoss_Cybernetics_Core" );
	  else
	    if( fullName.compare("Protoss_Forge") == 0 )
	      return Action( 40, 150, 0, 0, building, vector<string>{"Protoss_Nexus"}, "Protoss_Probe", Protoss, "pbf", "Protoss_Forge" );
	    else
	      if( fullName.compare("Protoss_Ground_Weapons_1") == 0 )
		return Action( 266, 100, 100, 0, upgrade, vector<string>{"Protoss_Forge", "Protoss_Assimilator"}, "Protoss_Forge", Protoss, "ppgw1", "Protoss_Ground_Weapons_1" );
	      else
		if( fullName.compare("Protoss_Pylon") == 0 )
		  return Action( 30, 100, 0, 0, building, vector<string>(), "Protoss_Probe", Protoss, "pby", "Protoss_Pylon" );
		else
		  if( fullName.compare("Protoss_Assimilator") == 0 )
		    return Action( 40, 100, 0, 0, building, vector<string>(), "Protoss_Probe", Protoss, "pba", "Protoss_Assimilator" );
		  else
		  {
		    cout << "WHY? " << fullName << endl;
		    return Action();
		  }
  }

  
  // vector< shared_ptr<BuildOrderConstraint> > makeProtossBuildOrderConstraints( const vector< Action > *vec, const BuildOrderDomain *domain )
  // {
  //   shared_ptr<BuildOrderConstraint> dependencies = make_shared<BuildOrderConstraint>( vec, domain );

  //   vector< shared_ptr<BuildOrderConstraint> > vecConstraints { dependencies };
  //   return vecConstraints;
  // }
}
