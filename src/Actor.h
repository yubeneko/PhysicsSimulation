#pragma once

#include <memory>
#include <string>
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>

class Actor final : public std::enable_shared_from_this<Actor>
{
public:
	enum ActorState
	{
		EActive,
		EPaused,
	};

	~Actor();

	// Actor のファクトリメソッド
	static std::shared_ptr<Actor> Create(
		class Core& core,
		const std::string& id);

	void Update(float deltaTime);
	void ComputeWorldTransform();
	void ProcessInput(const struct InputState& state);
	void Destroy();

	template <typename T>
	std::shared_ptr<T> AddComponent(int updateOrder = 100);

	template <typename T>
	std::weak_ptr<T> GetComponent();

	// Getter/Setter

	class Core& GetCore() const { return mCore; }

	std::string GetID() const { return mID; }
	void SetID(std::string id) { mID = id; }

	std::string GetTag() const { return mTag; }
	void SetTag(std::string tag) { mTag = tag; }

	glm::vec3 GetPosition() const { return mPosition; }
	void SetPosition(const glm::vec3& pos)
	{
		mShouldRecomputeWorldTransform = true;
		mPosition = pos;
	}

	glm::vec3 GetScale() const { return mScale; }
	void SetScale(const glm::vec3& scale)
	{
		mShouldRecomputeWorldTransform = true;
		mScale = scale;
	}

	glm::quat GetRotation() const { return mRotation; }
	void SetRotation(const glm::quat& q)
	{
		mShouldRecomputeWorldTransform = true;
		mRotation = q;
	}

	ActorState GetState() const { return mState; }
	void SetState(ActorState state) { mState = state; }

	bool IsDead() const { return mIsDead; }

	const glm::mat4 GetWorldTransform() const { return mWorldTransform; }

	// 前方ベクトルを取得する
	glm::vec3 GetForward() const { return mRotation * glm::vec3(0.0f, 0.0f, 1.0f); }
	glm::vec3 GetRight() const { return mRotation * glm::vec3(1.0f, 0.0f, 0.0f); }
	glm::vec3 GetUp() const { return mRotation * glm::vec3(0.0f, 1.0f, 0.0f); }

private:
	Actor(class Core& core, const std::string& id);

	class Core& mCore;
	std::string mID;
	std::string mTag;

	glm::vec3 mPosition{0.0f, 0.0f, 0.0f};
	glm::vec3 mScale{1.0f, 1.0f, 1.0f};
	glm::quat mRotation = glm::identity<glm::quat>();
	glm::mat4 mWorldTransform;

	ActorState mState = ActorState::EActive;
	bool mIsDead = false;

	std::vector<std::shared_ptr<class Component>> mComponents;
	bool mShouldRecomputeWorldTransform = true;
};

#include "ActorPrivate.h"