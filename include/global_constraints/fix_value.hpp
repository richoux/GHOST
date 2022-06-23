#pragma once

#include <vector>

#include "../variable.hpp"
#include "../constraint.hpp"

namespace ghost
{
	namespace global_constraints
	{

		class FixValue : public Constraint
		{
			int _value;
			mutable int _current_diff;

			double required_error( const std::vector<Variable*>& variables ) const override;
			double optional_delta_error( const std::vector<Variable*>& variables,
			                             const std::vector<int>& variable_indexes,
			                             const std::vector<int>& candidate_values ) const override;
	
		public:
			FixValue( const std::vector<int>& variables_index, int value );
		};
	}
}
