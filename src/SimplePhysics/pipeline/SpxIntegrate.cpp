#include "SpxIntegrate.h"

namespace SimplePhysics
{

void SpxApplyExternalForce(
	SpxState& state,
	const SpxRigidBody& body,
	const glm::vec3& externalForce,
	const glm::vec3& externalTorque,
	float timeStep)
{
	if (state.m_motionType == SpxMotionTypeStatic) { return; }

	// 剛体の姿勢
	glm::mat3 orientation = glm::toMat3(state.m_orientation);
	// 現在の姿勢における慣性テンソル
	glm::mat3 worldInertia = orientation * body.m_inertia * glm::transpose(orientation);
	glm::mat3 worldInertiaInv = orientation * glm::inverse(body.m_inertia) * glm::transpose(orientation);
	glm::vec3 angularMomentum = worldInertia * state.m_angularVelocity;

	// 速度/角速度の更新(オイラー陽解法)
	state.m_linearVelocity += externalForce / body.m_mass * timeStep;

	angularMomentum += externalTorque * timeStep;
	state.m_angularVelocity = worldInertiaInv * angularMomentum;

	// 力/トルクを適用した結果、速度/角速度が最大値を上回ってしまったら最大値に制限する

	float linVelSqr = glm::length2(state.m_linearVelocity);
	if (linVelSqr > (SPX_MAX_LINEAR_VELOCITY * SPX_MAX_LINEAR_VELOCITY))
	{
		state.m_linearVelocity = (state.m_linearVelocity / glm::sqrt(linVelSqr)) * SPX_MAX_LINEAR_VELOCITY;
	}

	float angVelSqr = glm::length2(state.m_angularVelocity);
	if (angVelSqr > (SPX_MAX_ANGULAR_VELOCITY() * SPX_MAX_ANGULAR_VELOCITY()))
	{
		state.m_angularVelocity = (state.m_angularVelocity / glm::sqrt(angVelSqr)) * static_cast<float>(SPX_MAX_ANGULAR_VELOCITY());
	}
}

void SpxIntegrate(
	SpxState* states,
	SpxUInt32 numRigidBodies,
	float timeStep)
{
	for (SpxUInt32 i = 0; i < numRigidBodies; i++)
	{
		SpxState& state = states[i];

		glm::quat dAng = glm::quat(0.0f, state.m_angularVelocity) * state.m_orientation * 0.5f;

		// 座標の更新
		state.m_position += state.m_linearVelocity * timeStep;
		// 姿勢の更新(クォータニオンの時間積分)
		state.m_orientation = normalize(state.m_orientation + dAng * timeStep);
	}
}

};	// namespace SimplePhysics