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

#include "misc/random.hpp"

using namespace std;

namespace ghost
{
  //! Domain is the class encoding the domain of your CSP/COP.
  /*! 
   */
  class Domain
  {
  public:
    //! Domain constructor.
    /*!
     * Constructor taking the outside-the-scope value and a vector of integer values, to 
     * make both the initial and current possible variable values. The outside-the-scope value
     * must not belong to this list, or an ArgumentException is raised.
     */
    Domain( const vector< int > &domain, int outsideScope = -1 )
      : currentDomain(domain),
	initialDomain(domain),
	outsideScope(outsideScope)
    { }

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

    //! Inline function following the NVI idiom. Calling v_restart.
    inline void restart( vector<TypeVariable> *variables ) { v_restart( variables ); }

    //! Inline function following the NVI idiom. Calling v_wipe.
    inline void wipe( vector<TypeVariable> *variables ) { v_wipe( variables ); }

    //! Inline function following the NVI idiom. Calling v_rebuild.
    inline void rebuild( vector<TypeVariable> *variables ) { v_rebuild( variables ); }

    //! Inline function following the NVI idiom. Calling v_copyBest.
    inline void copyBest( vector<TypeVariable> &best, vector<TypeVariable> *variables ) { v_copyBest( best, variables ); }

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

    //! virtual function called in Sovler::solve before
    //! postprocessingOptimization. By default, copy only the value of best[i] into variables[i].
    /*!
     * \param best The vector of best configuration found by the solver.
     * \param variables The regular vector used to describe the vector of variables.
     */
    virtual void v_copyBest( vector<TypeVariable> &best, vector<TypeVariable> *variables )
    {
      for( int i = 0 ; i < best.size() ; ++i )
	variables->at(i).setValue( best[i].getValue() );
    }

    vector< int > currentDomain;	//!< Vector of integers containing the current values of the domain.
    vector< int > initialDomain;	//!< Vector of integers containing the initial values of the domain.
    int outsideScope;			//!< Value representing all values outside the scope of the domain
    Random random;			//!< A random generator used by the function randomValue. 
  };
}
