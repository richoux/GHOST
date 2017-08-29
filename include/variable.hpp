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

#include <iostream>
#include <vector>

#include "domain.hpp"

using namespace std;

namespace ghost
{
  //! Variable is the class encoding the variables of your CSP/COP.
  /*! 
   * In GHOST, all variable objects must be instanciate from the same
   * concrete class. Be careful to model your CSP/COP in order to use one
   * kind of variable only, ie., all variable objects in the implementation 
   * of your CSP/COP must be instanciated from the same Variable (sub)class.
   *
   * To encode your CSP/COP variables, you can either directly use this
   * class Variable (there are no pure virtual functions here),
   * or inherit from it to make your own variable class.
   *
   * \sa Domain
   */
  class Variable
  {
    static int NBER_VAR;
    
    int _id; // Unique ID integer
    
    // The private Variable constructor
    Variable( const string& name,
	      const string& shortName,
	      const Domain& domain,
	      int index = 0);
    
    // For the copy-and-swap idiom
    void swap( Variable &other );

  protected:
    string	name;		//!< A string to give a full name to the variable (for instance, "Barracks").
    string	shortName;	//!< A string to give a shorten name to the variable (for instance, "B").
    Domain	domain;		//!< The domain of the variable.
    int		index;		//!< The domain's index corresponding to the current value of the variable.
    
  public:
    //! The default Variable constructor is disabled.
    Variable() = delete;

    //! First Variable constructor, with the vector of domain values and the outside-the-scope value.
    /*!
     * \param name A const reference of a string to give a full name to the variable (for instance, "Barracks").
     * \param shortName A const reference of a string to give a shorten name to the variable (for instance, "B").
     * \param domain A const reference to the vector of integers composing the domain to create.
     * \param index The domain's index corresponding to the variable initial value. Zero by default.
     */
    Variable( const string&		name,
	      const string&		shortName,
	      const vector<int>&	domain,
	      int			index = 0);
    
    //! Second Variable constructor, with a size and a starting value for the domain.
    /*!
     * \param name A const reference of a string to give a full name to the variable (for instance, "Barracks").
     * \param shortName A const reference of a string to give a shorten name to the variable (for instance, "B").
     * \param size The size of the domain to create.
     * \param startValue An integer representing the first value of the domain. The creating domain will then be the interval [startValue, startValue + size].
     * \param index The domain's index corresponding to the variable initial value. Zero by default.
     */
    Variable( const string&	name,
	      const string&	shortName,
	      int		size,
	      int		startValue,
	      int		index = 0);

    //! Variable copy constructor
    /*!
     * \param other A const reference to a Variable object.
     */
    Variable( const Variable &other );

    //! Variable's copy assignment operator
    /*!
     * The copy-and-swap idiom is applyed here.
     * 
     * \param other A Variable object.
     */
    Variable& operator=( Variable other );

    //! Default Variable destructor.
    virtual ~Variable() = default;

    //! Function initializing the variable to one random values of its domain.
    void do_random_initialization();
    
    /*! Function returning what values are in the domain.
     * \return a vector<int> of values belonging to the variable domain.
     */
    vector<int> possible_values() const;

    
    //! Inline function to get the current value of the variable.
    /*! 
     * If the index do not belong to the Domain range, raises an indexException.
     * \sa Domain
     */
    inline int get_value() const { return domain.get_value( index ); }

    
    //! Inline function to set the value of the variable.
    /*! 
     * If the given value is not in the domain, raises a valueException.
     * \param value An integer representing the new value to set.
     * \sa Domain
     */
    inline void	set_value( int value ) { index = domain.index_of( value ); }

    //! Inline function to get the variable name.
    inline string get_name() const { return name; }

    //! Inline function to get the variable short name.
    inline string get_short_name() const { return shortName; }

    //! Inline function to get the unique id of the Variable object.
    inline int get_id() const { return _id; }
    
    friend ostream& operator<<( ostream& os, const Variable& v )
    {
      return os
	<< "Variable name: " << v.name
	<< "\nShort name: " << v.shortName
	<< "\nValue: " <<  v.domain.get_value( v.index )
	<< "\n-------";
    }
  };
}
