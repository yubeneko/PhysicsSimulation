#pragma once

#include "Component.h"

class CameraMove : public Component
{
public:
	static std::shared_ptr<CameraMove> CreateComponent(
		std::weak_ptr<class Actor> owner,
		int updateOrder);

	void ProcessInput(const struct InputState& state) override;
	void Update(float deltaTime) override;

private:
	CameraMove(std::weak_ptr<class Actor> owner, int updateOrder);

	float mForwardSpeed = 0.0f;
	float mAngularSpeedY = 0.0f;
	float mUpSpeed = 0.0f;
};