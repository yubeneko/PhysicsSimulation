#pragma once

#include "../SpxBase.h"

namespace SimplePhysics
{
	// 拘束演算に必要なパラメータを保持
	struct SpxConstraint
	{
		glm::vec3 axis;		 // 拘束軸
		float jacDiagInv;	 // 拘束式の分母
		float rhs;			 // 初期拘束力
		float lowerLimit;	 // 拘束力の下限
		float upperLimit;	 // 拘束力の上限
		float accumImpulse;	 // 蓄積される拘束力
	};
};	// namespace SimplePhysics