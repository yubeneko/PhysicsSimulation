#pragma once

#include "../SpxBase.h"

namespace SimplePhysics
{
	/**
	 * @brief 剛体の属性を格納するデータ型
	 *
	 */
	struct SpxRigidBody
	{
		glm::mat3 m_inertia;  // 慣性テンソル
		float m_mass;		  // 質量
		float m_restitution;  // 反発係数
		float m_friction;	  // 摩擦係数

		void Reset()
		{
			m_mass = 1.0f;
			m_inertia = glm::mat3(1.0f);  // 単位行列
			m_restitution = 0.2f;
			m_friction = 0.6f;
		}
	};
};	// namespace SimplePhysics