#pragma once

#include "../SpxBase.h"
#include "../elements/SpxState.h"
#include "../elements/SpxRigidBody.h"

namespace SimplePhysics
{

// 最大並進速度
const float SPX_MAX_LINEAR_VELOCITY = 340.0f;
// 最大角速度
constexpr double SPX_MAX_ANGULAR_VELOCITY()
{
	return glm::pi<double>() * 60.0;
};

/**
 * @brief 剛体に外力を与える
 *
 * @param state 剛体の状態
 * @param body 剛体の属性
 * @param externalForce 与える力
 * @param externalTorque 与えるトルク
 * @param timeStep タイムステップ
 */
void SpxApplyExternalForce(
	SpxState& state,
	const SpxRigidBody& body,
	const glm::vec3& externalForce,
	const glm::vec3& externalTorque,
	float timeStep);

/**
 * @brief ソルバーの演算の結果を剛体の状態に適用
 *
 * @param states 剛体の状態の配列
 * @param numRigidBodies 剛体の数
 * @param timeStep タイムステップ
 */
void SpxIntegrate(
	SpxState* states,
	SpxUInt32 numRigidBodies,
	float timeStep);

};	// namespace SimplePhysics