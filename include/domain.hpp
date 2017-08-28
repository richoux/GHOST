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
#include <iterator>
#include <exception>

#include "misc/random.hpp"

using namespace std;

namespace ghost
{
  //! Domain is the class encoding the domain of your CSP/COP.
  /*! 
   * Domain is the class implementing variable domains, ie, the set of possible values a variable can take.
   * In GHOST, such values must be integers.
   */
  class Domain
  {
    vector< int >	_domain;	// Vector of integers containing the current values of the domain.
    Random		_random;	// A random generator used by the function randomValue. 
    
    // For the copy-and-swap idiom
    void swap( Domain &other );

  public:
    //! First Domain constructor.
    /*!
     * Constructor taking a vector of integer values.
     *
     * \param domain A vector of int corresponding to the variable domain.
     */
    Domain( const vector< int >& domain );//, int outsideScope = -1 );

    //! Second Domain constructor.
    /*!
     * Constructor taking the domain of size 'size' and a starting value 'starValue', 
     * thus creating a domain with all integer values in [x, x + N].
     *
     * \param size An integer specifying the size of the domain.
     * \param startValue An integer specifying what is the first value in the domain.
     */
    Domain( int size, int startValue );

    //! Domain copy constructor.
    /*!
     * \param other A const reference to a Domain object.
     */
    Domain( const Domain &other );

    //! Domain's copy assignment operator.
    /*!
     * The copy-and-swap idiom is applyed here.
     * 
     * \param other A Domain object.
     */
    Domain& operator=( Domain other );

    //! Domain destructor.
    ~Domain() = default;
    
    //! Inline function returning a random value from the domain, following a uniform distribution.
    inline int random_value() const
    {
      return _domain[ _random.get_random_number( _domain.size() ) ];
    }

    //! Inline function to get the size of the domain.
    /*!
     * Get the number of values currently composing the domain.
     * \return A size_t corresponding to the size of the domain.
     */
    inline size_t get_size() const
    {
      return _domain.size();
    }

    //! Get the value at the given index
    /*!
     * \param index is the index of the desired value.
     * \return The value at the given index if this one is in the range of the domain, raises an indexException otherwise.
     */    
    int get_value( int index ) const;

    //! Get the index of a given value.
    /*!
     * \return Returns its index if the given value is in the domain, or raises a valueException otherwise.
     */ 
    int index_of( int value ) const;

    friend ostream& operator<<( ostream& os, const Domain& domain )
    {
      os << "Size: " <<  domain.get_size() << "\nDomain: ";
      copy( begin( domain._domain ), end( domain._domain ), ostream_iterator<int>( os, " " ) );
      return os << "\n";
    }
  };

  struct indexException : std::exception
  {
    const char* what() const noexcept { return "Wrong index passed to Domain::get_value.\n"; }
  };

  struct valueException : std::exception
  {
    const char* what() const noexcept { return "Wrong value passed to Domain::index_of.\n"; }
  };
}
