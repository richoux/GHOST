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
 * Copyright (C) 2014-2024 Florian Richoux
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
#include <vector>
#include "error_projection_algorithm.hpp"
#include "space.hpp"
#include "../search_unit_data.hpp"

namespace ghost
{
	namespace algorithms
	{
		/*
		 * Strategy design pattern to implement different search space management policies.
		 */
		class SpacePolicy
		{
		protected:
			std::string name;
			std::unique_ptr<algorithms::ErrorProjection> error_projection;
			std::vector< std::unique_ptr<algorithms::Space> > space_pool; // To avoid re-creating spaces each time we switch
			bool switch_space_instead_reset; // true iff the current policy applies space switching instead of resets.
			int index_space_pool;
			
		public:
			SpacePolicy( std::string&& name,
			             std::unique_ptr<algorithms::ErrorProjection> error_projection,
									 bool switch_space_instead_reset,
			             int index_space_pool = 0 );

			SpacePolicy( std::string&& name,
									 bool switch_space_instead_reset );

			virtual ~SpacePolicy() = default;

			inline std::string get_name() const { return name; }
			inline std::string get_current_space_name() const { return space_pool[index_space_pool]->get_name(); }
			
			inline bool is_violation_space() const { return space_pool[index_space_pool]->is_violation_space(); }
			inline bool does_switch_space_instead_reset() const { return switch_space_instead_reset; }
						
			inline void set_error_projection( std::unique_ptr<algorithms::ErrorProjection> ep )
			{
				error_projection = std::move( ep );
			}

			inline std::string get_error_projection_name() const
			{
				return error_projection->get_name();
			}

			inline double get_fitness_variation( const SearchUnitData& data ) const
			{
				return space_pool[index_space_pool]->get_fitness_variation( data );
			}

			inline void update_fitness( const SearchUnitData& data ) const
			{
				space_pool[index_space_pool]->update_fitness( data );
			}
			
			// Can be useful to initialize some data structures before computing error projections.
			void initialize_data_structures( const SearchUnitData& data ) const;

			// Will reset data.error_variables and set the element of this vector to their projected cost
			void compute_variable_errors( const std::vector<Variable>& variables,
			                              const std::vector<std::shared_ptr<Constraint>>& constraints,
			                              SearchUnitData& data ) const;

			/*
			 * Procedure to switch space for TWM policies.
			 * Do nothing by default.
			 */
			virtual void switch_space();
			
			/*
			 * Procedure updating constraints and variables errors in data, when a local move is applied.
			 * \param variable_to_change The index of the variable currently selected by the search algorithm.
			 * \param new_value The new value assigned to the variable at the index variable_to_change.
			 * \param data A reference to the SearchUnitData object containing data about the problem instance and the search state, such as the number of variables and constraints, the current projected error on each variable, etc.
			 * \param model A reference to the problem model, to get access to the objective function for instance.
			 * \param error_projection A shared pointer to an error projection algorithm.
			 * \sa ErrorProjection
			 */
			virtual void update_errors( int variable_to_change,
			                            int new_value,
			                            SearchUnitData& data,
			                            const Model& model ) const = 0;

			// /*
			//  * TODO
			//  * \return true iff there is space switching.
			//  */
			// virtual bool local_minimum_management( int variable_to_change,
			//                                        SearchUnitData& data,
			//                                        int tabu_time_local_min,
			//                                        bool no_other_variables_to_try ) = 0;

			// /*
			//  * TODO
			//  * \return true iff there is space switching.
			//  */
			// virtual bool plateau_management( int variable_to_change,
			// 																 int new_value,
			// 																 SearchUnitData& data,
			// 																 const Options& options ) = 0;

			// /*
			//  * TODO
			//  * \return true iff there is space switching.
			//  */
			// virtual bool reset_management( SearchUnitData& data,
			// 															 const Options& options,
			// 															 const Model& model ) = 0;
		};
	}
}
