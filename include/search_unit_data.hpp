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

#include <vector>

#include "model.hpp"

namespace ghost
{
	/*
	 * SearchUnitData is the object containing inner data for search units.
	 */
	struct SearchUnitData
	{
		// General data about the curent model		
		int number_variables;
		int number_constraints;
		bool is_optimization;
		
		// Matrix to know which constraints contain a given variable
		// matrix_var_ctr[ variable_id ] = { constraint_id_1, ..., constraint_id_k }
		std::vector<std::vector<int> > matrix_var_ctr;

		// To know how many iterations each variable is still marked as tabu
		// tabu_list[2] = 3 --> variable with id=2 is marked tabu for the next 3 iterations of the search process
		// tabu_list[6] = 0 --> variable with id=6 is not marked as tabu (therefore, it is selectable during the search process)
		std::vector<int> tabu_list;

		// Variables about errors of the variables, and global satisfaction/optimization errors
		std::vector<double> error_variables;
		double best_sat_error;
		double best_opt_cost;
		double current_sat_error;
		double current_opt_cost;

		// Statistics about the current run
		int restarts;
		int resets;
		int local_moves;
		int search_iterations;
		int local_minimum;
		int plateau_moves;
		int plateau_local_minimum;

		SearchUnitData( const Model& model )
		: number_variables ( static_cast<int>( model.variables.size() ) ),
		number_constraints ( static_cast<int>( model.constraints.size() ) ),
		is_optimization( model.objective->is_optimization() ),
		matrix_var_ctr( std::vector<std::vector<int>>( number_variables ) ),
		tabu_list ( std::vector<int>( number_variables, 0 ) ),
		error_variables ( std::vector<double>( number_variables, 0.0 ) ),
		best_sat_error ( std::numeric_limits<double>::max() ),
		best_opt_cost ( std::numeric_limits<double>::max() ),
		current_sat_error ( std::numeric_limits<double>::max() ),
		current_opt_cost ( std::numeric_limits<double>::max() ),
		restarts ( 0 ),
		resets ( 0 ),
		local_moves ( 0 ),
		search_iterations ( 0 ),
		local_minimum ( 0 ),
		plateau_moves ( 0 ),
		plateau_local_minimum ( 0 )
		{ }

		void initialize_matrix( const Model& model )
		{
			// Save the id of each constraint where the current variable appears in.
			for( int variable_id = 0; variable_id < number_variables; ++variable_id )
				for( int constraint_id = 0; constraint_id < number_constraints; ++constraint_id )
					if( model.constraints[ constraint_id ]->has_variable( variable_id ) )
						matrix_var_ctr[ variable_id ].push_back( constraint_id );
		}
	};
}
