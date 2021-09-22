#pragma once

#include <vector>
#include <memory>
#include <ghost/model_builder.hpp>

class TutorialBuilder : public ghost::ModelBuilder
{
public:
	TutorialBuilder();

	void declare_variables() override;
	void declare_constraints() override;
	void declare_objective() override;
};
