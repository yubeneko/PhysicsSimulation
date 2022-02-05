#pragma once

#include "Component.h"

class CubeShooter : public Component
{
public:
	static std::shared_ptr<CubeShooter> CreateComponent(
		std::weak_ptr<class Actor> owner,
		int updateOrder);

	void ProcessInput(const struct InputState& state) override;

private:
	CubeShooter(std::weak_ptr<class Actor> owner,
				int updateOrder);
};