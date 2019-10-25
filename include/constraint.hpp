/*
 * GHOST (General meta-Heuristic Optimization Solving Tool) is a C++ library 
 * designed to help developers to model and implement optimization problem 
 * solving. It contains a meta-heuristic solver aiming to solve any kind of 
 * combinatorial and optimization real-time problems represented by a CSP/COP. 
 *
 * GHOST has been first developped to help making AI for the RTS game
 * StarCraft: Brood war, but can be used for any kind of applications where 
 * solving combinatorial and optimization problems within some tenth of 
 * milliseconds is needed. It is a generalization of the Wall-in project.
 * Please visit https://github.com/richoux/GHOST for further information.
 * 
 * Copyright (C) 2014-2019 Florian Richoux
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
#include <typeinfo>
#include <functional>
#include <cmath> // for isnan
#include <exception>
#include <string>

#include "variable.hpp"

using namespace std;

namespace ghost
{
  //! This class encodes constraints of your CSP/COP.
  /*! 
   * You cannot directly use this class Constraint to encode your CSP/COP
   * constraints, since this is an abstract class. To model a problem with GHOST, 
   * you have to make your own constraints by inheriting from this class. 
   * The same problem can be composed of several subclasses of Constraint.
   *
   * The only pure virtual Constraint function is required_cost.
   *
   * \sa Variable
   */

  class Constraint
  {
    static int NBER_CTR; //!< Static counter that increases each time one instanciates a Constraint object.

	  struct nanException : std::exception
    {
	    const vector< reference_wrapper<Variable> >&	variables;
	    string message;

	    nanException( const vector< reference_wrapper<Variable> >&	variables ) : variables(variables)
	    {
		    message = "Constraint required_cost returned a NaN value on variables (";
		    for( int i = 0; i < (int)variables.size() - 1; ++i )
			    message += to_string(variables[i].get().get_value()) + ", ";
		    message += to_string(variables[(int)variables.size() - 1].get().get_value()) + ")\n";
	    }
	    const char* what() const noexcept { return message.c_str(); }
    };

  protected:
    const vector< reference_wrapper<Variable> >&	variables;	//!< Const reference to the vector of variable references composing the CSP/COP.
    int							id;		//!< Unique ID integer

    //! Pure virtual function to compute the current cost of the constraint.
    /*!
     * This function is fundamental: it evalutes how much the current values of variables violate this contraint.
     * Let's consider the following example : consider the contraint (x = y).\n
     * If x = 42 and y = 42, then these values satify the contraint. The cost is then 0.\n
     * If x = 42 and y = 40, the constraint is not satified, but intuitively, we are closer to have a solution than with
     * x = 42 and y = 10,000. Thus the cost when y = 40 must be strictly lower than the cost when y = 10,000.\n
     * Thus, a good required_cost candidate for the contraint (x = y) could be the function |x-y|.
     * 
     * This function MUST returns a value greater than or equals to 0.
     *
     * \warning Do not implement side effect in this function. It is called by the solver 
     * to compute the constraint cost but also for some inner mechanisms (such as cost simulations).
     *
     * \return A positive double corresponding to the cost of the constraint with current variable values. 
     * Outputing 0 means current values are satisfying this constraint.
     * \sa cost
     */
    virtual double required_cost() const = 0;

  public:
    //! Unique constructor
    /*!
     * \param variables A const reference to a vector of variable references composing the constraint.
     */
    Constraint( const vector< reference_wrapper<Variable> >& variables );

    //! Default copy contructor.
    Constraint( const Constraint& other ) = default;
    //! Default move contructor.
    Constraint( Constraint&& other ) = default;
    
    //! Copy assignment operator disabled.
    Constraint& operator=( const Constraint& other ) = delete;
    //! Move assignment operator disabled.
    Constraint& operator=( Constraint&& other ) = delete;
    
    //! Default virtual destructor.
    virtual ~Constraint() = default;
    
    //! Inline function following the NVI idiom. Calling required_cost.
    /*!
     * @throw nanException
     * \sa required_cost
     */
    inline double cost() const
	  {
		  double value = required_cost();
		  if( std::isnan( value ) )
			  throw nanException( variables );
		  return value;
	  }

    //! Function to determine if the constraint contains a given variable. 
    /*!
     * Given a variable, returns if it composes the constraint.
     *
     * \param var A variable.
     * \return True iff the constraint contains var.
     */ 
    bool has_variable( const Variable& var ) const;

    //! Inline function to get the unique id of the Constraint object.
    inline int get_id() const { return id; }

    //! To have a nicer stream of Constraint.
    friend ostream& operator<<( ostream& os, const Constraint& c )
    {
      return os << "Constraint type: " <<  typeid(c).name() << endl;
    }
  };
}
