#pragma once

#include "../SpxBase.h"
#include "SpxShape.h"
#include "../glmExtension.h"

namespace SimplePhysics
{
	const SpxUInt8 SPX_NUM_SHAPES = 5;

	/**
	 * @brief 剛体の形状を保持するコンテナ
	 *
	 */
	struct SpxCollidable
	{
		SpxUInt8 m_numShapes;				// 保持する形状数
		SpxShape m_shapes[SPX_NUM_SHAPES];	// 形状の配列
		glm::vec3 m_center;					// AABBの中心座標
		glm::vec3 m_half;					// AABBのそれぞれの軸の大きさの半分

		void Reset()
		{
			m_numShapes = 0;
			m_center = glm::vec3(0.0f);
			m_center = glm::vec3(0.0f);
		}

		void AddShape(const SpxShape& shape)
		{
			if (m_numShapes < SPX_NUM_SHAPES)
			{
				m_shapes[m_numShapes] = shape;
				++m_numShapes;
			}
		}

		void Finish()
		{
			glm::vec3 aabbMax(-FLT_MAX), aabbMin(FLT_MAX);
			for (SpxUInt32 i = 0; i < m_numShapes; i++)	 // 保持している形状でループを回す
			{
				const SpxConvexMesh& mesh = m_shapes[i].m_geometry;

				for (SpxUInt32 v = 0; v < mesh.m_numVertices; v++)	// メッシュの頂点でループを回す
				{
					aabbMax = GLMExtension::MaxPerElem(
						aabbMax,
						m_shapes[i].m_offsetPosition + m_shapes[i].m_offsetQuaternion * mesh.m_vertices[v]);
					aabbMin = GLMExtension::MinPerElem(
						aabbMin,
						m_shapes[i].m_offsetPosition + m_shapes[i].m_offsetQuaternion * mesh.m_vertices[v]);
				}
			}

			m_center = (aabbMax + aabbMin) * 0.5f;
			m_half = (aabbMax - aabbMin) * 0.5f;
		}
	};
};	// namespace SimplePhysics