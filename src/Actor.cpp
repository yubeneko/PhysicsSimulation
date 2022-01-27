#include "Actor.h"
#include "Component.h"

Actor::Actor(Core& core, const std::string& id) : mCore(core), mID(id) {}

Actor::~Actor()
{
}

std::shared_ptr<Actor> Actor::Create(
	class Core& core,
	const std::string& id)
{
	return std::shared_ptr<Actor>(new Actor(core, id));
}

void Actor::Update(float deltaTime)
{
	if (mState == ActorState::EActive)
	{
		ComputeWorldTransform();

		// コンポーネントの更新
		for (auto comp : mComponents)
		{
			if (comp->GetEnabled())
			{
				comp->Update(deltaTime);
			}
		}

		ComputeWorldTransform();
	}
}

void Actor::ComputeWorldTransform()
{
	if (mShouldRecomputeWorldTransform)
	{
		mShouldRecomputeWorldTransform = false;
		// glm::mat4 T = glm::translate(glm::mat4(1.0f), mPosition);
		// glm::mat4 TR = T * glm::toMat4(mRotation);
		// mWorldTransform = glm::scale(TR, mScale);

		glm::mat4 T = glm::translate(glm::mat4(1.0f), mPosition);
		glm::mat4 R = glm::toMat4(mRotation);
		glm::mat4 S = glm::scale(glm::mat4(1.0f), mScale);
		mWorldTransform = T * R * S;

		for (auto comp : mComponents)
		{
			comp->OnUpdateWorldTransform();
		}
	}
}

void Actor::ProcessInput(const InputState& state)
{
	for (auto comp : mComponents)
	{
		if (comp->GetEnabled())
		{
			comp->ProcessInput(state);
		}
	}
}

void Actor::Destroy()
{
	mIsDead = true;
}