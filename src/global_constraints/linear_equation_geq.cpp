/*
 * GHOST (General meta-Heuristic Optimization Solving Tool) is a C++ framework
 * designed to help developers to model and implement optimization problem
 * solving. It contains a meta-heuristic solver aiming to solve any kind of
 * combinatorial and optimization real-time problems represented by a CSP/COP/EF-CSP/EF-COP. 
 *
 * First developed to solve game-related optimization problems, GHOST can be used for
 * any kind of applications where solving combinatorial and optimization problems. In
 * particular, it had been designed to be able to solve not-too-complex problem instances
 * within some milliseconds, making it very suitable for highly reactive or embedded systems.
 * Please visit https://github.com/richoux/GHOST for further information.
 *
 * Copyright (C) 2014-2025 Florian Richoux
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

#include <cmath>
#include <algorithm>
#include <iostream>

#include "global_constraints/linear_equation_geq.hpp"

using ghost::global_constraints::LinearEquationGeq;

LinearEquationGeq::LinearEquationGeq( const std::vector<int>& variables_index, double rhs, const std::vector<double>& coefficients )
	: LinearEquation( variables_index, rhs, coefficients )
{ }

LinearEquationGeq::LinearEquationGeq( const std::vector<int>& variables_index, double rhs )
	: LinearEquation( variables_index, rhs, std::vector<double>( variables_index.size(), 1.0 ) )
{ }

LinearEquationGeq::LinearEquationGeq( const std::vector<Variable>& variables, double rhs, const std::vector<double>& coefficients )
	: LinearEquation( variables, rhs, coefficients )
{ }

LinearEquationGeq::LinearEquationGeq( const std::vector<Variable>& variables, double rhs )
	: LinearEquation( variables, rhs, std::vector<double>( variables.size(), 1.0 ) )
{ }

double LinearEquationGeq::compute_error( double sum ) const
{
	return std::max( 0.0, rhs - sum );
}
