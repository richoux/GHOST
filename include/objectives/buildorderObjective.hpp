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
#include <memory>

#include "objective.hpp"
#include "../variables/action.hpp"
#include "../domains/buildorderDomain.hpp"

using namespace std;

namespace ghost
{
  /***********************/
  /* BuildOrderObjective */
  /***********************/
  class BuildOrderObjective : public Objective<Action, BuildOrderDomain>
  {
  public:
    BuildOrderObjective( const string &name );
    BuildOrderObjective( const string &name, const vector< pair<string, int> > &input, vector<Action> &variables );

  protected:
    virtual void v_setHelper( const Action &b, const vector< Action > *vecVariables, const BuildOrderDomain *domain );

    virtual double v_postprocessOptimization( vector< Action > *vecActions,
					      BuildOrderDomain *domain,
					      double &bestCost );

    struct Simulation
    {
      Simulation( Action *action,
		  string additionalAction,
		  int frame,
		  int stockMineral,
		  int stockGas,
		  double incomeMineralPerMinute,
		  double incomeGasPerMinute,
		  int supplyUsed,
		  int supplyCapacity )
	: action(action),
	  additionalAction(additionalAction),
	  frame(frame),
	  stockMineral(stockMineral),
	  stockGas(stockGas),
	  incomeMineralPerMinute(incomeMineralPerMinute),
	  incomeGasPerMinute(incomeGasPerMinute),
	  supplyUsed(supplyUsed),
	  supplyCapacity(supplyCapacity)
      { }
      
      Action	*action;
      string	additionalAction;
      int	frame;
      int	stockMineral;
      int	stockGas;
      double	incomeMineralPerMinute;
      double	incomeGasPerMinute;
      int	supplyUsed;
      int	supplyCapacity;
    };

    struct Goal
    {
      Goal( string fullName, string type, int toHave, int current )
	: fullName(fullName), type(type), toHave(toHave), current(current)
      { }
      
      string	fullName;
      string	type;
      int	toHave;
      int	current;
    };


    void makeVecVariables( const pair<string, int> &input, vector<Action> &variables, vector<Goal> &goals );
    void makeVecVariables( const Action &action, vector<Action> &variables, int count );

    
    vector<Simulation>	simulation;
    vector<Goal>	goals;
  };
  
  /***********/
  /* NoneObj */
  /***********/
  // class NoneObj : public BuildOrderObjective
  // {
  // public:
  //   NoneObj();

  // private:
  //   double v_cost( const vector< Action > *vecVariables, const BuildOrderDomain *domain ) const;
  //   int v_heuristicVariable( const vector< int > &vecId, const vector< Action > *vecVariables, BuildOrderDomain *domain );
  //   void v_setHelper( const Action &b, const vector< Action > *vecVariables, const BuildOrderDomain *domain );
  //   double v_postprocessOptimization( vector< Action > *vecVariables, BuildOrderDomain *domain, double &bestCost );
  // };

  /*******************/
  /* MakeSpanMinCost */
  /*******************/
  class MakeSpanMinCost : public BuildOrderObjective
  {
  public:
    MakeSpanMinCost();

  private:
    double v_cost( const vector< Action > *vecVariables, const BuildOrderDomain *domain ) const;
    int v_heuristicVariable( const vector< int > &vecId, const vector< Action > *vecVariables, BuildOrderDomain *domain );
    void v_setHelper( const Action &b, const vector< Action > *vecVariables, const BuildOrderDomain *domain );
    double v_postprocessOptimization( vector< Action > *vecVariables, BuildOrderDomain *domain, double &bestCost );
  };

  /*******************/
  /* MakeSpanMaxProd */
  /*******************/
  class MakeSpanMaxProd : public BuildOrderObjective
  {
  public:
    MakeSpanMaxProd();

  private:
    double v_cost( const vector< Action > *vecVariables, const BuildOrderDomain *domain ) const;
    int v_heuristicVariable( const vector< int > &vecId, const vector< Action > *vecVariables, BuildOrderDomain *domain );
    void v_setHelper( const Action &b, const vector< Action > *vecVariables, const BuildOrderDomain *domain );
    double v_postprocessOptimization( vector< Action > *vecVariables, BuildOrderDomain *domain, double &bestCost );
  };
}
