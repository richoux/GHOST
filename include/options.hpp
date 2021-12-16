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
 * Copyright (C) 2014-2021 Florian Richoux
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

#include <memory>
#include <algorithm>

#include "print.hpp"

namespace ghost
{
	/*!
	 * Options is a structure containing all optional arguments for Solver::solve.
	 *
	 * \sa Print
	 */
	struct Options
	{
		bool custom_starting_point; //!< To force starting the search on a custom variables assignment.
		bool resume_search; //!< Allowing stop-and-resume computation.
		bool parallel_runs; //!< To enable parallel runs of the solver. Using all available physical cores if number_threads is not specified.
		int number_threads; //!< Number of threads the solver will use for the search.
		std::shared_ptr<Print> print; //!< Allowing custom solution print (by derivating a class from ghost::Print)
		int tabu_time_local_min; //!< Number of local moves a variable of a local minimum is marked tabu.
		int tabu_time_selected; //!< Number of local moves a selected variable is marked tabu.
		int percent_chance_escape_plateau; //!< Percentage of chance to espace a (1-dimension, ie, related to 1 variable) plateau rather than exploring it.
		int reset_threshold; //!< Number of variables marked as tabu required to trigger a reset.
		int restart_threshold; //!< Trigger a restart every 'restart_threshold' reset. Set to 0 to never trigger restarts.
		int number_variables_to_reset; //!< Number of variables to randomly change the value at each reset.
		int number_start_samplings; //!< Number of variable assignments the solver randomly draw, if custom_starting_point and resume_search are false.

		//! Unique constructor
		Options();

		//! Default destructor
		~Options() = default;

		//! Unique copy constructor
		Options( const Options& other );

		//! Unique move constructor
		Options( Options&& other );

		/*!
		 * Unique copy assign operator
		 *
		 * No move assign operator on purpose (yes, we violate the rule of 5 here).
		 */
		Options& operator=( Options other );
	};
}
