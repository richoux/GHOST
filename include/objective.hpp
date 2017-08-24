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

#include "variable.hpp"
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
  class Objective
  {
  public:
    //! The unique Objective constructor
    /*!
     * \param name A string to give the Objective object a specific name.
     */
    Objective( const string& name );

    //! Inline function following the NVI idiom. Calling v_cost.
    //! \sa required_cost
    inline double cost( const vector< shared_ptr< Variable > >& vecVariables ) const
    { return required_cost( vecVariables ); }

    //! Inline function following the NVI idiom. Calling expert_heuristicVariable.
    //! \sa expert_heuristic_variable
    inline shared_ptr< Variable > heuristic_variable( const vector< shared_ptr< Variable > >& vecVariables ) const
    { return expert_heuristic_variable( vecVariables ); }
    
    //! Inline function following the NVI idiom. Calling expert_heuristicValue.
    //! \sa expert_heuristic_value
    inline int heuristic_value( const vector< shared_ptr< Variable > >& vecVariables,
				shared_ptr< Variable > var,
				const vector< int >& valuesList ) const
    { return expert_heuristic_value( vecVariables, var, valuesList ); }

    //! Inline function following the NVI idiom. Calling expert_heuristicValue.
    //! \sa expert_heuristic_value
    inline shared_ptr< Variable > heuristic_value( const vector< shared_ptr< Variable > >& variablesList ) const
    { return expert_heuristic_value( variablesList ); }

    //! Inline function following the NVI idiom. Calling expert_postprocessSatisfaction.
    //! \sa expert_postprocess_satisfaction
    inline void postprocess_satisfaction( const vector< shared_ptr< Variable > >& vecVariables,
					  double& bestCost,
					  vector< int >& bestSolution ) const
    { expert_postprocess_satisfaction( vecVariables, bestCost, bestSolution ); }

    //! Inline function following the NVI idiom. Calling expert_postprocessOptimization.
    //! \sa expert_postprocess_optimization
    inline void postprocess_optimization( const vector< shared_ptr< Variable > >& vecVariables,
					  double& bestCost,
					  vector< int >& bestSolution ) const
    { expert_postprocess_optimization( vecVariables, bestCost, bestSolution ); }

    //! Inline accessor to get the name of the objective object.
    inline string get_name() const { return name; }

  protected:
    //! Pure virtual function to compute the value of the objective function on the current configuration.
    /*! 
     * \param vecVariables A pointer to the vector of variable objects of the CSP/COP.
     * \return The value of the objective function on the current configuration.
     * \sa cost
     */
    virtual double required_cost( const vector< shared_ptr< Variable > >& vecVariables ) const = 0;

    //! Virtual function to apply the variable heuristic used by the solver.
    /*! 
     * \param vecVariables The vector of variable objects of the CSP/COP.
     * \return The address of a random variable in vecVariables
     * \sa heuristic_variable
     */
    virtual shared_ptr< Variable > expert_heuristic_variable( const vector< shared_ptr< Variable > >& vecVariables ) const;

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
     * \return The selected value according to the heuristic.
     * \sa heuristic_value, Random
     */
    virtual int	expert_heuristic_value( const vector< shared_ptr< Variable > >& vecVariables,
					shared_ptr< Variable > var,
					const vector< int >& valuesList ) const;

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
     * \return The selected value according to the heuristic.
     * \sa heuristic_value, Random
     */
    virtual shared_ptr< Variable > expert_heuristic_value( const vector< shared_ptr< Variable > >& variablesList ) const;

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
     * \sa postprocess_satisfaction
     */
    virtual void expert_postprocess_satisfaction( const vector< shared_ptr< Variable > >& vecVariables,
						  double& bestCost,
						  vector< int >& solution ) const
    { }

    //! Virtual function to perform optimization post-processing.
    /*! 
     * This function is called by the solver after all optimization
     * runs to apply human-knowledge optimization, allowing to improve
     * the optimization cost.
     *
     * This implementation by default does nothing.
     *
     * WARNING: The computation spantime of this function is not taken into account by timeouts given to the solver. 
     * If you override this function, be sure its computation time is neglictable compare to the optimization timeout
     * you give to Solver::solve.  
     * 
     * \param vecVariables A constant pointer to the vector of variable objects of the CSP/COP.
     * \param bestCost A reference the double representing the best optimization cost found by the solver so far.
     * \sa postprocess_optimization
     */
    virtual void expert_postprocess_optimization( const vector< shared_ptr< Variable > >& vecVariables,
						  double& bestCost,
						  vector< int >& bestSolution ) const
    { }

    Random random;				//!< Random generator used by the function heuristicValue.
    string name;				//!< String for the name of the objective object.
  };

  //! NullObjective is used when no objective functions have been given to the solver (ie, for pure satisfaction runs). 
  class NullObjective : public Objective
  {
    using Objective::random;
    
  public:
    NullObjective() : Objective("nullObjective") { }

  private:
    double required_cost( const vector< shared_ptr< Variable > >& vecVariables ) const override { return 0.; }

    shared_ptr< Variable > expert_heuristic_variable( const vector< shared_ptr< Variable > >& vecVariables ) const override
    {
      return vecVariables[ random.get_random_number( vecVariables.size() ) ];
    }

    int expert_heuristic_value( const vector< shared_ptr< Variable > >& vecVariables,
				shared_ptr< Variable > var,
				const vector< int >& valuesList ) const override
    {
      return valuesList[ random.get_random_number( valuesList.size() ) ];
    }

    shared_ptr< Variable > expert_heuristic_value( const vector< shared_ptr< Variable > >& variablesList ) const override
    {
      return variablesList[ random.get_random_number( variablesList.size() ) ];
    }
  };
}
