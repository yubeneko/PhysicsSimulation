#pragma once

#include "../SpxBase.h"
#include "SpxContact.h"

namespace SimplePhysics
{
	// ペアの種類
	enum SpxPairType
	{
		SpxPairTypeNew,	  // 新規ペア
		SpxPairTypeKeep,  // 維持
	};

	/**
	 * @brief 衝突時に必要な情報へのリンクを保持するデータ構造
	 * 剛体のインデックスについては、2つの剛体のペアに対してユニークなIDを割り当てるために
	 * rigidbodyA < rigidbodyB となっている必要がある。
	 *
	 */
	struct SpxPair
	{
		SpxPairType type; // 種類
		union {
			SpxUInt64 key; // ユニークなキー
			struct {
				SpxUInt32 rigidBodyA; // 剛体Aのインデックス
				SpxUInt32 rigidBodyB; // 剛体Bのインデックス
			};
		};
		SpxContact* contact; // 衝突情報
	};
};	// namespace SimplePhysics