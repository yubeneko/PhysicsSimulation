#pragma once

#include "../SpxBase.h"
#include "SpxConvexMesh.h"

#include <glm/gtx/quaternion.hpp>

namespace SimplePhysics
{
	struct SpxShape
	{
		SpxConvexMesh m_geometry;	   // 凸メッシュ
		glm::vec3 m_offsetPosition;	   // 座標のオフセット
		glm::quat m_offsetQuaternion;  // 回転のオフセット
		void* userData;				   // ユーザーデータ

		void Reset()
		{
			m_geometry.Reset();
			m_offsetPosition = glm::vec3(0.0f);
			m_offsetQuaternion = glm::identity<glm::quat>();
			userData = nullptr;
		}
	};
};	// namespace SimplePhysics