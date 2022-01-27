#include "CameraMove.h"
#include "Actor.h"
#include "InputSystem.h"

#include <SDL.h>

CameraMove::CameraMove(std::weak_ptr<Actor> owner, int updateOrder)
  : Component(owner, updateOrder)
{
}

std::shared_ptr<CameraMove> CameraMove::CreateComponent(
	std::weak_ptr<class Actor> owner,
	int updateOrder)
{
	return std::shared_ptr<CameraMove>(new CameraMove(owner, updateOrder));
}

void CameraMove::ProcessInput(const struct InputState& state)
{
	mForwardSpeed = 0.0f;
	mAngularSpeedY = 0.0f;
	mUpSpeed = 0.0f;

	if (state.keyboard.GetKeyValue(SDL_SCANCODE_W))
	{
		mForwardSpeed += 5.0f;
	}

	if (state.keyboard.GetKeyValue(SDL_SCANCODE_S))
	{
		mForwardSpeed -= 5.0f;
	}

	if (state.keyboard.GetKeyValue(SDL_SCANCODE_A))
	{
		mAngularSpeedY += 90.0f;
	}

	if (state.keyboard.GetKeyValue(SDL_SCANCODE_D))
	{
		mAngularSpeedY -= 90.0f;
	}

	if (state.keyboard.GetKeyValue(SDL_SCANCODE_UP))
	{
		mUpSpeed += 5.0f;
	}

	if (state.keyboard.GetKeyValue(SDL_SCANCODE_DOWN))
	{
		mUpSpeed -= 5.0f;
	}
}

void CameraMove::Update(float deltaTime)
{
	auto owner = mOwner.lock();
	glm::vec3 pos = owner->GetPosition();
	pos += owner->GetForward() * mForwardSpeed * deltaTime;
	pos += glm::vec3(0.0f, 1.0f, 0.0f) * mUpSpeed * deltaTime;
	owner->SetPosition(pos);

	float rotAngle = glm::radians(mAngularSpeedY * deltaTime);
	glm::quat rot = glm::angleAxis(rotAngle, glm::vec3(0.0f, 1.0f, 0.0f)) * owner->GetRotation();
	owner->SetRotation(rot);
}