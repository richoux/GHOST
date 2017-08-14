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

#include <algorithm>
#include <limits>
#include <vector>

#include "misc/random.hpp"

using namespace std;

namespace ghost
{
  //! Objective is the class encoding objective functions of your CSP/COP.
  /*! 
   * In GHOST, many different objective objects can be instanciate.
   *
   * The Objective class is a template class, waiting for both the
   * type of variable and the type of domain. Thus, you must
   * instanciate a constraint by specifying the class of your variable
   * objects and the class of your domain object, like for instance
   * Objective<Variable, Domain> or Objective<MyCustomVariable,
   * MyCustomDomain>, if MyCustomVariable inherits from the
   * ghost::Variable class and MyCustomDomain inherits from the
   * ghost::Domain class.
   *
   * You cannot directly use this class Objective to encode your
   * objective functions since this is an abstract class. Thus, you
   * must write your own objective class inheriting from ghost::Objective.
   *
   * In this class, each virtual function follows the Non-Virtual
   * Interface Idiom (see http://www.gotw.ca/publications/mill18.htm).
   * The only pure virtual function is v_cost.
   *
   * \sa Variable
   */
  template <typename TypeVariable>
  class Objective
  {
  public:
    //! The unique Objective constructor
    /*!
     * \param name A string to give the Objective object a specific name.
     */
    Objective( const string& name, bool permut = false ) : name(name), permutation(permut) { }

    //! Inline function following the NVI idiom. Calling v_cost.
    //! \sa v_cost
    inline double cost( vector< TypeVariable > *vecVariables ) const
    { return v_cost( vecVariables ); }

    //! Inline function following the NVI idiom. Calling v_heuristicVariable.
    //! \sa v_heuristicVariable
    inline int heuristicVariable( const vector< int >& vecVarId,
				  vector< TypeVariable > *vecVariables )
    { return v_heuristicVariable( vecVarId, vecVariables ); }
    
    //! Inline function following the NVI idiom. Calling v_heuristicValue.
    //! \sa v_heuristicValue
    inline int heuristicValue( const std::vector< double >& vecGlobalCosts, 
				double& bestEstimatedCost,
				int& bestValue ) const
    { return v_heuristicValue( vecGlobalCosts, bestEstimatedCost, bestValue ); }

    //! Inline function following the NVI idiom. Calling v_setHelper.
    //! \sa v_setHelper
    inline void setHelper( const TypeVariable& variable,
			   vector< TypeVariable > *vecVariables )
    { v_setHelper( variable, vecVariables ); }

    //! Inline function following the NVI idiom. Calling v_postprocessSatisfaction.
    //! \sa v_postprocessSatisfaction
    inline double postprocessSatisfaction( vector< TypeVariable > *vecVariables,
					   double& bestCost,
					   vector< TypeVariable >& bestSolution,
					   double sat_timeout )
    { return v_postprocessSatisfaction( vecVariables, bestCost, bestSolution, sat_timeout ); }

    //! Inline function following the NVI idiom. Calling v_postprocessOptimization.
    //! \sa v_postprocessOptimization
    inline double postprocessOptimization( vector< TypeVariable > *vecVariables,
					   double& bestCost,
					   double opt_timeout )
    { return v_postprocessOptimization( vecVariables, bestCost, opt_timeout ); }

    //! Inline accessor to get the name of the objective object.
    inline string getName() const { return name; }

    //! Inline function to initialize heuristicValueHelper to a vector of MAX_INT values.  
    //! \sa heuristicValueHelper
    inline void initHelper( int size )
    {
      heuristicValueHelper = std::vector<double>( size, numeric_limits<int>::max() );
    }

    //! Inline function to reset heuristicValueHelper with MAX_INT values.  
    //! \sa heuristicValueHelper
    inline void resetHelper()
    {
      std::fill( heuristicValueHelper.begin(), heuristicValueHelper.end(), numeric_limits<int>::max() );
    }

    //! Inline function returning permutation.
    //! \sa permutation
    inline bool isPermutation() const { return permutation; }

  protected:
    //! Pure virtual function to compute the value of the objective function on the current configuration.
    /*! 
     * \param vecVariables A pointer to the vector of variable objects of the CSP/COP.
     * \return The value of the objective function on the current configuration.
     * \sa cost
     */
    virtual double v_cost( vector< TypeVariable > *vecVariables ) const = 0;

    //! Pure virtual function to apply the variable heuristic used by the solver.
    /*! 
     * \param vecVarId A constant reference to the vector of variable ID objects of the CSP/COP.
     * \param vecVariables A pointer to the vector of variable objects of the CSP/COP.
     * \return The ID of the selected variable according to the heuristic.
     * \sa heuristicVariable
     */
    virtual int	v_heuristicVariable( const vector< int >& vecVarId,
				     vector< TypeVariable > *vecVariables )
    {

    }

    //! Pure virtual function to set heuristicValueHelper[currentVar.getValue()].
    /*! 
     * \param currentVar A constant reference to a variable object.
     * \param vecVariables A pointer to the vector of variable objects of the CSP/COP.
     * \sa setHelper, heuristicValueHelper
     */
    virtual void v_setHelper( const TypeVariable& currentVar,
			      vector< TypeVariable > *vecVariables ) = 0;

