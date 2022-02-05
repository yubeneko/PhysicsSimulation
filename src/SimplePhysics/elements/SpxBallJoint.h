#pragma once

#include "../SpxBase.h"
#include "SpxConstraint.h"

namespace SimplePhysics
{
	struct SpxBallJoint
	{
		float bias;				   // 拘束の強さの調整値
		SpxUInt32 rigidBodyA;	   // 剛体Aへのインデックス
		SpxUInt32 rigidBodyB;	   // 剛体Bへのインデックス
		glm::vec3 anchorA;		   // 剛体Aのローカル座標系における接続点
		glm::vec3 anchorB;		   // 剛体Bのローカル座標系における接続点
		SpxConstraint constraint;  // 拘束

		void Reset()
		{
			bias = 0.1f;
			constraint.accumImpulse = 0.0f;
		}
	};
};	// namespace SimplePhysics