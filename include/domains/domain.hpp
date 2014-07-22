/*
 * GHOST (General meta-Heuristic Optimization Solving Tool) is a C++ library 
 * designed for StarCraft: Brood war. 
 * GHOST is a meta-heuristic solver aiming to solve any kind of combinatorial 
 * and optimization RTS-related problems represented by a CSP/COP. 
 * It is a generalization of the project Wall-in.
 * Please visit https://github.com/richoux/GHOST for further information.
 * 
 * Copyright (C) 2014 Florian Richoux
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

#include "../misc/random.hpp"

using namespace std;

namespace ghost
{
  //! Domain is the class encoding the domain of your CSP/COP.
  /*! 
   * In GHOST, only one domain object should be instanciate. At least,
   * the solver is only taking one domain object in parameter.
   *
   * The Domain class is a template class, waiting for the type of
   * variable. Thus, you must instanciate a domain by specifying the
   * class of your variable objects, like for instance
   * Domain<Variable> or Domain<MyCustomVariable>, if MyCustomVariable
   * inherits from the ghost::Variable class.
   *
   * Since in GHOST, variables can only take integer values, a domain
   * object would contain the possible integer values for each
   * variable of the CSP/COP.
   *
   * To encode your CSP/COP domain, you can either directly use this
   * class Domain (there are no pure virtual functions here),
   * or inherit from it to make your own domain class.
   *
   * \sa Variable
   */
  template <typename TypeVariable>
  class Domain
  {
  public:
    //! First Domain constructor.
    /*!
     * In this constructor, the domain of each variable is built to be
     * equals to the range [start, start + size[
     *
     * \param size An integer to specify the size of the domain.
     * \param numberVariables An integer to specify the number of variables in the CSP/COP.
     * \param start The starting value of the domain. If not given, the default value is 0.
     */
    Domain( int size, int numberVariables, int start = 0 )
      : size(size),
	domains(vector< vector<int> >( numberVariables )),
	initialDomain(vector<int>( size ))
    {
      std::iota( begin(initialDomain), end(initialDomain), start );
      for( int i = 0; i < numberVariables; ++i )
      {
	// domains[i] = vector<int>( size );
	// std::iota( begin(domains[i]), end(domains[i]), start );

	domains[i] = initialDomain;
      }
    }

    //! Second and last Domain constructor.
    /*!
     * In this constructor, the domain of each variable is given as a
     * parameter.
     *
     * \param size An integer to specify the size of the domain.
     * \param numberVariables An integer to specify the number of variables in the CSP/COP.
     * \param initialDomain A constant reference to an vector of integer, representing the inital domain for each variable.
     */
    Domain( int size, int numberVariables, const vector< int > &initialDomain )
      : size(size),
	domains(vector< vector<int> >( numberVariables )),
	initialDomain(initialDomain)
    {
      for( int i = 0; i < numberVariables; ++i )
      {
	domains[i] = vector<int>( size );	  
	std::copy( begin(initialDomain), end(initialDomain), domains[i].begin() );
      }
    }
        
    //! Inline function following the NVI idiom. Calling v_restart.
    inline void restart( vector<TypeVariable> *variables ) { v_restart( variables ); }

    //! Inline function following the NVI idiom. Calling v_wipe.
    inline void wipe( vector<TypeVariable> *variables ) { v_wipe( variables ); }

    //! Inline function following the NVI idiom. Calling v_rebuild.
    inline void rebuild( vector<TypeVariable> *variables ) { v_rebuild( variables ); }

    //! Inline function to get a random value among the possible values of a given variable.
    /*!
     * \param variable A constant reference to a variable.
     * \return A random value among the possible values of variable.
     * \sa Random
     */
    inline int randomValue( const TypeVariable& variable )
    {
      vector<int> possibilities = domains[ variable.getId() ];
      return possibilities[ random.getRandNum( possibilities.size() ) ];
    }
      
    //! Inline function to get the vector of the possible values of a given variable.
    /*!
     * \param variable A constant reference to a variable.
     * \return The vector of integers of all possible values of variable.
     */
    inline vector<int> valuesOf( const TypeVariable& variable ) const
    {
      return domains[ variable.getId() ];
    }
      
    //! Inline function to reset the domain of a given variable to the
    //! initial domain.
    /*!
     * The domain of the given variable will be reset to the initial
     * domain created or given while the domain object has been
     * instanciated.
     *
     * \param variable A constant reference to a variable.
     */
    inline void	resetDomain( const TypeVariable& variable )
    {
      domains[ variable.getId() ] = initialDomain;
    }
      
    //! Inline function to reset all variable domains to the initial
    //! domain. 
    /*!
     * All variable domains will be reset to the initial domain
     * created or given while the domain object has been instanciated.
     */
    inline void	resetAllDomains()
    {
      for( auto& d : domains )
	d = initialDomain;
    }

    //! Inline accessor to get the size of the domain.
    inline int getSize() const { return size; }

    //! Inline function to add something into the domain.
    /*!
     * The implementation by default does nothing. This function has
     * been declared because it could be useful for some custom domain
     * classes to add a value from the given variable into a custom
     * data structure. This function is called into the solver three times:
     * - during a move (Solver::move), ie, when the solver assigns a new value to a given variable.
     * - during a reset (Solver::reset).
     * - just between the end of the optimization run and the beginning of the optimization post-processing, in Solver::solve.
     *
     * \param variable A constant reference to a variable.
     */
    inline void add( const TypeVariable& variable ) { }      

    //! Inline function to clear (or remove) something into the domain.
    /*!
     * The implementation by default does nothing. This function has
     * been declared because it could be useful for some custom domain
     * classes to clear/remove a value from the given variable into a custom
     * data structure. This function is called into the solver three times:
     * - during a move (Solver::move), ie, when the solver assigns a new value to a given variable.
     * - during a reset (Solver::reset).
     * - just between the end of the optimization run and the beginning of the optimization post-processing, in Solver::solve.
     *
     * \param variable A constant reference to a variable.
     */
    inline void	clear( const TypeVariable& variable ) { }

    //! friend override of operator<<
    friend ostream& operator<<( ostream& os, const Domain<TypeVariable>& domain )
    {
      os << "Domain type: " <<  typeid(domain).name() << endl
	 << "Size: " <<  domain.size << endl;
      return os;
    }

  protected:
    //! Pure virtual function restarting the
    //! search process from a fresh and randomly generated
    //! configuration.
    virtual void v_restart( vector<TypeVariable> *variables ) = 0;

    //! Empty virtual function called in Sovler::solve before
    //! postprocessingOptimization. Can be use to flush variables
    //! in the domain.
    virtual void v_wipe( vector<TypeVariable> *variables ) { }

    //! Empty virtual function called in Sovler::solve before
    //! postprocessingOptimization. Can be use to rebuild the domain.
    virtual void v_rebuild( vector<TypeVariable> *variables ) { }

    int size;				//!< An integer to specify the size of the domain.
    vector< vector< int > > domains;	//!< The vector of vector of integers, containing the domain of each variables. Thus, domains[i] is the domain of the variable i.
    vector< int > initialDomain;	//!< The initial domain, created or given according to the constructor which has been called. 
    Random random;			//!< The random generator used by the function randomValue. 
  };
}
