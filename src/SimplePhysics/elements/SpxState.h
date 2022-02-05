#pragma once

#include "../SpxBase.h"
#include <glm/gtx/quaternion.hpp>

namespace SimplePhysics
{
	// モーションタイプ(剛体の振る舞い)
	enum SpxMotionType
	{
		SpxMotionTypeActive,  // アクティブ
		SpxMotionTypeStatic,  // 固定
	};

	// 剛体の状態(速度、姿勢等)
	struct SpxState
	{
		glm::vec3 m_position;		  // 座標
		glm::quat m_orientation;	  // 姿勢
		glm::vec3 m_linearVelocity;	  // 並進速度
		glm::vec3 m_angularVelocity;  // 回転速度
		SpxMotionType m_motionType;	  // 動的か固定されているか

		void Reset()
		{
			m_position = glm::vec3(0.0f);
			m_orientation = glm::identity<glm::quat>();
			m_linearVelocity = glm::vec3(0.0f);
			m_angularVelocity = glm::vec3(0.0f);
			m_motionType = SpxMotionTypeActive;
		}
	};
};	// namespace SimplePhysics