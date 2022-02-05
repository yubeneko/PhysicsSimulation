#include "SpxBroadphase.h"
#include "SpxSort.h"
#include "../glmExtension.h"

#include <string>
#include <sstream>

namespace SimplePhysics
{
const float SPX_AABB_EXPAND = 0.01f;

static inline bool SpxIntersectAABB(
	const glm::vec3& centerA,
	const glm::vec3& halfA,
	const glm::vec3& centerB,
	const glm::vec3& halfB)
{
	// 2つのAABBの中心間の距離(各々の軸にて)が2つのAABBのそれぞれの半分の距離を足した数値よりも大きいならば、
	// そのAABBはその軸において重なっていない。円の交差判定と同じ原理。
	if (glm::abs(centerA.x - centerB.x) > halfA.x + halfB.x) { return false; }
	if (glm::abs(centerA.y - centerB.y) > halfA.y + halfB.y) { return false; }
	if (glm::abs(centerA.z - centerB.z) > halfA.z + halfB.z) { return false; }

	// 全ての軸で重なっているならば2つのAABBは重なっている
	return true;
}

// コピペ修正
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
	SpxBroadPhaseCallback callback)
{
	assert(states);
	assert(collidables);
	assert(oldPairs);
	assert(newPairs);
	assert(allocator);

	numNewPairs = 0;

	// AABB交差ペアを見つける（総当たり）
	// 処理の内容を明確にするため、ここでは空間分割テクニックを使っていませんが、
	// 理論編で解説されているSweet and prune等の手法を使えば高速化できます。
	for (SpxUInt32 i = 0; i < numRigidBodies; i++)
	{
		for (SpxUInt32 j = i + 1; j < numRigidBodies; j++)
		{
			const SpxState& stateA = states[i];
			const SpxCollidable& collidableA = collidables[i];
			const SpxState& stateB = states[j];
			const SpxCollidable& collidableB = collidables[j];

			if (callback && !callback(i, j, userData))
			{
				continue;
			}

			// 2つの剛体のAABBを作成

			glm::mat3 orientationA(stateA.m_orientation);
			glm::vec3 centerA = stateA.m_position + orientationA * collidableA.m_center;
			glm::vec3 halfA = GLMExtension::AbsPerElem(orientationA) * (collidableA.m_half + glm::vec3(SPX_AABB_EXPAND));  // AABBサイズを若干拡張

			glm::mat3 orientationB(stateB.m_orientation);
			glm::vec3 centerB = stateB.m_position + orientationB * collidableB.m_center;
			glm::vec3 halfB = GLMExtension::AbsPerElem(orientationB) * (collidableB.m_half + glm::vec3(SPX_AABB_EXPAND));  // AABBサイズを若干拡張

			// 2つのAABBの衝突判定
			if (SpxIntersectAABB(centerA, halfA, centerB, halfB) && numNewPairs < maxPairs)
			{
				SpxPair& newPair = newPairs[numNewPairs++];

				// インデックスの登録の順番は重要。なぜなら、この2つの数値を元にして作られた数値で
				// ソートを行うため。

				newPair.rigidBodyA =
					i < j ? i : j;	// Aには小さい方をセット
				newPair.rigidBodyB =
					i < j ? j : i;	// Bには大きい方をセット
				newPair.contact = NULL;
			}
		}
	}

	// ソート
	{
		SpxPair* sortBuff = (SpxPair*)allocator->allocate(sizeof(SpxPair) * numNewPairs);
		SpxSort<SpxPair>(newPairs, sortBuff, numNewPairs);
		allocator->deallocate(sortBuff);
	}

	// 新しく検出したペアと過去のペアを比較
	SpxPair* outNewPairs = (SpxPair*)allocator->allocate(sizeof(SpxPair) * numNewPairs);
	SpxPair* outKeepPairs = (SpxPair*)allocator->allocate(sizeof(SpxPair) * numOldPairs);
	assert(outNewPairs);
	assert(outKeepPairs);

	SpxUInt32 nNew = 0;
	SpxUInt32 nKeep = 0;

	SpxUInt32 oldId = 0, newId = 0;

	// oldPairs と newPairs はどちらもユニークなキーでソートされている。
	// このキーはペアになっている2つの剛体のIDをもとに作られている。

	// 新しく検出したペアと過去のペアを順番に比較していく
	while (oldId < numOldPairs && newId < numNewPairs)
	{
		if (newPairs[newId].key > oldPairs[oldId].key)
		{
			// 新規に検出された key の方が前のキーよりも大きかった
			// ->
			// 前のフレームのそのインデックスにおけるペアはすでに衝突していない
			// remove
			allocator->deallocate(oldPairs[oldId].contact);
			oldId++;
		}
		else if (newPairs[newId].key == oldPairs[oldId].key) {
			// キーが同じ -> 前のフレームでもその2つのペアは衝突していた
			// keep
			assert(nKeep <= numOldPairs);
			outKeepPairs[nKeep] = oldPairs[oldId];
			nKeep++;
			oldId++;
			newId++;
		}
		else {
			// new
			assert(nNew <= numNewPairs);
			outNewPairs[nNew] = newPairs[newId];
			nNew++;
			newId++;
		}
	};

	// 片方の配列が空になったときの残りの方の配列の処理
	if (newId < numNewPairs)
	{
		// all new
		for (; newId < numNewPairs; newId++, nNew++)
		{
			assert(nNew <= numNewPairs);
			outNewPairs[nNew] = newPairs[newId];
		}
	}
	else if (oldId < numOldPairs) {
		// all remove
		for (; oldId < numOldPairs; oldId++)
		{
			allocator->deallocate(oldPairs[oldId].contact);
		}
	}

	// ~~~~~ 新規衝突ペアの衝突点の情報をリセット ~~~~~
	for (SpxUInt32 i = 0; i < nNew; i++)
	{
		outNewPairs[i].contact = (SpxContact*)allocator->allocate(sizeof(SpxContact));
		outNewPairs[i].contact->Reset();
	}

	// ~~~~~ 継続して衝突しているペアの状態を更新 ~~~~~
	for (SpxUInt32 i = 0; i < nKeep; i++)
	{
		outKeepPairs[i].contact->Refresh(
			states[outKeepPairs[i].rigidBodyA].m_position,
			states[outKeepPairs[i].rigidBodyA].m_orientation,
			states[outKeepPairs[i].rigidBodyB].m_position,
			states[outKeepPairs[i].rigidBodyB].m_orientation);
	}

	//  ~~~~~
	//  今回検出されたペア(既存のペアと新規衝突ペア)の配列にデータをコピーしていく
	//  ~~~~~
	numNewPairs = 0;
	for (SpxUInt32 i = 0; i < nKeep; i++)
	{
		outKeepPairs[i].type = SpxPairTypeKeep;
		newPairs[numNewPairs++] = outKeepPairs[i];
	}
	for (SpxUInt32 i = 0; i < nNew; i++)
	{
		outNewPairs[i].type = SpxPairTypeNew;
		newPairs[numNewPairs++] = outNewPairs[i];
	}

	allocator->deallocate(outKeepPairs);
	allocator->deallocate(outNewPairs);

	// ソート
	{
		SpxPair* sortBuff = (SpxPair*)allocator->allocate(sizeof(SpxPair) * numNewPairs);
		SpxSort<SpxPair>(newPairs, sortBuff, numNewPairs);
		allocator->deallocate(sortBuff);
	}
}

};	// namespace SimplePhysics