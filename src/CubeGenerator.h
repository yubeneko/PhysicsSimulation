#pragma once

#include "Component.h"
#include <glm/glm.hpp>
#include <vector>

class CubeGenerator : public Component
{
public:
	static std::shared_ptr<CubeGenerator> CreateComponent(
		std::weak_ptr<class Actor> owner,
		int updateOrder);

	void ProcessInput(const struct InputState& state) override;

private:
	CubeGenerator(std::weak_ptr<class Actor> owner, int updateOrder);
	glm::vec3 mGenPosition{-3.0f, 3.0f, 3.0f};
	int mCount = 0;
	std::vector<std::weak_ptr<class Actor>> mCubes;
};