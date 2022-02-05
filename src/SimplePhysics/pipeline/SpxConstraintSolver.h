#pragma once

#include "../SpxBase.h"
#include "../elements/SpxState.h"
#include "../elements/SpxRigidBody.h"
#include "../elements/SpxPair.h"
#include "../elements/SpxBallJoint.h"
#include "SpxAllocator.h"

namespace SimplePhysics
{

/**
 * @brief 拘束ソルバー
 *
 * @param states 剛体の状態の配列
 * @param bodies 剛体の属性の配列
 * @param numRigidBodies 剛体の数
 * @param pairs ペア配列
 * @param numPairs ペア数
 * @param joints ジョイント配列
 * @param numJoints ジョイント数
 * @param iteration 計算の反復回数
 * @param bias 位置補正のバイアス
 * @param slop 貫通許容誤差
 * @param timeStep タイムステップ
 * @param allocator アロケータ
 */
void SpxSolveConstraints(
	SpxState* states,
	const SpxRigidBody* bodies,
	SpxUInt32 numRigidBodies,
	const SpxPair* pairs,
	SpxUInt32 numPairs,
	SpxBallJoint* joints,
	SpxUInt32 numJoints,
	SpxUInt32 iteration,
	float bias,
	float slop,
	float timeStep,
	SpxAllocator* allocator);

};	// namespace SimplePhysics