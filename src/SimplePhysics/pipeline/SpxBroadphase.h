#pragma once

#include "../SpxBase.h"
#include "../elements/SpxState.h"
#include "../elements/SpxCollidable.h"
#include "../elements/SpxPair.h"
#include "SpxAllocator.h"

#include <functional>

namespace SimplePhysics
{
	/**
	 * @brief ブロードフェーズのコールバック
	 *
	 */
	using SpxBroadPhaseCallback = bool (*)(SpxUInt32, SpxUInt32, void*);

	/**
	 * @brief ブロードフェーズ
	 *
	 * @param state 剛体の状態の配列
	 * @param collidables 剛体の形状の配列
	 * @param numRigidBodies 剛体の数
	 * @param oldPairs 前のフレームのペア
	 * @param numOldPairs 前のフレームのペア数
	 * @param[out] newPairs 新規に検出されたペア
	 * @param[out] numNewPairs 新規に検出されたペア数
	 * @param maxPairs 検出ペアの最大数
	 * @param allocator アロケータ
	 * @param userData コールバック時に渡されるユーザーデータ
	 * @param callback コールバック
	 */
	void SpxBroadPhase(
		const SpxState* states,
		const SpxCollidable* collidables,
		SpxUInt32 numRigidBodies,
		const SpxPair* oldPairs,
		const SpxUInt32 numOldPairs,
		SpxPair* newPairs,
		SpxUInt32& numNewPairs,
		const SpxUInt32 maxPairs,
		SpxAllocator* allocator,
		void* userData,
		SpxBroadPhaseCallback callback = nullptr);

};	// namespace SimplePhysics