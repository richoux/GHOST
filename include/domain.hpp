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
#include <iterator>
#include <exception>

#include "misc/random.hpp"

using namespace std;

namespace ghost
{
  //! This class encodes domains of your CSP/COP variables. You cannot inherits your own class from Domain.  
  /*! 
   * Domain is the class implementing variable domains, ie, the set of possible values a variable can take.
   * In GHOST, such values must be integers, possibly positive, negative or both. 
   */
  class Domain final
  {
    vector< int >	_domain;	//!< Vector of integers containing the current values of the domain.
    vector< int >	_indexes;	//!< Vector of integers containing indexes of current values of the domain.
    int			_minValue;	//!< Min value, used for indexes.
    int			_maxValue;	//!< Max value.
    size_t		_size;		//!< Size of _domain, ie, number of elements it contains.
    Random		_random;	//!< A random generator used by the function randomValue. 

    /*
      Why having both domain and indexes vectors?

      The domain vector contains integers modelling possible values of a variable. Such values can be 
      {7, -1, 3}. Then, your domain can be not canonically ordered and have 'holes', ie, non-contiguous integers.
      Thus, it can be more convient for Variable objects to handle the index of their value in the domain rather 
      than their value itself. Indead, taking the next value in the domain is just incrementing the current index, 
      rather than searching for the next possible value in the domain.
      However, it is necessary to know which index is associated to a value, for instance for seting a new value 
      (in fact, a new index) to a variable. We had then two choices: searching for such an index in the domain vector
      or storing all indexes in another vector. For speed sake, we chose this second option.
     */
    
    struct indexException : std::exception
    {
      const char* what() const noexcept { return "Wrong index passed to Domain::get_value.\n"; }
    };
    
    struct valueException : std::exception
    {
      const char* what() const noexcept { return "Wrong value passed to Domain::index_of.\n"; }
    };
    
    //! For the copy-and-swap idiom
    void swap( Domain &other );

  public:
    /*!
     * Constructor taking a vector of integer values. Values in this vector will constitute the domain values. 
     * For instance, the code as follows
     * 
     * std::vector<int> v{ 7, -1, 3 };\n
     * Domain d( v );
     * 
     * will create a domain with three values: 7, -1 and 3, in that order.
     *
     * \param domain A vector of int corresponding to the variable domain.
     */
    Domain( const vector< int >& domain );

    /*!
     * Constructor taking a starting value 'startValue' and the domain of size 'size', 
     * thus creating a domain with all integers in [startValue, startValue + size].
     *
     * \param startValue An integer specifying what is the first value in the domain.
     * \param size A size_t specifying the number of elements in the domain.
     */
    Domain( int startValue, size_t size );

    /*!
     * Unique copy constructor.
     *
     * \param other A const reference to a Domain object.
     */
    Domain( const Domain &other );

    /*!
     * Copy assignment operator follwing the copy-and-swap idiom.
     * 
     * \param other A Domain object.
     */
    Domain& operator=( Domain other );

    //! Default destructor.
    ~Domain() = default;
    
    /*!
     * Inline function returning a random value from the domain, following a near-uniform distribution.
     * 
     * \sa Random
     */
    inline int random_value() const
    {
      return _domain[ _random.get_random_number( (int)_size ) ];
    }

    /*!
     * Return the number of values currently composing the domain.
     * 
     * \return A size_t corresponding to the size of the domain.
     */
    inline size_t get_size() const
    {
      return _size;
    }

    /*!
     * Return the minimal value in the domain
     * 
     * \return The minimal value in the domain.
     */    
	  inline int get_min_value() const { return _minValue; }

	  /*!
     * Return the maximal value in the domain
     * 
     * \return The maximal value in the domain.
     */    
	  inline int get_max_value() const { return _maxValue; }

	  /*!
     * Inline function to get the full domain.
     *
     * \return A const reference to the vector of integer within Domain representing the domain.
     */
    inline const vector<int>& get_domain() const
    {
      return _domain;
    }

    /*!
     * Get the value at the given index. 
     * 
     * \param index is the index of the desired value.
     * \return The value at the given index if this one is within the domain range, or raises an indexException otherwise.
     */    
    int get_value( int index ) const;

    /*!
     * Get the index of a given value.
     * 
     * \return If the given value is in the domain, returns its index; raises a valueException otherwise.
     */ 
    int index_of( int value ) const;

    //! To have a nicer stream of Domain.
    friend ostream& operator<<( ostream& os, const Domain& domain )
    {
      os << "Size: " <<  domain._size << "\nDomain: ";
      copy( begin( domain._domain ), end( domain._domain ), ostream_iterator<int>( os, " " ) );
      return os << "\n";
    }
  };
}
