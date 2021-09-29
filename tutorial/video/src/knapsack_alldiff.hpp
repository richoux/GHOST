#pragma once

#include <vector>
#include <ghost/constraint.hpp>

class KSAllDiff : public ghost::Constraint
{

public:
	KSAllDiff( const std::vector<ghost::Variable>& variables );

	double required_error( const std::vector<ghost::Variable*>& variables ) const override;
};
