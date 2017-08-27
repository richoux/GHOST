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
 * Copyright (C) 2014-2017 Florian Richoux
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

#include "variable.hpp"

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
    static int NBER_CTR;

    // //! For the copy-and-swap idiom
    // void swap( Constraint &other );

  protected:
    vector< TypeVariable >	variables;	//!< The vector of variable pointers compositing the CSP/COP.
    int				id;		//!< Unique ID integer

    //! Pure virtual function to compute the current cost of the constraint.
    //! WARNING: do not implement side effect in this function. It will be called by the solver
    //! to compute the constraint cost but also for some cost simulations.
    virtual double required_cost() const = 0;

  public:
    Constraint() = default;
    
    //! The unique Constraint constructor
    /*!
     * \param variables The vector of variable pointers composition the CSP/COP.
     */
    Constraint( const vector< TypeVariable >& variables );

    //! Constraint copy constructor
    /*!
     * \param other A reference to a TypeVariable object.
     */
    Constraint( const Constraint<TypeVariable> &other );

    // //! Constraint's copy assignment operator
    // /*!
    //  * The copy-and-swap idiom is applyed here.
    //  * 
    //  * \param other A Constraint object.
    //  */
    // Constraint& operator=( Constraint other );
    
    //! Default Constraint destructor.
    virtual ~Constraint() = default;

    //! Inline function following the NVI idiom. Calling v_cost.
    //! \sa required_cost
    inline double cost() const { return required_cost(); }

    //! Given a variable, does this variable composes the constraint?
    //! \param var A variable.
    //! \return True iff the constraint contains var 
    bool has_variable( const TypeVariable& var ) const;

    //! Inline function to get the unique id of the Constraint object.
    inline int get_id() const { return id; }

    //! friend override of operator<<
    friend ostream& operator<<( ostream& os, const Constraint<TypeVariable>& c )
    {
      return os << "Constraint type: " <<  typeid(c).name() << endl;
    }
  };

  ////////////////////
  // Implementation //
  ////////////////////

  template <typename TypeVariable>
  int Constraint<TypeVariable>::NBER_CTR = 0;

  template <typename TypeVariable>
  Constraint<TypeVariable>::Constraint( const vector< TypeVariable >& variables )
    : variables	( variables ),
      id		( NBER_CTR++ )
  { }
  
  template <typename TypeVariable>
  Constraint<TypeVariable>::Constraint( const Constraint<TypeVariable> &other )
    : variables	( other.variables ),
      id		( other.id )
  { }
  
  // Constraint& Constraint::operator=( Constraint other )
  // {
  //   this->swap( other );
  //   return *this;
  // }
  
  // void Constraint::swap( Constraint &other )
  // {
  //   swap( this->variables, other.variables );
  //   swap( this->id, other.id );
  // }
  
  template <typename TypeVariable>
  bool Constraint<TypeVariable>::has_variable( const TypeVariable& var ) const
  {
    auto it = find_if( variables.cbegin(),
		       variables.cend(),
		       [&]( auto& v ){ return v.get_id() == var.get_id(); } );
    return it != variables.cend();
  }  
}
