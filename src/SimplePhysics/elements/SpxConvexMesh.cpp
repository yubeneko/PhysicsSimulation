#include "SpxConvexMesh.h"
#include <cstring>
#include <iostream>
#include "../glmExtension.h"
#include <cassert>

namespace SimplePhysics
{

void SpxGetProjection(
	float& pmin,
	float& pmax,
	const SpxConvexMesh* convexMesh,
	const glm::vec3& axis)
{
	float pmin_ = FLT_MAX;
	float pmax_ = -FLT_MAX;

	// 全ての頂点に対して軸に投影していく
	for (SpxUInt32 i = 0; i < convexMesh->m_numVertices; i++)
	{
		float prj = dot(axis, convexMesh->m_vertices[i]);
		pmin_ = glm::min(pmin_, prj);
		pmax_ = glm::max(pmax_, prj);
	}

	pmin = pmin_;
	pmax = pmax_;
}

// コピペ修正
bool SpxCreateConvexMesh(
	SpxConvexMesh* convexMesh,
	const float* vertices,
	SpxUInt32 numVertices,
	const SpxUInt16* indices,
	SpxUInt32 numIndices,
	const glm::vec3& scale)
{
	assert(convexMesh);
	assert(vertices);
	assert(indices);
	assert(dot(scale, scale) > 0.0f);

	if (numVertices > SPX_CONVEX_MESH_MAX_VERTICES || numIndices > SPX_CONVEX_MESH_MAX_FACETS * 3)
	{
		return false;
	}

	memset(convexMesh, 0, sizeof(SpxConvexMesh));

	// 頂点バッファ作成
	for (SpxUInt32 i = 0; i < numVertices; i++)
	{
		convexMesh->m_vertices[i][0] = vertices[i * 3];
		convexMesh->m_vertices[i][1] = vertices[i * 3 + 1];
		convexMesh->m_vertices[i][2] = vertices[i * 3 + 2];
		// 要素をスケーリング
		convexMesh->m_vertices[i] *= scale;
	}
	convexMesh->m_numVertices = numVertices;

	// 面バッファ作成
	SpxUInt32 nf = 0;
	for (SpxUInt32 i = 0; i < numIndices / 3; i++)
	{
		glm::vec3 p[3] = {
			convexMesh->m_vertices[indices[i * 3]],
			convexMesh->m_vertices[indices[i * 3 + 1]],
			convexMesh->m_vertices[indices[i * 3 + 2]]};

		// 面の法線ベクトル
		glm::vec3 normal = glm::cross(p[1] - p[0], p[2] - p[0]);
		// 外積の大きさはその2つのベクトルで作る平行四辺形の面積に等しい。
		// 面積が0に近いということは、その面は縮退面なので面としてカウントしない
		float areaSqr = glm::length2(normal);  // 面積
		if (areaSqr > SPX_EPSILON * SPX_EPSILON)
		{  // 縮退面は登録しない
			convexMesh->m_facets[nf].vertId[0] = (SpxUInt8)indices[i * 3];
			convexMesh->m_facets[nf].vertId[1] = (SpxUInt8)indices[i * 3 + 1];
			convexMesh->m_facets[nf].vertId[2] = (SpxUInt8)indices[i * 3 + 2];
			convexMesh->m_facets[nf].normal = normal / sqrtf(areaSqr);
			nf++;
		}
	}
	convexMesh->m_numFacets = nf;

	// エッジバッファ作成
	// 重複したエッジのセットを登録しないために、ハッシュテーブルのようなものを作っておく
	// また、1つのエッジを共有する面は2つあるが、その2つ目の面が見つかった時に面同士のなす角を調べるためにもこれを使う

	// エッジのハッシュテーブル
	SpxUInt8 edgeIdTable[SPX_CONVEX_MESH_MAX_VERTICES * SPX_CONVEX_MESH_MAX_VERTICES / 2];
	memset(edgeIdTable, 0xff, sizeof(edgeIdTable));

	SpxUInt32 ne = 0;
	for (SpxUInt32 i = 0; i < convexMesh->m_numFacets; i++)
	{
		// これから調査する面
		SpxFacet& facet = convexMesh->m_facets[i];
		// 面の辺でループを回していく
		for (SpxUInt32 e = 0; e < 3; e++)
		{
			SpxUInt32 vertId0 = glm::min(facet.vertId[e % 3], facet.vertId[(e + 1) % 3]);
			SpxUInt32 vertId1 = glm::max(facet.vertId[e % 3], facet.vertId[(e + 1) % 3]);
			// 頂点IDからハッシュ値を作る
			SpxUInt32 tableId = vertId1 * (vertId1 - 1) / 2 + vertId0;
			if (edgeIdTable[tableId] == 0xff)
			{
				// 初回時は登録のみ
				convexMesh->m_edges[ne].facetId[0] = i;
				convexMesh->m_edges[ne].facetId[1] = i;
				convexMesh->m_edges[ne].vertId[0] = (SpxUInt8)vertId0;
				convexMesh->m_edges[ne].vertId[1] = (SpxUInt8)vertId1;
				convexMesh->m_edges[ne].type = SpxEdgeTypeConvex;  // 凸エッジで初期化
				facet.edgeId[e] = ne;
				edgeIdTable[tableId] = ne;
				ne++;
				if (ne > SPX_CONVEX_MESH_MAX_EDGES)
				{
					return false;
				}
			}
			else {
				// 共有面を見つけたので、エッジの角度を判定
				assert(edgeIdTable[tableId] < SPX_CONVEX_MESH_MAX_EDGES);
				SpxEdge& edge = convexMesh->m_edges[edgeIdTable[tableId]];
				// 今調べているエッジを共有するもう1つの面
				SpxFacet& facetB = convexMesh->m_facets[edge.facetId[0]];

				assert(edge.facetId[0] == edge.facetId[1]);

				// エッジに含まれないＡ面の頂点がB面の表か裏かで判断
				glm::vec3 s = convexMesh->m_vertices[facet.vertId[(e + 2) % 3]];
				glm::vec3 q = convexMesh->m_vertices[facetB.vertId[0]];
				float d = glm::dot(s - q, facetB.normal);

				if (d < -SPX_EPSILON)
				{
					// d が(ほぼ)0より小さい -> なす角が鈍角
					// -> その辺は凸になっている
					edge.type = SpxEdgeTypeConvex;
				}
				else if (d > SPX_EPSILON) {
					// d が(ほぼ)0より大きかった -> なす角が鋭角である
					// -> その辺は凹んだ位置にある
					// つまり、本来ここに来てはいけない
					edge.type = SpxEdgeTypeConcave;
				}
				else {
					edge.type = SpxEdgeTypeFlat;
				}
				// エッジのもう片方の面のIDを登録
				edge.facetId[1] = i;
			}
		}
	}
	convexMesh->m_numEdges = ne;

	return true;
}

};	// namespace SimplePhysics