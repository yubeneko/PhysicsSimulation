#include "CubeShooter.h"
#include "InputSystem.h"
#include "PresetActor.h"
#include "RigidBody.h"
#include "MeshComponent.h"
#include "Actor.h"

#include <SDL_scancode.h>

std::shared_ptr<CubeShooter> CubeShooter::CreateComponent(
	std::weak_ptr<Actor> owner,
	int updateOrder)
{
	return std::shared_ptr<CubeShooter>(new CubeShooter(owner, updateOrder));
}

CubeShooter::CubeShooter(std::weak_ptr<Actor> owner,
						 int updateOrder)
  : Component(owner, updateOrder)
{
}

void CubeShooter::ProcessInput(const struct InputState& state)
{
	if (state.keyboard.GetKeyDown(SDL_SCANCODE_SPACE))
	{
		auto cube = PresetActor::CreatePreset(
			mOwner.lock()->GetCore(), "cube", PresetActor::PresetType::Cube);
		cube->SetPosition(mOwner.lock()->GetPosition());
		cube->SetScale(glm::vec3(0.5f));

		auto meshComponent = cube->GetComponent<MeshComponent>();
		MeshComponent::Material& mat = meshComponent.lock()->GetMaterial();
		mat.mAmbient = glm::vec3(0.9f, 0.1f, 0.9f);
		mat.mDiffuse = glm::vec3(0.9f, 0.1f, 0.9f);
		mat.mSpecular = glm::vec3(0.9f, 0.9f, 0.9f);
		mat.mSmoothness = 100.0f;

		auto rb = cube->AddComponent<RigidBody>();
	}
}