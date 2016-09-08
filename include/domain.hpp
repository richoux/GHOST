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
#include <typeinfo>
#include <algorithm>
#include <numeric>
#include <iterator>
#include <exception>

#include "misc/random.hpp"

using namespace std;

namespace ghost
{
  //! Domain is the class encoding the domain of your CSP/COP.
  /*! 
   * Domain is the class implementing variables' domains, ie, the set of possible values a variable can take.
   * In GHOST, such values must be integers.
   * 
   * A domain contains:
   * 1. the container of current possible values of the variable it belongs to, 
   * 2. the container of initial values (if one wants to reset the domain, since values in the current domain may change),
   * 3. an integer representing values outside the domain scope
   * 4. finally, a pseudo-random number generator.
   */
  class Domain
  {
    vector< int > currentDomain;	//!< Vector of integers containing the current values of the domain.
    vector< int > initialDomain;	//!< Vector of integers containing the initial values of the domain.
    int outsideScope;			//!< Value representing all values outside the scope of the domain
    Random random;			//!< A random generator used by the function randomValue. 
    
    virtual bool v_isInitialized() const
    {
      return !currentDomain.empty();
    }
    
    virtual void v_resetToInitial()
    {
      currentDomain.resize( initialDomain.size() );
      std::copy( begin( initialDomain ), end( initialDomain ), begin( currentDomain ) );
    }
    
    virtual bool v_removeValue( int value )
    {
      int index = indexOf( value );
      if( index == -1 )
	return false;
      else
      {
	currentDomain.erase( begin( currentDomain ) + index );
	return true;
      }
      
      // auto& it = std::find( begin( currentDomain ), end( currentDomain ), value );
      // if( it == end( currentDomain ) )
      // 	return false;
      // else
      // {
      // 	currentDomain.erase( it );
      // 	return true;
      // }
    }
    
    virtual int v_randomValue()
    {
      return currentDomain[ random.getRandNum( currentDomain.size() ) ];
    }
    
    virtual size_t v_getSize() const
    {
      return currentDomain.size();
    }
    
    virtual size_t v_getInitialSize() const
    {
      return initialDomain.size();
    }
    
    virtual int v_maxValue() const
    {
      return *std::max_element( begin( currentDomain ), end( currentDomain ) );
    }
    
    virtual int v_minValue() const
    {
      return *std::min_element( begin( currentDomain ), end( currentDomain ) );
    }
    
    virtual int v_maxInitialValue() const
    {
      return *std::max_element( begin( initialDomain ), end( initialDomain ) );
    }
    
    virtual int v_minInitialValue() const
    {
      return *std::min_element( begin( initialDomain ), end( initialDomain ) );
    }
    
    virtual int v_getValue( int index ) const
    {
      if( index >=0 && index < currentDomain.size() )
	return currentDomain[ index ];
      else
	return outsideScope;
    }
    
    virtual int v_indexOf( int value ) const
    {
      auto it = std::find( begin( currentDomain ), end( currentDomain ), value );
      if( it == end( currentDomain ) )
	return -1;
      else
	return it - begin( currentDomain );
    }
    
  public:
    //! Domain constructor.
    /*!
     * Basic constructor taking the outside-the-scope value (-1 by default).
     */
    Domain( int outsideScope = -1 )
      : outsideScope(outsideScope)
    { }
    
    //! Domain constructor.
    /*!
     * Constructor taking a vector of integer values the outside-the-scope value (-1 by default), to 
     * initialize both the initial and current possible variable values. The outside-the-scope value
     * must not belong to this list, or an exception is raised (throw 0).
     */
    Domain( const vector< int > &domain, int outsideScope = -1 )
      : currentDomain(domain),
	initialDomain(domain),
	outsideScope(outsideScope)
    {
      if( std::find( begin( domain ), end( domain ), outsideScope ) != end( domain ) )
	throw 0;
    }

