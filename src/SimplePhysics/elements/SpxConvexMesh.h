#pragma once

#include "../SpxBase.h"

namespace SimplePhysics
{
	// 凸メッシュの頂点、辺、面の最大値
	const SpxUInt32 SPX_CONVEX_MESH_MAX_VERTICES = 34;
	const SpxUInt32 SPX_CONVEX_MESH_MAX_EDGES = 96;
	const SpxUInt32 SPX_CONVEX_MESH_MAX_FACETS = 64;

	enum SpxEdgeType
	{
		SpxEdgeTypeConvex,	 // 凸エッジ
		SpxEdgeTypeConcave,	 // 凹エッジ
		SpxEdgeTypeFlat,	 // 平坦エッジ
	};

	// エッジ
	struct SpxEdge
	{
		SpxUInt8 type;		  // エッジの種類
		SpxUInt8 vertId[2];	  // 端点の頂点インデックス
		SpxUInt8 facetId[2];  // 共有する面インデックス
	};

	// 3角形面
	struct SpxFacet
	{
		SpxUInt8 vertId[3];	 // 頂点インデックス
		SpxUInt8 edgeId[3];	 // エッジインデックス
		glm::vec3 normal;	 // 面法線ベクトル
	};

	// 凸メッシュ
	struct SpxConvexMesh
	{
		SpxUInt8 m_numVertices;								 // 頂点数(最大で255個)
		SpxUInt8 m_numFacets;								 // 面の数
		SpxUInt8 m_numEdges;								 // エッジの数
		glm::vec3 m_vertices[SPX_CONVEX_MESH_MAX_VERTICES];	 // 頂点配列
		SpxEdge m_edges[SPX_CONVEX_MESH_MAX_EDGES];		 // エッジ配列
		SpxFacet m_facets[SPX_CONVEX_MESH_MAX_FACETS];		 // 面配列

		// 初期化
		void Reset()
		{
			m_numVertices = 0;
			m_numFacets = 0;
			m_numEdges = 0;
		}
	};

	/**
	 * @brief 軸上に凸メッシュを投影して最小値と最大値を得る
	 *
	 * @param pmin 投影領域の最小値
	 * @param pmax 投影領域の最大値
	 * @param convexMesh 凸メッシュ
	 * @param axis 投影軸
	 */
	void SpxGetProjection(
		float& pmin,
		float& pmax,
		const SpxConvexMesh* convexMesh,
		const glm::vec3& axis);

	/**
	 * @brief 凸メッシュを作成する <br>
	 * - 入力データがすでに凸包になっていること <br>
	 * - 入力データは3角ポリゴンで構成されていること <br>
	 * - 3平面から共有されるエッジ、穴あき面はNG <br>
	 * - 縮退面(面積が0の面)は自動的に削除される
	 *
	 * @param convexMesh 凸メッシュ
	 * @param vertices 頂点配列
	 * (xyzの順に頂点が並ぶ。つまり、サイズは頂点数*3個分)
	 * @param numVertices 頂点数(四面体なら4)
	 * @param indices 面を構成する頂点のインデックスの配列
	 * インデックスは反時計回りが面の表になるので、インデックスの順番は大事
	 * 1つの面に対して3つの頂点idが必要なので、四面体ならばindicesのサイズは12になる。
	 * @param numIndices 面インデックス数(indicesのサイズを3で割った数。つまり面の数)
	 * @param scale スケール
	 * @return true
	 * @return false
	 */
	bool SpxCreateConvexMesh(
		SpxConvexMesh* convexMesh,
		const float* vertices,
		SpxUInt32 numVertices,
		const SpxUInt16* indices,
		SpxUInt32 numIndices,
		const glm::vec3& scale = glm::vec3(1.0f));
};	// namespace SimplePhysics