#pragma once

#include "Component.h"
#include "PhysicsWorld.h"

class RigidBody : public Component, public std::enable_shared_from_this<RigidBody>
{
public:
	static std::shared_ptr<RigidBody> CreateComponent(
		std::weak_ptr<class Actor> owner,
		int updateOrder);

	void SetMotionType(SimplePhysics::SpxMotionType type);
	/**
	 * @brief 激力を与える。(速度を変更する)
	 * 
	 * @param velocity 速度
	 */
	void ApplyImpulse(glm::vec3 velocity);

private:
	void Update(float deltaTime) override;
	RigidBody(std::weak_ptr<class Actor> owner, int updateOrder);
	int mID;
	class PhysicsWorld& mPhysicsWorld;
};