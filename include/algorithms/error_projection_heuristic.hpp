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
 * Copyright (C) 2014-2023 Florian Richoux
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

#include "../constraint.hpp"
#include "../variable.hpp"

namespace ghost
{
	namespace algorithms
	{
		class ErrorProjection
		{
		protected:
			std::string name;
			int number_variables;
			int number_constraints;

		public:
			ErrorProjection( std::string&& name )
				: name( std::move( name ) )
			{ }

			//! Default virtual destructor.
			virtual ~ErrorProjection() = default;

			inline std::string get_name() const { return name; }
			inline void set_number_variables( int num ) { number_variables = num ; }
			inline void set_number_constraints( int num ) { number_constraints = num ; }

			virtual void initialize_data_structures() {};

			virtual void compute_variable_errors( std::vector<double>& error_variables,
			                                      const std::vector<Variable>& variables,
			                                      const std::vector<std::vector<int>>& matrix_var_ctr,
			                                      const std::vector<std::shared_ptr<Constraint>>& constraints ) = 0;

			virtual void update_variable_errors( std::vector<double>& error_variables,
			                                     const std::vector<Variable>& variables,
			                                     const std::vector<std::vector<int>>& matrix_var_ctr,
			                                     std::shared_ptr<Constraint> constraint,
			                                     double delta ) = 0;
		};
	}
}
