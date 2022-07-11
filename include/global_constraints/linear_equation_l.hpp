/*
 * GHOST (General meta-Heuristic Optimization Solving Tool) is a C++ framework
 * designed to help developers to model and implement optimization problem
 * solving. It contains a meta-heuristic solver aiming to solve any kind of
 * combinatorial and optimization real-time problems represented by a CSP/COP/EFSP/EFOP. 
 *
 * First developped to solve game-related optimization problems, GHOST can be used for
 * any kind of applications where solving combinatorial and optimization problems. In
 * particular, it had been designed to be able to solve not-too-complex problem instances
 * within some milliseconds, making it very suitable for highly reactive or embedded systems.
 * Please visit https://github.com/richoux/GHOST for further information.
 *
 * Copyright (C) 2014-2022 Florian Richoux
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

#include "linear_equation.hpp"

namespace ghost
{
	namespace global_constraints
	{
		/*!
		 * Implementation of the Linear Equation constraint with less than, i.e., the Scalar Product constraint with the atom '<'.
		 * See http://sofdem.github.io/gccat/gccat/Cscalar_product.html
		 */
		class LinearEquationL : public LinearEquation
		{
			double compute_error( double sum ) const override;

		public:
			/*!
			 * Constructor with a vector of variable IDs. This vector is internally used by ghost::Constraint
			 * to know what variables from the global variable vector it is handling.
			 * \param variables_index a const reference to a vector of IDs of variables composing the constraint.
			 * \param rhs the right-hand side real number of the equation.
			 * \param coefficients the vector of real numbers coefficients for each variable of the equation.
			 */
			LinearEquationL( const std::vector<int>& variables_index, double rhs, const std::vector<double>& coefficients );

			/*!
			 * Constructor with a vector of variable IDs. This constructor calls LinearEquationL( const std::vector<int>& index, double rhs, const std::vector<double>& coefficients), with all coefficients set to 1.0.
			 * \param variables_index a const reference to a vector of IDs of variables composing the constraint.
			 * \param rhs the right-hand side real number of the equation.
			 */
			LinearEquationL( const std::vector<int>& variables_index, double rhs );

			/*!
			 * Constructor with a vector of variable.
			 * \param variables a const reference to a vector of variables composing the constraint.
			 * \param rhs the right-hand side real number of the equation.
			 * \param coefficients the vector of real numbers coefficients for each variable of the equation.
			 */
			LinearEquationL( const std::vector<Variable>& variables, double rhs, const std::vector<double>& coefficients );

			/*!
			 * Constructor with a vector of variable. This constructor calls LinearEquationL( const std::vector<Variable>& variables, double rhs, const std::vector<double>& coefficients), with all coefficients set to 1.0.
			 * \param variables a const reference to a vector of variables composing the constraint.
			 * \param rhs the right-hand side real number of the equation.
			 */
			LinearEquationL( const std::vector<Variable>& variables, double rhs );
};
	}
}
