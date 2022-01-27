#include "CubeGenerator.h"
#include "InputSystem.h"
#include "PresetActor.h"
#include "Actor.h"
#include <string>
#include <SDL_scancode.h>

CubeGenerator::CubeGenerator(std::weak_ptr<Actor> owner, int updateOrder)
  : Component(owner, updateOrder)
{
}

std::shared_ptr<CubeGenerator> CubeGenerator::CreateComponent(
	std::weak_ptr<class Actor> owner,
	int updateOrder)
{
	return std::shared_ptr<CubeGenerator>(new CubeGenerator(owner, updateOrder));
}

void CubeGenerator::ProcessInput(const InputState& state)
{
	if (state.keyboard.GetKeyDown(SDL_SCANCODE_SPACE))
	{
		auto cube = PresetActor::CreatePreset(
			mOwner.lock()->GetCore(),
			"Cube" + std::to_string(mCount),
			PresetActor::PresetType::Cube);

		cube->SetPosition(mGenPosition);
		++mCount;
		mGenPosition.x += 1.5f;
		if (mCount % 5 == 0)
		{
			mGenPosition.x = -3.0f;
			mGenPosition.y += 1.5f;
		}

		mCubes.emplace_back(std::weak_ptr(cube));
	}

	if (state.keyboard.GetKeyDown(SDL_SCANCODE_LCTRL))
	{
		if (!mCubes.empty())
		{
			auto cube = mCubes.back().lock();
			if (cube)
			{
				cube->Destroy();
				mCubes.pop_back();
			}
		}
	}
}