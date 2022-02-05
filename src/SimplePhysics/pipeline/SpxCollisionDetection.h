#pragma once

#include "../SpxBase.h"
#include "../elements/SpxState.h"
#include "../elements/SpxCollidable.h"
#include "../elements/SpxPair.h"

namespace SimplePhysics
{
	/**
	 * @brief 衝突検出のナローフェーズ
	 *
	 * @param states 剛体の状態の配列
	 * @param collidables 剛体の形状の配列
	 * @param numRigidBodies 剛体の数
	 * @param pairs ペア配列
	 * @param numPairs ペア数
	 */
	void SpxDetectCollision(
		const SpxState* states,
		const SpxCollidable* collidables,
		SpxUInt32 numRigidBodies,
		const SpxPair* pairs,
		SpxUInt32 numPairs);
};	// namespace SimplePhysics