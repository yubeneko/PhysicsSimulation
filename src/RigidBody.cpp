#include "RigidBody.h"
#include "Core.h"
#include "Actor.h"
#include "PhysicsWorld.h"

std::shared_ptr<RigidBody> RigidBody::CreateComponent(
	std::weak_ptr<Actor> owner,
	int updateOrder)
{
	auto rb = std::shared_ptr<RigidBody>(new RigidBody(owner, updateOrder));
	rb->mID = owner.lock()->GetCore().GetPhysicsWorld().AddRigidbody(*rb);
	return rb;
}

RigidBody::RigidBody(std::weak_ptr<Actor> owner, int updateOrder)
  : Component(owner, updateOrder),
	mPhysicsWorld(owner.lock()->GetCore().GetPhysicsWorld())
{
}

void RigidBody::SetMotionType(SimplePhysics::SpxMotionType type)
{
	mPhysicsWorld.SetMotionType(mID, type);
}

void RigidBody::ApplyImpulse(glm::vec3 velocity)
{
	mPhysicsWorld.ApplyImpulse(mID, velocity);
}

void RigidBody::Update(float deltaTime)
{
	const SimplePhysics::SpxState& state = mPhysicsWorld.GetState(mID);
	const SimplePhysics::SpxCollidable& collidable = mPhysicsWorld.GetCollidable(mID);

	// 今のところ、剛体の数は1つだけとする
	glm::mat4x3 rigidBodyTransform = GLMExtension::To3x4TransformMat(state.m_orientation, state.m_position);

	const SimplePhysics::SpxShape& shape = collidable.m_shapes[0];
	glm::mat4x3 shapeTransform = GLMExtension::To3x4TransformMat(shape.m_offsetQuaternion, shape.m_offsetPosition);
	glm::mat4x3 worldTransform = GLMExtension::AffineTransformMultiply(rigidBodyTransform, shapeTransform);

	auto owner = mOwner.lock();
	owner->SetPosition(GLMExtension::GetTranslation(worldTransform));
	owner->SetRotation(glm::quat(glm::mat3(worldTransform)));
}