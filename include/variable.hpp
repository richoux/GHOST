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

#include <iostream>
#include <vector>
#include <string>

#include "domain.hpp"

using namespace std;

namespace ghost
{
  //! This class encodes variables of your CSP/COP. You cannot inherits your own class from Variable.
  /*! 
   * In GHOST, all variables are discrete variables with a Domain containing intergers only 
   * (positive, negative or both). Since you cannot inherits from Variable, if your constraints 
   * or your objective functions need specific details about your variables (for instance, each variable models 
   * an agent with 2D coordinates), you must store these data on your own containers side by side with 
   * the variables vector (see Constraint and Objective).
   *
   * While modeling your probem with GHOST, make sure you understand the difference between your variable values 
   * (stored in the variable's domain) and your variable additional data (such as 2D coordinates for instance). 
   * You must manage additional data with your own data structures or classes.
   *
   * \sa Domain Constraint Objective
   */
  class Variable final
  {
    friend class Solver;
    static int	NBER_VAR;	//!< Static counter that increases each time one instanciates a Variable object.
    int		_id;		//!< Unique ID integer taking the current value of NBER_VAR

    string	_name;		//!< A string to give a full name to the variable (for instance, "Barracks").
    string	_shortName;	//!< A string to give a shorten name to the variable (for instance, "B").
    Domain	_domain;	//!< The domain of the variable.
    int		_index;		//!< The domain's index corresponding to the current value of the variable.
    int		_cache_value;	//!< Cache of the Variable current value.
    
    //! Regurlar private Variable constructor
    Variable( const string& name,
	      const string& shortName,
	      const Domain& domain,
	      int index = 0 );

    //! Default private constructor
    Variable();
    
    //! For the copy-and-swap idiom
    void swap( Variable &other );

  public:
    
    // //! The default Variable constructor is disabled.
    // Variable() = delete;

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
	      int			index = 0 );
    
    //! Second Variable constructor, with a starting value and a size for the domain.
    /*!
     * \param name A const reference of a string to give a full name to the variable (for instance, "Barracks").
     * \param shortName A const reference of a string to give a shorten name to the variable (for instance, "B").
     * \param startValue An integer representing the first value of the domain. The creating domain will then be the interval [startValue, startValue + size].
     * \param size A size_t corresponding to the size of the domain to create.
     * \param index The domain's index corresponding to the variable initial value. Zero by default.
     */
    Variable( const string&	name,
	      const string&	shortName,
	      int		startValue,
	      size_t		size,
	      int		index = 0 );

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
    ~Variable() = default;

    //! Inline function initializing the variable to one random values of its domain.
	  inline void pick_random_value() { set_value( _domain.random_value() ); }
    
    /*! Inline function returning what values are in the domain.
     * \return a vector<int> of values belonging to the variable domain.
     */
	  inline const vector<int>& possible_values() const { return _domain.get_domain(); }
    
    //! Inline function to get the current value of the variable.
    /*! 
     * \return An integer corresponding to the variable value. If the variable index does not belong to its domain range, an indexException is raised.
     * \sa Domain
     */
    // inline int get_value() const { return _domain.get_value( _index ); }
    inline int get_value() const { return _cache_value; }

    //! Inline function to set the value of the variable.
    /*! 
     * If the given value is not in the domain, raises a valueException.
     * \param value An integer representing the new value to set.
     * \sa Domain
     */
    inline void	set_value( int value )
    {
      _index = _domain.index_of( value );
      _cache_value = value;
    }

    //! Inline function returning the size of the domain of the variable.
    /*! 
     * \return a size_t equals to size of the domain of the variable.
     * \sa Domain
     */
    inline size_t get_domain_size() const { return _domain.get_size(); }

	  //! Inline function returning the minimal value in the variable's domain.
    /*! 
     * \return the minimal value in the variable's domain.
     * \sa Domain
     */
    inline int get_domain_min_value() const { return _domain.get_min_value(); }

	  //! Inline function returning the maximal value in the variable's domain.
    /*! 
     * \return the maximal value in the variable's domain.
     * \sa Domain
     */
    inline int get_domain_max_value() const { return _domain.get_max_value(); }

    //! Inline function to get the variable name.
    inline string get_name() const { return _name; }

    //! Inline function to get the variable short name.
    inline string get_short_name() const { return _shortName; }

    //! Inline function to get the unique id of the Variable object.
    inline int get_id() const { return _id; }

    //! To have a nicer stream of Variable.
    friend ostream& operator<<( ostream& os, const Variable& v )
    {
      return os
	<< "Variable name: " << v._name
	<< "\nShort name: " << v._shortName
	<< "\nValue: " <<  v._domain.get_value( v._index )
	<< "\n-------";
    }
  };
}
