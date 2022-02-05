#pragma once

#include <SDL.h>
#include <string>
#include <vector>
#include <memory>

class Core
{
public:
	Core();
	~Core();
	bool Initialize();
	void RunLoop();
	void Shutdown();
	std::string GetWindowName() const { return mWindowName; }
	class Renderer& GetRenderer() const { return *mRenderer; }
	class PhysicsWorld& GetPhysicsWorld() const { return *mPhysicsWorld; }

	std::shared_ptr<class Actor> CreateActor(const std::string& id);

private:
	void ProcessInput();
	void Update();
	void GenerateOutput();

	bool mIsRunning = true;
	Uint32 mTicksCount = 0;
	std::string mWindowName = "Physics Simulation";
	bool mUpdatingActors = false;

	std::unique_ptr<class Renderer> mRenderer;
	std::unique_ptr<class InputSystem> mInputSystem;
	std::unique_ptr<class PhysicsWorld> mPhysicsWorld;

	std::vector<std::shared_ptr<class Actor>> mActors;
	std::vector<std::shared_ptr<class Actor>> mPendingActors;
};