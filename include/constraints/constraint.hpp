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
#include <iostream>
#include <memory>
#include <typeinfo>

#include "../objectives/objective.hpp"

using namespace std;

namespace ghost
{
  //! Constraint is the class encoding constraints of your CSP/COP.
  /*! 
   * In GHOST, many different constraint objects can be instanciate.
   *
   * The Constraint class is a template class, waiting for both the
   * type of variable and the type of domain. Thus, you must
   * instanciate a constraint by specifying the class of your variable
   * objects and the class of your domain object, like for instance
   * Constraint<Variable, Domain> or Constraint<MyCustomVariable,
   * MyCustomDomain>, if MyCustomVariable inherits from the
   * ghost::Variable class and MyCustomDomain inherits from the
   * ghost::Domain class.
   *
   * You cannot directly use this class Constraint to encode your CSP/COP
   * constraints, since this is an abstract class (see the list of
   * pure virtual functions below). Thus, you must write your own
   * constraint class inheriting from ghost::Constraint.
   *
   * Pure virtual Constraint functions:
   * - cost
   * - simulateCost
   *
   * \sa Variable, Domain
   */
  template <typename TypeVariable, typename TypeDomain>
  class Constraint
  {
  public:
    //! The unique Constraint constructor
    /*!
     * \param variables A constant pointer toward the vector of variable objects of the CSP/COP.
     * \param domain A constant pointer toward the domain object of the CSP/COP.
     */
    Constraint( const vector< TypeVariable > *variables, const TypeDomain *domain )
      : variables( const_cast< vector< TypeVariable >* >(variables) ), domain( const_cast<TypeDomain*>(domain) ) { }

    //! Pure virtual function to compute the current cost of the constraint. 
    /*!
     * In cost, the parameter varCost is not given to be used by the
     * function, but to store into varCost the projected cost of each
     * variable. This must be computed INSIDE the cost function.
     *
     * \param varCost A reference to a vector of double in order to store the projected cost of each variable.
     * \return A double representing the cost of the constraint on the current configuration.
     * \sa simulateCost
     */    
    virtual double cost( vector<double> &varCost ) const = 0;

    //! Pure virtual function to simulate the cost of the constraint on all possible values of the given variable. 
    /*!
     * In cost, the parameter vecVarSimCosts is not given to be used
     * by the function, but to store into vecVarSimCosts the projected
     * cost of currentVar on all possible values. This must be
     * computed INSIDE the simulateCost function.
     *
     * \param currentVar A reference to the variable we want to change the current value.
     * \param possibleValues A reference to a constant vector of the possible values for currentVar.
     * \param vecVarSimCosts A reference to the vector of vector of double in order to store the projected cost of currentVar on all possible values. 
     * \return The vector of the cost of the constraint for each possible value of currentVar.
     * \sa cost
     */    
    virtual vector<double> simulateCost( TypeVariable &currentVar,
					 const vector<int> &possibleValues,
					 vector< vector<double> > &vecVarSimCosts ) = 0;

    
    //! friend override of operator<<
    friend ostream& operator<<( ostream& os, const Constraint<TypeVariable, TypeDomain>& c )
    {
      return os << "Constraint type: " <<  typeid(c).name() << std::endl;
    }
    
  protected:
    vector< TypeVariable > *variables;	//!< A pointer to the vector of variable objects of the CSP/COP.
    TypeDomain *domain;			//!< A pointer to the domain object of the CSP/COP.
  };  
}
