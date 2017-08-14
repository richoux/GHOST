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
#include <iostream>
#include <memory>
#include <typeinfo>

#include "objective.hpp"

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
   * The only pure virtual Constraint function is v_cost
   *
   * \sa Variable
   */
  template <typename TypeVariable>
  class Constraint
  {
  public:
    //! The unique Constraint constructor
    /*!
     * \param variables A constant pointer toward the vector of variable objects of the CSP/COP.
     * \param domain A constant pointer toward the domain object of the CSP/COP.
     */
    Constraint( const vector< TypeVariable > *variables )
      : variables( const_cast< vector< TypeVariable >* >(variables) ) { }


    //! Inline function following the NVI idiom. Calling v_cost.
    //! \sa v_cost
    inline double cost( vector<double> &varCost ) const { return v_cost( varCost ); }

    // //! Inline function following the NVI idiom. Calling v_simulteCost.
    // //! \sa v_simulteCost
    // inline vector<double> simulateCost( TypeVariable &currentVar,
    // 					const vector<int> &possibleValues,
    // 					vector< vector<double> > &vecVarSimCosts,
    // 					shared_ptr< Objective< TypeVariable, TypeDomain > > objective )
    // { return v_simulateCost( currentVar, possibleValues, vecVarSimCosts, objective ); }

    // //! Inline function following the NVI idiom, with objective = nullptr. Calling v_simulteCost.
    // //! \sa v_simulteCost
    // inline vector<double> simulateCost( TypeVariable &currentVar,
    // 					const vector<int> &possibleValues,
    // 					vector< vector<double> > &vecVarSimCosts )
    // { return v_simulateCost( currentVar, possibleValues, vecVarSimCosts, nullptr ); }

    //! friend override of operator<<
    friend ostream& operator<<( ostream& os, const Constraint<TypeVariable, TypeDomain>& c )
    {
      return os << "Constraint type: " <<  typeid(c).name() << std::endl;
    }
    
  protected:
    //! Pure virtual function to compute the current cost of the constraint. 
    /*!
     * In cost, the parameter varCost is not given to be used by the
     * function, but to store into varCost the projected cost of each
     * variable. This must be computed INSIDE the cost function.
     *
     * \param varCost A reference to a vector of double in order to store the projected cost of each variable.
     * \return A double representing the cost of the constraint on the current configuration.
     * \sa cost v_simulateCost
     */    
    virtual double v_cost( vector<double> &varCost ) const = 0;

    // //! Pure virtual function to simulate the cost of the constraint on all possible values of the given variable. 
    // /*!
    //  * In v_simulatesCost, the parameter vecVarSimCosts is not given to be used
    //  * by the function, but to store into vecVarSimCosts the projected
    //  * cost of currentVar on all possible values. This must be
    //  * computed INSIDE the simulateCost function.
    //  *
    //  * \param currentVar A reference to the variable we want to change the current value.
    //  * \param possibleValues A reference to a constant vector of the possible values for currentVar.
    //  * \param vecVarSimCosts A reference to the vector of vector of double in order to store the projected cost of currentVar on all possible values.
    //  * \param objective The (plausibly null) current objective object. This parameter is necessary to update the heuristic value helper.
    //  * \return The vector of the cost of the constraint for each possible value of currentVar.
    //  * \sa simulateCost v_cost
    //  */    
    // virtual vector<double> v_simulateCost( TypeVariable &currentVar,
    // 					   const vector<int> &possibleValues,
    // 					   vector< vector<double> > &vecVarSimCosts,
    // 					   shared_ptr< Objective< TypeVariable, TypeDomain > > objective ) = 0;

    
    vector< TypeVariable > *variables;	//!< A pointer to the vector of variable objects of the CSP/COP.
  };  
}
