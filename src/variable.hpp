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
#include <memory>
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
   */
  class Variable
  {
  private:
    double _projectedCost; //!< The cost of the variable. This is for inner mecanisms, no need to worry about that.  

    //! The private Variable constructor
    /*!
     * \param name A string to give a full name to the variable (for instance, "Barracks").
     * \param shortName A string to give a shorten name to the variable (for instance, "B").
     * \param domain A unique smart pointer to a Domain object.
     * \param index The domain's index corresponding to the variable initial value.
     * \sa Domain
     */
    Variable( string name, string shortName, unique_ptr<Domain> domain, int index )
      : name(name),
	shortName(shortName),
	domain(std::move( domain )),
	index(index)
    { }

  protected:
    string		name;		//!< A string to give a full name to the variable (for instance, "Barracks").
    string		shortName;	//!< A string to give a shorten name to the variable (for instance, "B").
    unique_ptr<Domain>	domain;		//!< A unique smart pointer on the variable domain.
    int			index;		//!< The domain's index corresponding to the current value of the variable.
    
    //! Inline function used for the copy-and-swap idiom.
    /*!
     * \param other A reference to a Variable object.
     */
    inline void swap( Variable &other )
    {
      std::swap(this->name, other.name);
      std::swap(this->shortName, other.shortName);
      // std::swap(this->domain, other.domain);
      domain = std::move( other.domain );
      std::swap(this->index, other.index);
      std::swap(this->_projectedCost, other._projectedCost);
    }  
    
  public:
    //! Short Variable constructor initializing the name and short name only.
    Variable( string name, string shortName )
      : Variable( name, shortName, nullptr, -1 )
    { }

    //! First Variable constructor, with the vector of domain values and the outside-the-scope value.
    /*!
     * \param name A string to give a full name to the variable (for instance, "Barracks").
     * \param shortName A string to give a shorten name to the variable (for instance, "B").
     * \param index The domain's index corresponding to the variable initial value.
     * \param domain A vector of integers composing the domain to create.
     * \param outsideScope An integer representing all values outside the domain scope (-1 by default).
     * \sa Domain
     */
    Variable( string name, string shortName, int index, vector<int> domain, int outsideScope = -1 )
      : Variable( name, shortName, make_unique<Domain>( domain, outsideScope ), index )
    { }
    
    //! Second Variable constructor, with a size and a starting value for the domain.
    /*!
     * \param name A string to give a full name to the variable (for instance, "Barracks").
     * \param shortName A string to give a shorten name to the variable (for instance, "B").
     * \param index The domain's index corresponding to the variable initial value.
     * \param size The size of the domain to create.
     * \param startValue An integer representing the first value of the domain. The creating domain will then be the interval [startValue, startValue + size], with startValue-1 corresponding to the outside-the-scope value.
     * \sa Domain
     */
    Variable( string name, string shortName, int index, int size, int startValue )
      : Variable( name, shortName, make_unique<Domain>( size, startValue ), index )
    { }

    virtual ~Variable()
    {
      domain.release();
    }

    //! Variable copy constructor
    /*!
     * \param other A reference to a Variable object.
     */
    Variable( const Variable &other )
      : name(other.name),
	shortName(other.shortName),
	domain(make_unique<Domain>(other.domain.get())),
	index(other.index)
    { }

    //! Variable's copy assignment operator
    /*!
     * The copy-and-swap idiom is applyed here.
     * 
     * \param other A Variable object.
     */
    Variable& operator=( Variable other )
    {
      this->swap( other );
      return *this;
    }

    //! Inline function to know if the domain has been initialized.
    /*!
     * \return True if and only if the domain variable is initialized.
     */
    inline bool hasInitializedDomain()
    {
      if( !domain )
	return false;
      else
	return domain->isInitialized();
    }
    
    // //! Inline function reseting the domain with its initial values.
    // /*!
    //  * \sa domain::resetToInitial()
    //  */
    // inline void resetDomain() { domain->resetToInitial(); }
    
    //! Shifting to the next domain value.
    /*!
     * Set the current value to the next value in the domain, 
     * or to the first one if we reach the domain upper bound.
     */
    void shiftValue()
    {
      if( index >= 0 )
	index = index < domain->getSize() - 1 ? index + 1 : 0;
    }

    //! Shifting to the previous domain value.
    /*!
     * Set the current value to the previous value in the domain, 
     * or to the last one if we reach the domain lower bound.
     */
    void unshiftValue()
    {
      if( index >= 0 )
	index = index > 0 ? index - 1 : domain->getSize() - 1;
    }

    //! Inline function to get the current value of the variable.
    inline int getValue() const	{ return domain->getValue( index ); }
    
    //! Inline function to set the value of the variable.
    /*! 
     * If the given value is not in the variable domain, then the variable value is set to the outsideScope value of the domain.
     * \param value An integer representing the new value to set.
     */
    inline void	setValue( int value ) { index = domain->indexOf( value ); }

    /*! Function returning what values are in the current domain.
     * \return a vector<int> of values belonging to the variable domain.
     */
    vector< int > possibleValues()
    {
      vector< int > possibleValues;

      for( int i = 0 ; i < domain->getSize() ; ++i )
	possibleValues.push_back( domain->getValue( i ) );

      return possibleValues;
    }    
    
    //! Inline function to get the variable name.
    inline string getName() const { return name; }

    //! Inline function to get the variable short name.
    inline string getShortName() const { return shortName; }

    //! friend override of operator<<
    friend std::ostream& operator<<( std::ostream& os, const Variable& v )
    {
      return os
	<< "Variable name: " << v.name
	<< "\nShort name: " << v.shortName
	<< "\nValue: " <<  v.domain->getValue( v.index )
	<< "\n-------";
    }
  };
}
