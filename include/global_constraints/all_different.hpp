#pragma once

#include <vector>

#include "../variable.hpp"
#include "../constraint.hpp"

namespace ghost
{
	namespace global_constraints
	{
		class AllDifferent : public Constraint
		{
			mutable std::vector<int> _count;
			
			double required_error( const std::vector<Variable*>& variables ) const override;
			
			double optional_delta_error( const std::vector<Variable*>& variables,
			                             const std::vector<int>& variable_indexes,
			                             const std::vector<int>& candidate_values ) const override;
			
			void conditional_update_data_structures( const std::vector<Variable*>& variables,
			                                         int variable_index,
			                                         int new_value ) override;
			
		public:
			AllDifferent( const std::vector<int>& variables_index );
		};
	}
}
