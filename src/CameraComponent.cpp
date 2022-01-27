#include "CameraComponent.h"
#include "Actor.h"
#include "Core.h"
#include "Renderer.h"

#include <glm/glm.hpp>
#include <glm/ext/matrix_transform.hpp>

CameraComponent::CameraComponent(
	std::weak_ptr<class Actor> owner,
	int updateOrder)
  : Component(owner, updateOrder),
	mRenderer(owner.lock()->GetCore().GetRenderer())
{
}

std::shared_ptr<CameraComponent> CameraComponent::CreateComponent(
	std::weak_ptr<class Actor> owner,
	int updateOrder)
{
	return std::shared_ptr<CameraComponent>(new CameraComponent(owner, updateOrder));
}

void CameraComponent::Update(float deltaTime)
{
	auto owner = mOwner.lock();
	glm::vec3 cameraPos = owner->GetPosition();

	glm::mat4 view = glm::lookAt(
		cameraPos,								   // カメラの座標
		cameraPos + owner->GetForward() * 100.0f,  // 向いている方向
		glm::vec3(0, 1, 0));					   // 上方向

	mRenderer.SetViewMatrix(view);
	mRenderer.SetCameraPos(cameraPos);
}