    //! Virtual function to perform satisfaction post-processing.
    /*! 
     * This function is called by the solver after a satisfaction run,
     * if the solver was able to find a solution, to apply
     * human-knowledge in order to "clean-up" the proposed solution.
     *
     * This implementation by default does nothing.
     * 
     * \param vecVariables A constant pointer to the vector of variable objects of the CSP/COP.
     * \param bestCost A reference the double representing the best global cost found by the solver so far.
     * \param solution A reference to the vector of variables of the solution found by the solver.
     * \param sat_timeout The satisfaction timeout given in argument of Solver::solve
     * \return The function runtime in milliseconds.
     * \sa postprocessSatisfaction
     */
    virtual double v_postprocessSatisfaction( vector< TypeVariable > *vecVariables,
					      double& bestCost,
					      vector< TypeVariable > &solution,
					      double sat_timeout) const
    {
      bestCost = 0.;
      return 0.;
    }

    //! Virtual function to perform optimization post-processing.
    /*! 
     * This function is called by the solver after all optimization
     * runs to apply human-knowledge optimization, allowing to improve
     * the optimization cost.
     *
     * This implementation by default does nothing.
     * 
     * \param vecVariables A constant pointer to the vector of variable objects of the CSP/COP.
     * \param bestCost A reference the double representing the best optimization cost found by the solver so far.
     * \param opt_timeout The optimization timeout given in argument of Solver::solve
     * \return The function runtime in milliseconds.
     * \sa postprocessOptimization
     */
    virtual double v_postprocessOptimization( vector< TypeVariable > *vecVariables,
					      double& bestCost,
					      double opt_timeout)
    {
      bestCost = 0.;
      return 0.;
    }

    //! Virtual function to apply the value heuristic used by the solver.
    /*! 
     * This default implementation outputs the value leading to the
     * lowest global cost. It uses heuristicValueHelper as a
     * tiebreaker, if two or more values lead to configurations with
     * the same lowest global cost. If two or more values cannot be
     * tiebreak by heuristicValueHelper, one of them is randomly
     * selected.
     *
     * \param 
     * \param 
     * \param 
     * \return The selected value according to the heuristic.
     * \sa heuristicValue, heuristicValueHelper, Random
     */
    virtual int	v_heuristicValue( const std::vector< double >& vecGlobalCosts, 
				  double& bestEstimatedCost,
				  int& bestValue ) const
    {
      int best = 0;
      double bestHelp = numeric_limits<int>::max();

      vector<int> draw;
	
      for( int i = 0; i < vecGlobalCosts.size(); ++i )
      {
	if( vecGlobalCosts[i] == bestEstimatedCost
	    && vecGlobalCosts[i] < numeric_limits<int>::max()
	    && heuristicValueHelper.at( i ) == bestHelp )
	{
	  draw.push_back(i);
	}
	else
	  if( vecGlobalCosts[i] < bestEstimatedCost
	      || ( vecGlobalCosts[i] == bestEstimatedCost
		   && vecGlobalCosts[i] < numeric_limits<int>::max()
		   && heuristicValueHelper.at( i ) < bestHelp ) )
	  {
	    draw.clear();
	    bestEstimatedCost = vecGlobalCosts[i];

	    // WARNING! This line implies we start our domain by the value -1
	    // which is not necessarily the case. TOFIX
	    bestValue = i - 1;
	    if( heuristicValueHelper.at( i ) < bestHelp )
	      bestHelp = heuristicValueHelper.at( i );
	    best = i;
	  }
      }

      if( draw.size() > 1 )
      {
	int i = draw[ const_cast<Objective*>(this)->randomVar.getRandNum( draw.size() ) ];
	  
	bestEstimatedCost = vecGlobalCosts[i];
	
	// WARNING! This line implies we start our domain by the value -1
	// which is not necessarily the case. TOFIX
	bestValue = i - 1;
	if( heuristicValueHelper.at( i ) < bestHelp )
	  bestHelp = heuristicValueHelper.at( i );
	best = i;
      }

      return best;
    }


    Random randomVar;				//!< Random generator used by the function heuristicValue.
    string name;				//!< String for the name of the objective object.
    vector<double> heuristicValueHelper;	//!< Vector of local costs used by the value heuristic. Represent the cost for each possible value of a given variable. Used to tie-break equivalent configuration costs.
    bool   permutation;				//!< Boolean telling is the problem is a permutation problem or not.
  };

  //! NullObjective is used when no objective functions have been given to the solver (ie, for pure satisfaction runs). 
  template <typename TypeVariable>
  class NullObjective : public Objective<TypeVariable>
  {
    using Objective<TypeVariable>::randomVar;
    using Objective<TypeVariable>::heuristicValueHelper;
    
  public:
    NullObjective() : Objective<TypeVariable>("nullObjective") { }

  private:
    double v_cost( vector< TypeVariable > *vecVariables ) const override { return 0.; }
    
    int	v_heuristicVariable( const vector< int >& vecId,
			     vector< TypeVariable > *vecVariables ) override
    {
      return vecId[ randomVar.getRandNum( vecId.size() ) ];
    }
    
    void v_setHelper( const TypeVariable& b,
		      vector< TypeVariable > *vecVariables ) override
    {
      heuristicValueHelper.at( b.getValue() ) = 0;
    }
  };
}
