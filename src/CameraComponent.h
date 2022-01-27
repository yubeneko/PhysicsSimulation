#pragma once

#include "Component.h"

class CameraComponent : public Component
{
public:
	static std::shared_ptr<CameraComponent> CreateComponent(
		std::weak_ptr<class Actor> owner,
		int updateOrder);

	void Update(float deltaTime) override;

private:
	CameraComponent(
		std::weak_ptr<class Actor> owner,
		int updateOrder);

	class Renderer& mRenderer;
};