    //! Domain constructor.
    /*!
     * Constructor taking the domain size N and a starting value x, and creating a domain
     * with all values in [x, x + N]. The outside-the-scope value is set to x-1.
     */
    Domain( int size, int startValue )
      : currentDomain(vector<int>(size)),
	initialDomain(vector<int>(size)),
	outsideScope(startValue-1)
    {
      std::iota( begin( currentDomain ), end( currentDomain ), startValue );
      std::iota( begin( initialDomain ), end( initialDomain ), startValue );
    }

    //! Inline function following the NVI idiom, calling v_isInitialized.
    /*!
     * Used to know if the Domain object is just an empty shell or a properly 
     * initialized domain. In some cases, it can be convenient to instanciate 
     * a domain object first and to fill it up with values latter.
     * \return True if and only if the domain has been initialized (i.e., the current domain is not empty).
     */
    inline bool isInitialized() const { return v_isInitialized(); }

    //! Inline function following the NVI idiom, calling v_resetToInitial.
    /*!
     * Resets the set of current values to the set of initial values. 
     * Allow the recover all values in the domain if we filtered some of them. 
     */
    inline void resetToInitial() { v_resetToInitial(); }

    //! Inline function following the NVI idiom, calling v_removeValue.
    /*!
     * Deletes a given value from the set of current domain values.
     * \param value is the value to remove from the domain
     * \return True if the value has been removed. False if the value to remove was not in the current domain.
     */
    inline bool removeValue( int value ) { return v_removeValue( value ); }

    //! Inline function following the NVI idiom, calling v_randomValue.
    /*!
     * Returns a random value from the domain.
     */
    inline int randomValue() { return v_randomValue(); }

    //! Inline function following the NVI idiom, calling v_getSize.
    /*!
     * Get the number of values currently contained by the domain.
     * \return A size_t corresponding to the size of currentDomain.
     */
    inline size_t getSize() const { return v_getSize(); }

    //! Inline function following the NVI idiom, calling v_getInitialSize.
    /*!
     * Get the number of values initially contained by the domain.
     * \return A size_t corresponding to the size of initialDomain.
     */    
    inline size_t getInitialSize() const { return v_getInitialSize(); }

    //! Inline function following the NVI idiom, calling v_maxValue.
    /*!
     * Get the highest value in the current domain. 
     */
    inline int maxValue() const { return v_maxValue(); }

    //! Inline function following the NVI idiom, calling v_minValue.
    /*!
     * Get the lowest value in the current domain. 
     */
    inline int minValue() const { return v_minValue(); }

    //! Inline function following the NVI idiom, calling v_maxInitialValue.
    /*!
     * Get the highest value in the initial domain. 
     */
    inline int maxInitialValue() const { return v_maxInitialValue(); }

    //! Inline function following the NVI idiom, calling v_minInitialValue.
    /*!
     * Get the lowest value in the initial domain. 
     */
    inline int minInitialValue() const { return v_minInitialValue(); }

    //! Inline function following the NVI idiom, calling v_getValue.
    /*!
     * Get the value at the given index
     * \param index is the index of the desired value.
     * \return The value at the given index if this one is in the range of the domain, otherwise the outside-the-scope value.
     */    
    inline int getValue( int index ) const { return v_getValue( index ); }

    //! Inline function following the NVI idiom, calling v_indexOf.
    /*!
     * Get the index of a given value.
     * \return If the given value is in the domain, it returns its index, and -1 otherwise.
     */ 
    inline int indexOf( int value ) const { return v_indexOf( value ); }

    //! Inline function returning the outside-scope value.
    /*!
     * Returns the value outsideScope.
     */ 
    inline int getOutsideScope() const { return outsideScope; }
    
    //! friend override of operator<<
    /*!
     * Prints on the standard output the current domain size and content.
     */ 
    friend ostream& operator<<( ostream& os, const Domain& domain )
    {
      os << "Size: " <<  domain.getSize() << "\nCurrent domain: ";
      std::copy( begin( domain.currentDomain ), end( domain.currentDomain ), std::ostream_iterator<int>( os, " " ) );
      return os << "\n";
    }
  };
}
