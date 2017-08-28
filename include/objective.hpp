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
   * The Objective class is a template class, waiting for the type of variable 
   * composing the objective. Thus, you must instanciate an objective by 
   * specifying the class of your variable objects, like for instance
   * Objective<Variable> or Objective<MyCustomVariable>, where MyCustomVariable 
   * must inherits from ghost::Variable.
   *
   * You cannot directly use this class Objective to encode your
   * objective functions since this is an abstract class. Thus, you
   * must write your own objective class inheriting from ghost::Objective.
   *
   * In this class, each virtual function follows the Non-Virtual
   * Interface Idiom (see http://www.gotw.ca/publications/mill18.htm).
   * The only pure virtual function is required_cost. All other virtual functions
   * have a default behavior implemented and are prefixed by 'expert_'. It is
   * highly recommended to override these functions only if you know what you are doing.
   *
   * \sa Variable
   */
  template <typename TypeVariable>
  class Objective
  {
  public:
    //! The unique Objective constructor
    /*!
     * \param name A const reference to a string to give the Objective object a specific name.
     */
    Objective( const string& name );

    //! Default copy and move contructors are explicitely declared.
    Objective( const Objective& other ) = default;
    Objective( Objective&& other ) = default;

    //! Copy and move assignment operators are disabled.
    Objective& operator=( const Objective& other ) = delete;
    Objective& operator=( Objective&& other ) = delete;

    // Default virtual destructor.
    virtual ~Objective() = default;
        
    //! Inline function following the NVI idiom. Calling v_cost.
    /*! 
     * \sa required_cost
     */
    inline double cost( vector< TypeVariable > *variables ) const
    { return required_cost( variables ); }

    //! Inline function following the NVI idiom. Calling expert_heuristic_variable.
    /*! 
     * \sa expert_heuristic_variable
     */
    inline TypeVariable* heuristic_variable( vector< TypeVariable* > *variables ) const
    { return expert_heuristic_variable( variables ); }
    
    //! Inline function following the NVI idiom. Calling expert_heuristic_value.
    /*! 
     * \sa expert_heuristic_value
     */
    inline int heuristic_value( vector< TypeVariable > *variables,
				TypeVariable *var,
				const vector< int >& possible_values ) const
    { return expert_heuristic_value( variables, var, possible_values ); }

    //! Inline function following the NVI idiom. Calling expert_heuristic_value.
    /*! 
     * \sa expert_heuristic_value
     */
    inline TypeVariable* heuristic_value( vector< TypeVariable* > *bad_variables ) const
    { return expert_heuristic_value( bad_variables ); }

    //! Inline function following the NVI idiom. Calling expert_postprocess_satisfaction.
    /*! 
     * \sa expert_postprocess_satisfaction
     */
    inline void postprocess_satisfaction( vector< TypeVariable > *variables,
					  double& bestCost,
					  vector< int >& solution ) const
    { expert_postprocess_satisfaction( variables, bestCost, solution ); }

    //! Inline function following the NVI idiom. Calling expert_postprocess_optimization.
    /*! 
     * \sa expert_postprocess_optimization
     */
    inline void postprocess_optimization( vector< TypeVariable > *variables,
					  double& bestCost,
					  vector< int >& solution ) const
    { expert_postprocess_optimization( variables, bestCost, solution ); }

    //! Inline accessor to get the name of the objective object.
    inline string get_name() const { return name; }

  protected:
    
    Random random;	//!< Random generator used by the function heuristicValue.
    string name;	//!< String for the name of the objective object.

    //! Pure virtual function to compute the value of the objective function on the current configuration.
    /*! 
     * \param variables A pointer to the vector of variable objects of the CSP/COP.
     * \return The value of the objective function on the current configuration.
     * \sa cost
     */
    virtual double required_cost( vector< TypeVariable > *variables ) const = 0;

    //! Virtual function to apply the variable heuristic used by the solver.
    /*! 
     * By default, returns a random variable among the vector in input.
     *
     * \param variables A pointer to the vector of variable pointers of the CSP/COP.
     * \return The address of a random variable in variables
     * \sa heuristic_variable
     */
    virtual TypeVariable* expert_heuristic_variable( vector< TypeVariable* > *variables ) const;

    //! Virtual function to apply the value heuristic used by the solver for non permutation problems.
    /*! 
     * This default implementation outputs the value leading to the
     * lowest objective cost. If two or more values lead to configurations 
     * with the same lowest cost, one of them is randomly returned.
     *
     * \param variables A pointer to the vector of all variables.
     * \param var A pointer to the variable to change.
     * \param possible_values A const reference to the vector of possible values of var. 
     * \return The selected value according to the heuristic.
     * \sa heuristic_value, Random
     */
    virtual int	expert_heuristic_value( vector< TypeVariable > *variables,
					TypeVariable *var,
					const vector< int >& possible_values ) const;

    //! Virtual function to apply the value heuristic used by the solver for permutation problems.
    /*! 
     * By default, returns a random variable among the vector in input.
     *
     * \param bad_variables A pointer to the vector of variable pointers.
     * \return The selected variable to swap with, according to the heuristic.
     * \sa heuristic_value, Random
     */
    virtual TypeVariable* expert_heuristic_value( vector< TypeVariable* > *bad_variables ) const;

    //! Virtual function to perform satisfaction post-processing.
    /*! 
     * This function is called by the solver after a satisfaction run,
     * if the solver was able to find a solution, to apply
     * human-knowledge in order to "clean-up" the proposed solution.
     *
     * This implementation by default does nothing.
     * 
     * \param variables A pointer to the vector of variable objects of the CSP/COP.
     * \param bestCost A reference the double representing the best global cost found by the solver so far. This parameter must be updated.
     * \param solution A reference to the vector of variables of the solution found by the solver. This parameter must be updated.
     * \sa postprocess_satisfaction
     */
    virtual void expert_postprocess_satisfaction( vector< TypeVariable > *variables,
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
     * \param variables A pointer to the vector of variable objects of the CSP/COP.
     * \param bestCost A reference the double representing the best optimization cost found by the solver so far. This parameter must be updated.
     * \param solution A reference to the vector of variables of the solution found by the solver. This parameter must be updated.
     * \sa postprocess_optimization
     */
    virtual void expert_postprocess_optimization( vector< TypeVariable > *variables,
						  double& bestCost,
						  vector< int >& solution ) const
    { }
  };

  //! NullObjective is used when no objective functions have been given to the solver (ie, for pure satisfaction runs). 
  template <typename TypeVariable>
  class NullObjective : public Objective<TypeVariable>
  {
    using Objective<TypeVariable>::random;
    
  public:
    NullObjective() : Objective<TypeVariable>("nullObjective") { }

  private:
    double required_cost( vector< TypeVariable > *variables ) const override { return 0.; }

    int expert_heuristic_value( vector< TypeVariable > *variables,
				TypeVariable *var,
				const vector< int >& valuesList ) const override
    {
      return valuesList[ random.get_random_number( valuesList.size() ) ];
    }
  };

  ////////////////////
  // Implementation //
  ////////////////////
  
  template <typename TypeVariable>
  Objective<TypeVariable>::Objective( const string& name )
    : name(name)
  { }
  
  template <typename TypeVariable>
  TypeVariable* Objective<TypeVariable>::expert_heuristic_variable( vector< TypeVariable* > *variables ) const
  {
    return (*variables)[ random.get_random_number( variables->size() ) ];
  }
  
  template <typename TypeVariable>
  int Objective<TypeVariable>::expert_heuristic_value( vector< TypeVariable > *variables,
						       TypeVariable *var,
						       const vector< int >& possible_values ) const
  {
    double minCost = numeric_limits<double>::max();
    double simulatedCost;
    
    int backup = var->get_value();
    vector<int> bestValues;
    
    for( auto& v : possible_values )
    {
      var->set_value( v );
      simulatedCost = cost( variables );
      
      if( minCost > simulatedCost )
      {
	minCost = simulatedCost;
	bestValues.clear();
	bestValues.push_back( v );
      }
      else
	if( minCost == simulatedCost )
	  bestValues.push_back( v );
    }
    
    var->set_value( backup );
    
    return bestValues[ random.get_random_number( bestValues.size() ) ];
  }
  
  template <typename TypeVariable>
  TypeVariable* Objective<TypeVariable>::expert_heuristic_value( vector< TypeVariable* > *bad_variables ) const
  {
    return (*bad_variables)[ random.get_random_number( bad_variables->size() ) ];
  } 
}
