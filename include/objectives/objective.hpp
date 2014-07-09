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

#include <algorithm>
#include <limits>
#include <vector>

#include "../misc/random.hpp"
#include "../misc/constants.hpp"

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
   * objective functions, since this is an abstract class (see the
   * list of pure virtual functions below). Thus, you must write your
   * own objective class inheriting from ghost::Objective.
   *
   * In this class, each virtual function follows the Non-Virtual Interface
   * Idiom (see http://www.gotw.ca/publications/mill18.htm). The list
   * of all Objective pure virtual functions is below:
   * - v_cost 
   * - v_heuristicVariable
   * - v_setHelper
   *
   * \sa Variable, Domain
   */
  template <typename TypeVariable, typename TypeDomain>
  class Objective
  {
  public:
    //! The unique Objective constructor
    /*!
     * \param name A string to give the Objective object a specific name.
     */
    Objective( const string &name ) : name(name) { }

    //! Inline function following the NVI idiom. Calling v_cost.
    //! \sa v_cost
    inline double cost( const vector< TypeVariable > *vecVariables,
			const TypeDomain *domain ) const
    { return v_cost(vecVariables, domain); }

    //! Inline function following the NVI idiom. Calling v_heuristicVariable.
    //! \sa v_heuristicVariable
    inline int heuristicVariable( const vector< int > &vecVarId,
				  const vector< TypeVariable > *vecVariables,
				  TypeDomain *domain )
    { return v_heuristicVariable(vecVarId, vecVariables, domain); }
    
    //! Inline function following the NVI idiom. Calling v_heuristicValue.
    //! \sa v_heuristicValue
    inline int heuristicValue( const std::vector< double > &vecGlobalCosts, 
				double &bestEstimatedCost,
				int &bestValue ) const
    { return v_heuristicValue( vecGlobalCosts, bestEstimatedCost, bestValue ); }

    //! Inline function following the NVI idiom. Calling v_setHelper.
    //! \sa v_setHelper
    inline void setHelper( const TypeVariable &variable,
			   const vector< TypeVariable > *vecVariables,
			   const TypeDomain *domain )
    { v_setHelper(variable, vecVariables, domain); }

    //! Inline function following the NVI idiom. Calling v_postprocessSatisfaction.
    //! \sa v_postprocessSatisfaction
    inline double postprocessSatisfaction( vector< TypeVariable > *vecVariables,
					   TypeDomain *domain,
					   double &bestCost,
					   vector<int> &bestSolution)
    { return v_postprocessSatisfaction(vecVariables, domain, bestCost, bestSolution); }

    //! Inline function following the NVI idiom. Calling v_postprocessOptimization.
    //! \sa v_postprocessOptimization
    inline double postprocessOptimization( vector< TypeVariable > *vecVariables,
					   TypeDomain *domain,
					   double &bestCost )
    { return v_postprocessOptimization(vecVariables, domain, bestCost); }

    //! Inline accessor to get the name of the objective object.
    inline string getName() { return name; }

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

    //! updateHelper is used to update heuristicValueHelper.
    /*! 
     * The function updateHelper is called by Solver::solve before
     * each call of heuristicValue.
     *
     * \param currentVar A reference to a variable object.
     * \param possibleValues A constant reference to the vector of all possible values of currentVar.
     * \param variables A constant pointer to the vector of variable objects of the CSP/COP.
     * \param domain A pointer to the domain object of the CSP/COP.
     * \sa heuristicValueHelper
    */
    void updateHelper( TypeVariable &currentVar,
		       const vector<int> &possibleValues,
		       const vector< TypeVariable > *variables,
		       TypeDomain *domain )
    {
      resetHelper();
      int backup = currentVar.getValue();
      
      for( auto &val : possibleValues )
      {
	domain->clear( currentVar );
	currentVar.setValue( val );
	domain->add( currentVar );
	
	setHelper( currentVar, variables, domain );
      }

	domain->clear( currentVar );
	currentVar.setValue( backup );
	domain->add( currentVar );      
    }

  protected:
    //! Pure virtual function to compute the value of the objective function on the current configuration.
    /*! 
     * \param vecVariables A constant pointer to the vector of variable objects of the CSP/COP.
     * \param domain A constant pointer to the domain object of the CSP/COP.
     * \return The value of the objective function on the current configuration.
     * \sa cost
     */
    virtual double v_cost( const vector< TypeVariable > *vecVariables,
			   const TypeDomain *domain ) const = 0;

    //! Pure virtual function to apply the variable heuristic used by the solver.
    /*! 
     * \param vecVarId A constant reference to the vector of variable ID objects of the CSP/COP.
     * \param vecVariables A constant pointer to the vector of variable objects of the CSP/COP.
     * \param domain A constant pointer to the domain object of the CSP/COP.
     * \return The ID of the selected variable according to the heuristic.
     * \sa heuristicVariable
     */
    virtual int	v_heuristicVariable( const vector< int > &vecVarId,
				     const vector< TypeVariable > *vecVariables,
				     TypeDomain *domain ) = 0;

    //! Pure virtual function to set heuristicValueHelper[currentVar.getValue()].
    /*! 
     * \param currentVar A constant reference to a variable object.
     * \param vecVariables A constant pointer to the vector of variable objects of the CSP/COP.
     * \param domain A constant pointer to the domain object of the CSP/COP.
     * \sa setHelper, heuristicValueHelper
     */
    virtual void v_setHelper( const TypeVariable &currentVar,
			      const vector< TypeVariable > *vecVariables,
			      const TypeDomain *domain ) = 0;

    //! Virtual function to perform satisfaction post-processing.
    /*! 
     * This function is called by the solver after a satisfaction run,
     * if the solver was able to find a solution, to apply
     * human-knowledge in order to "clean-up" the proposed solution.
     *
     * This implementation by default does nothing.
     * 
     * \param vecVariables A constant pointer to the vector of variable objects of the CSP/COP.
     * \param domain A constant pointer to the domain object of the CSP/COP.
     * \param bestCost A reference the double representing the best global cost found by the solver so far.
     * \param solution A reference to the vector of variable values of the solution found by the solver.
     * \return The function runtime in milliseconds.
     * \sa postprocessSatisfaction
     */
    virtual double v_postprocessSatisfaction( vector< TypeVariable > *vecVariables,
					      TypeDomain *domain,
					      double &bestCost,
					      vector<int> &solution ) const { return 0.; }

    //! Virtual function to perform optimization post-processing.
    /*! 
     * This function is called by the solver after all optimization
     * runs to apply human-knowledge optimization, allowing to improve
     * the optimization cost.
     *
     * This implementation by default does nothing.
     * 
     * \param vecVariables A constant pointer to the vector of variable objects of the CSP/COP.
     * \param domain A constant pointer to the domain object of the CSP/COP.
     * \param bestCost A reference the double representing the best optimization cost found by the solver so far.
     * \return The function runtime in milliseconds.
     * \sa postprocessOptimization
     */
    virtual double v_postprocessOptimization( vector< TypeVariable > *vecVariables,
					      TypeDomain *domain,
					      double &bestCost ) { return 0.; }

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
    virtual int	v_heuristicValue( const std::vector< double > &vecGlobalCosts, 
				  double &bestEstimatedCost,
				  int &bestValue ) const
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
	bestValue = i - 1;
	if( heuristicValueHelper.at( i ) < bestHelp )
	  bestHelp = heuristicValueHelper.at( i );
	best = i;
      }

      return best;
    }


    Random randomVar;				//!< The random generator used by the function heuristicValue.
    string name;				//!< A string for the name of the objective object.
    vector<double> heuristicValueHelper;	//!< The vector of double values implementing the value heuristic for each possible value of a given variable.
  };

  //! NullObjective is used when no objective functions have been given to the solver (ie, for pure satisfaction runs). 
  template <typename TypeVariable, typename TypeDomain>
  class NullObjective : public Objective<TypeVariable, TypeDomain>
  {
    using Objective<TypeVariable, TypeDomain>::randomVar;
    using Objective<TypeVariable, TypeDomain>::heuristicValueHelper;
    
  public:
    NullObjective() : Objective<TypeVariable, TypeDomain>("nullObjective") { }

  private:
    virtual double v_cost( const vector< TypeVariable > *vecVariables,
			   const TypeDomain *domain ) const
    { return 0.; }
    
    virtual int	v_heuristicVariable( const vector< int > &vecId,
				     const vector< TypeVariable > *vecVariables,
				     TypeDomain *domain )
    {
      return vecId[ randomVar.getRandNum( vecId.size() ) ];
    }
    
    virtual void v_setHelper( const TypeVariable &b,
			      const vector< TypeVariable > *vecVariables,
			      const TypeDomain *domain )
    {
      heuristicValueHelper.at( b.getValue() ) = 0;
    }
  };
}
