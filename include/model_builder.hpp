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
#include <memory>

#include "model.hpp"

namespace ghost
{
	/*!
	 * This is the base class from which users need to derive their ModelBuilder class. 
	 *
	 * ghost::ModelBuilder cannot be directly used to encode user-defined model builder,
	 * since this is an abstract class. Users need to make their own derived model builder.
	 *
	 * Once users have written their own Constraint class(es), and eventually an Objective
	 * class and an AuxiliaryData class, they need to declare what their combinatorial
	 * problem is by:
	 * - declaring what the variables of the problem are, and what is their associated
	 * domain, i.e., what is the set of values each variable can take.
	 * - declaring what the constraints of the problem are, and what variables are in
	 * their scope.
	 * - for optimization problems, declaring what is the objective function to minimize or
	 * maximize.
	 * - eventually, declaring some auxiliary data to keep updated user-defined data
	 * structures while the solver is changing the value of variables.
	 *
	 * A user-defined ModelBuilder class is here to declare all elements above composing
	 * a problem instance.
	 *
	 * \sa Variable Constraint Objective AuxiliaryData
	 */
	class ModelBuilder
	{
		template<typename ModelBuilderType> friend class Solver;

		Model build_model();

	protected:
		std::vector<Variable> variables; //!< The global vector containing all variables of the problem instance.
		std::vector<std::shared_ptr<Constraint>> constraints; //!< The vector of shared pointers of each constraint composing the problem instance.
		std::shared_ptr<Objective> objective; //!< The shared pointer of the objective function of the problem instance. Is set to nullptr is declare_objective() is not overriden.
		std::shared_ptr<AuxiliaryData> auxiliary_data; //!< The shared pointer of the auxiliary data of the problem instance. Is set to nullptr is declare_auxiliary_data() is not overriden.
		bool permutation_problem;
		
	public:
		/*!
		 * Unique constructor.
		 *
		 * \param permutation_problem a Boolean to declare if the problem is a permutation problem. False by default.
		 */
		ModelBuilder( bool permutation_problem = false );
		
		//! Default virtual destructor.
		virtual ~ModelBuilder() = default;

		/*!
		 * Mandatory method to declare the variables of the problem instance.
		 *
		 * The implementation should be like\n
		 * void UserBuilder::declare_variables()\n
		 * {\n
		 *   variables.emplace_back(parameters_of_Variable_constructor);\n
		 *   variables.emplace_back(parameters_of_Variable_constructor);\n
		 *   ...\n
		 *   variables.emplace_back(parameters_of_Variable_constructor);\n
		 * }\n
		 *
		 * Alternatively, if the problem has many variables with similar domains with 
		 * all integers in [first_value_domain, domain_size - 1], users can declare
		 * several variables at once:\n
		 * void UserBuilder::declare_variables()\n
		 * {\n
		 *   create_n_variables( number_of_variables, first_value_domain, domain_size);\n
		 * }
		 */
		virtual void declare_variables() = 0;

		/*!
		 * Mandatory method to declare the constraints of the problem instance.
		 *
		 * The implementation should be like\n
		 * void UserBuilder::declare_constraints()\n
		 * {\n
		 *   constraints.push_back( std::make_shared<UserConstraint-1>(parameters_of_Constraint-1_constructor ) );\n
		 *   constraints.push_back( std::make_shared<UserConstraint-1>(parameters_of_Constraint-1_constructor ) ); // the model may need several constraints of the same type UserConstraint-1\n
		 *   ...\n
		 *   constraints.push_back( std::make_shared<UserConstraint-k>(parameters_of_Constraint-k_constructor ) );\n
		 * }
		 */
		virtual void declare_constraints() = 0;

		/*!
		 * If working with an optimization problem, mandatory method to declare the objective function
		 * of the problem instance.
		 *
		 * No need to override this method for decision problems (CSP and EFSP models).
		 * For optimization problems (COP and EFOP models), the implementation should be like\n
		 * void UserBuilder::declare_objective()\n
		 * {\n
		 *   objective = std::make_shared<UserObjective>(parameters_of_UserObjective_constructor);\n
		 * }
		 */
		virtual void declare_objective();

		/*!
		 * Method to declare the auxiliary data of the problem instance.
		 *
		 * No need to override this method if the problem don't need auxiliary data.
		 * Otherwise, the implementation should be like\n
		 * void UserBuilder::declare_auxiliary_data()\n
		 * {\n
		 *   auxiliary_data = std::make_shared<UserData>(parameters_of_UserData_constructor);\n
		 * }
		 */
		virtual void declare_auxiliary_data();

		/*!
		 * Method to create 'number' identical variables, all with a domain given as input.
		 *
		 * \param number the number of variables to create.
		 * \param domain the domain to copy and give to each variable.
		 * \param index an optional parameter to make variables starting with the index-th value
		 * of the domain. Set to 0 by default.
		 */
		void create_n_variables( int number, const std::vector<int>& domain, int index = 0 );

		/*!
		 * Method to create 'number' identical variables, all with a domain containing all integers
		 * in [starting_value, starting_value + size - 1].
		 *
		 * \param number the number of variables to create.
		 * \param starting_value the first value of each domain.
		 * \param size the size of each domain.
		 * \param index an optional parameter to make variables starting with the index-th value
		 * of the domain. Set to 0 by default.
		 */
		void create_n_variables( int number, int starting_value, std::size_t size, int index = 0 );

		/*!
		 * Inline method returning the number of declared variables. This may be helpful in some
		 * specific cases to know how variables are composing the problem instance.
		 */
		inline int get_number_variables() { return static_cast<int>( variables.size() ); }
	};
}
