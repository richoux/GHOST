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
    
    int _id; //!< Unique ID integer
    
    //! The private Variable constructor
    /*!
     * \param name A string to give a full name to the variable (for instance, "Barracks").
     * \param shortName A string to give a shorten name to the variable (for instance, "B").
     * \param domain A shared pointer to a Domain object.
     * \param index The domain's index corresponding to the variable initial value.
     * \sa Domain
     */
    Variable( const string& name,
	      const string& shortName,
	      const Domain& domain,
	      int index );
    
    //! For the copy-and-swap idiom
    void swap( Variable &other );

  protected:
    string	name;		//!< A string to give a full name to the variable (for instance, "Barracks").
    string	shortName;	//!< A string to give a shorten name to the variable (for instance, "B").
    Domain	domain;		//!< A shared pointer on the variable domain.
    int		index;		//!< The domain's index corresponding to the current value of the variable.
    
  public:
    Variable() = delete;

    //! First Variable constructor, with the vector of domain values and the outside-the-scope value.
    /*!
     * \param name A string to give a full name to the variable (for instance, "Barracks").
     * \param shortName A string to give a shorten name to the variable (for instance, "B").
     * \param index The domain's index corresponding to the variable initial value.
     * \param domain A vector of integers composing the domain to create.
     * \param outsideScope An integer representing all values outside the domain scope (-1 by default).
     * \sa Domain
     */
    Variable( const string&	name,
	      const string&	shortName,
	      int		index,
	      vector<int>	domain,
	      int		outsideScope = -1 );
    
    //! Second Variable constructor, with a size and a starting value for the domain.
    /*!
     * \param name A string to give a full name to the variable (for instance, "Barracks").
     * \param shortName A string to give a shorten name to the variable (for instance, "B").
     * \param index The domain's index corresponding to the variable initial value.
     * \param size The size of the domain to create.
     * \param startValue An integer representing the first value of the domain. The creating domain will then be the interval [startValue, startValue + size], with startValue-1 corresponding to the outside-the-scope value.
     * \sa Domain
     */
    Variable( const string&	name,
	      const string&	shortName,
	      int		index,
	      int		size,
	      int		startValue );

    //! Variable copy constructor
    /*!
     * \param other A reference to a Variable object.
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
    
    //! Shifting to the next domain value.
    /*!
     * Set the current value to the next value in the domain, 
     * or to the first one if we reach the domain upper bound.
     */
    void shift_value();

    //! Shifting to the previous domain value.
    /*!
     * Set the current value to the previous value in the domain, 
     * or to the last one if we reach the domain lower bound.
     */
    void unshift_value();

    /*! Function returning what values are in the current domain.
     * \return a vector<int> of values belonging to the variable domain.
     */
    vector<int> possible_values() const;

    
    /////////////////////////
    // Bonne idée de récupérer outsideScope ici ? Ça demande de savoir comment est fait Domain.

    //! Inline function to get the current value of the variable.
    inline int get_value() const { return domain.get_value( index ); }

    
    /////////////////////////
    // Bonne idée de permettre l'affectation d'une valeur hors domaine ?
    
    //! Inline function to set the value of the variable.
    /*! 
     * If the given value is not in the variable domain, then the variable value is set to the outsideScope value of the domain.
     * \param value An integer representing the new value to set.
     */
    inline void	set_value( int value ) { index = domain.index_of( value ); }

    //! Function to know if the variable has been assigned to a value of its domain or not.
    /*! 
     * Returns true if the variable has been assigned to a value of its domain. 
     * Returns false otherwise.
     */
    bool is_assigned() const;

    //! Inline function to get the variable name.
    inline string get_name() const { return name; }

    //! Inline function to get the variable short name.
    inline string get_short_name() const { return shortName; }

    //! Inline function to get the unique id of the Variable object.
    inline int get_id() const { return _id; }
    
    //! friend override of operator<<
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
