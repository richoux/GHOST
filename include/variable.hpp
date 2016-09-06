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

#include <algorithm>
#include <iostream>
#include <typeinfo>

using namespace std;

namespace ghost
{
  //! Variable is the class encoding the variables of your CSP/COP.
  /*! 
   * In GHOST, all variable objects must be instanciate from the same
   * concrete class. Be careful to model your CSP/COP in order to use one
   * kind of variable only.
   *
   * To encode your CSP/COP variables, you can either directly use this
   * class Variable (there are no pure virtual functions here),
   * or inherit from it to make your own variable class.
   */
  class Variable
  {
  public:
    //! Empty Variable constructor by default, doing nothing.
    Variable() { }

    //! The regular Variable constructor
    /*!
     * When this constructor is called, the class variable
     * numberVariables is automatically incremented.
     * 
     * \param name A string to give a shorten name to the variable (for instance, "B").
     * \param fullName A string to give a full name to the variable (for instance, "Barracks").
     * \param value The initial value of the variable, -1 by default.
     * \sa numberVariables
     */
    Variable( string name, string fullName, int value = -1 )
      : name(name),
	fullName(fullName),
	id(Variable::numberVariables++),
	value(value)
    { }

    //! Variable's copy constructor, designed to NOT increment numberVariables
    /*!
     * \param other A reference to a Variable object.
     * \sa numberVariables
     */
    Variable( const Variable &other )
      : name(other.name),
	fullName(other.fullName),
	id(other.id),
	value(other.value)
    { }

    //! Variable's copy assignment operator, designed to NOT increment numberVariables
    /*!
     * The copy-and-swap idiom is applyed here.
     * 
     * \param other A Variable object.
     * \sa numberVariables
     */
    Variable& operator=( Variable other )
    {
      this->swap( other );
      return *this;
    }

    //! Inline function to compare (less-than operator) two Variable objects.
    //! In this class, operator< is implemented to compare two Variable objects regarding their id.
    inline bool		operator<( const Variable& other )	const	{ return id < other.id; }

    //! Inline function to shift the object value.
    //! In this class, shiftValue is implemented to increment the value (++value).
    inline void		shiftValue()					{ ++value; }

    //! Inline function to unshift the object value.
    //! In this class, unshiftValue is implemented to decrement the value (--value).
    inline void		unshiftValue()					{ --value; }

    //! Inline function to swap the value of two objects.
    /*! 
     * In this class, swapValue calls std::swap between this->value and other.value.
     *
     * \param other A reference to a Variable object.
     */
    inline void		swapValue( Variable &other )			{ std::swap(this->value, other.value); }

    //! Inline mutator to set the object's value.
    /*! 
     * In this class, setValue is a mere value = v
     *
     * \param v An integer representing the new value to set.
     */
    inline void		setValue( int v )			{ value = v; }

    //! Inline accessor to get the object's value.
    inline int		getValue()			const	{ return value; }

    //! Inline accessor to get the object's id.
    inline int		getId()				const	{ return id; }

    //! Inline accessor to get the object's name.
    inline string	getName()			const	{ return name; }

    //! Inline accessor to get the object's full name.
    inline string	getFullName()			const	{ return fullName; }

    //! friend override of operator<<
    friend std::ostream& operator<<( std::ostream& os, const Variable& v )
    {
      return os
	<< "Variable type: " <<  typeid(v).name() << std::endl
	<< "Name: " << v.name << std::endl
	<< "Full name: " << v.fullName << std::endl
	<< "Id num: " << v.id << std::endl
	<< "Value: " <<  v.value << std::endl
	<< "-------" << std::endl;
    }
    
  protected:
    //! Inline function used for the copy-and-swap idiom.
    /*!
     * \param other A reference to a Variable object.
     */
    inline void swap( Variable &other )
    {
      std::swap(this->name, other.name);
      std::swap(this->fullName, other.fullName);
      std::swap(this->id, other.id);
      std::swap(this->value, other.value);
    }
  
    string	name;		//!< A string to give a shorten name to the variable (for instance, "B").
    string	fullName;	//!< A string to give a full name to the variable (for instance, "Barracks").
    int		id;		//!< An integer to stamp the object. Its value must be unique among all Variable objects.
    int		value;		//!< The value of the variable. Must be an integer (it can take negative values).
    
  private:
    static int numberVariables; //!< A static integer to make sure the object's id is unique. Incremented by calling the regular constructor.
  };
}
