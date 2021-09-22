#pragma once

#include <vector>
#include <memory>
#include <ghost/model_builder.hpp>

class KSBuilder : public ghost::ModelBuilder
{
public:
	KSBuilder();

	void declare_variables() override;
	void declare_constraints() override;
	void declare_objective() override;
	void declare_auxiliary_data() override;
};
