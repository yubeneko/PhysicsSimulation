#include "Core.h"
#include "Renderer.h"
#include "InputSystem.h"
#include "Actor.h"
#include "SceneLoader.h"
#include "PhysicsWorld.h"

#include <algorithm>
#include <glm/glm.hpp>

Core::Core() {}

// Core.h にて 前方参照で Renderer を unique_ptr で持つために
// デストラクタを非インライン化する。
Core::~Core() = default;

bool Core::Initialize()
{
	if (SDL_Init(SDL_INIT_VIDEO) != 0)
	{
		SDL_Log("Unable to initialize SDL : %s", SDL_GetError());
		return false;
	}

	mInputSystem = std::make_unique<InputSystem>(*this);
	if (!mInputSystem->Initialize())
	{
		SDL_Log("Failed to initialize Input System");
		mInputSystem.reset();
		return false;
	}

	mRenderer = std::make_unique<Renderer>(*this);
	if (!mRenderer->Initialize(1024.0f, 768.0f))
	{
		SDL_Log("Failed to initialize renderer.");
		mRenderer.reset();
		return false;
	}

	mPhysicsWorld = std::make_unique<PhysicsWorld>();

	mTicksCount = SDL_GetTicks();

	SceneLoader::LoadScene(*this);

	return true;
}

void Core::RunLoop()
{
	while (mIsRunning)
	{
		ProcessInput();
		Update();
		GenerateOutput();
	}
}

void Core::Shutdown()
{
	if (mRenderer)
	{
		mRenderer->Shutdown();
	}

	if (mInputSystem)
	{
		mInputSystem->Shutdown();
	}

	SDL_Quit();
}

std::shared_ptr<Actor> Core::CreateActor(const std::string& id)
{
	auto actor = Actor::Create(*this, id);

	if (mUpdatingActors)
	{
		mPendingActors.emplace_back(actor);
	}
	else
	{
		mActors.emplace_back(actor);
	}

	return actor;
}

void Core::ProcessInput()
{
	mInputSystem->PrepareForUpdate();

	SDL_Event event;
	while (SDL_PollEvent(&event))
	{
		switch (event.type)
		{
			case SDL_QUIT:
				mIsRunning = false;
				break;

			case SDL_MOUSEWHEEL:
				mInputSystem->ProcessEvent(event);
				break;
		}
	}

	mInputSystem->Update();
	const InputState& state = mInputSystem->GetState();

	if (state.keyboard.GetKeyDown(SDL_SCANCODE_ESCAPE))
	{
		mIsRunning = false;
	}

	mUpdatingActors = true;
	for (auto actor : mActors)
	{
		actor->ProcessInput(state);
	}
	mUpdatingActors = false;
}

void Core::Update()
{
	// 前回のフレームから16ms経過するまで待機
	// つまり、目標のフレームレートは約60FPS
	while (!SDL_TICKS_PASSED(SDL_GetTicks(), mTicksCount + 16))
		;

	float deltaTime = (SDL_GetTicks() - mTicksCount) / 1000.0f;
	if (deltaTime > 0.05f) { deltaTime = 0.05f; }
	mTicksCount = SDL_GetTicks();

	mUpdatingActors = true;
	for (auto actor : mActors)
	{
		actor->Update(deltaTime);
	}
	mUpdatingActors = false;

	for (auto pending : mPendingActors)
	{
		pending->ComputeWorldTransform();
		mActors.emplace_back(pending);
	}
	mPendingActors.clear();

	// 剛体シミュレーションを実行
	// TODO: 並列処理化する
	mPhysicsWorld->Simulate();

	// 死亡したアクターをリストから除去
	auto tail = std::remove_if(
		mActors.begin(),
		mActors.end(),
		[](std::shared_ptr<Actor> actor) {
			return actor->IsDead();
		});

	mActors.erase(tail, mActors.end());
}

void Core::GenerateOutput()
{
	mRenderer->Draw();
}