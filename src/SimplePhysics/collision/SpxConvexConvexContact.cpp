#include "SpxConvexConvexContact.h"
#include "SpxClosestFunction.h"
#include "../glmExtension.h"

namespace SimplePhysics
{

// 分離軸の種類
enum SpxSatType
{
	// 凸メッシュBの面法線を分離軸にしたとき
	SpxSatTypePointAFacetB,
	// 凸メッシュAの面法線を分離軸にしたとき
	SpxSatTypePointBFacetA,
	// エッジとエッジの外積を分離軸にしたとき
	SpxSatTypeEdgeEdge,
};

// clang-format off

// 分離軸の判定用のマクロ関数
#define SPX_CHECK_MINMAX(axis,AMin,AMax,BMin,BMax,type) \
{\
	++satCount;\
	float d1 = AMin - BMax;\
	float d2 = BMin - AMax;\
	if(d1 >= 0.0f || d2 >= 0.0f) {\
	/* 2つの凸メッシュは衝突していなかった */\
		return false;\
	}\
	if(distanceMin < d1) {\
		/* 貫通深度の更新 */\
		distanceMin = d1;\
		axisMin = axis;\
		satType = type;\
		axisFlip = false;\
	}\
	if(distanceMin < d2) {\
		distanceMin = d2;\
		/* Aを押し返す方向は反転する */\
		axisMin = -axis;\
		satType = type;\
		axisFlip = true;\
	}\
}

// clang-format on
bool SpxConvexConvexContact_local(
	const SpxConvexMesh& convexA,
	const glm::mat4x3& transformA,
	const SpxConvexMesh& convexB,
	const glm::mat4x3& transformB,
	glm::vec3& normal,
	float& penetrationDepth,
	glm::vec3& contactPointA,
	glm::vec3& contactPointB)
{
	// Bローカル->Aローカルへの変換行列
	// Bのローカル座標系 -- 変換1 --> ワールド座標系 -- 変換2 --> Aのローカル座標系 にする。
	// 変換1はBのワールド変換行列、変換2はAのワールド変換行列の逆行列になる。
	glm::mat4x3 transformAB = GLMExtension::AffineTransformMultiply(GLMExtension::OrthoInverse(transformA), transformB);
	// Bローカル->Aローカルへの変換の回転成分
	glm::mat3 matrixAB(transformAB);
	// Bローカル->Aローカルへの変換の並進移動成分
	glm::vec3 offsetAB = GLMExtension::GetTranslation(transformAB);

	// Aローカル->Bローカルへの変換
	glm::mat4x3 transformBA = GLMExtension::OrthoInverse(transformAB);
	// Aローカル->Bローカルへの変換の回転成分
	glm::mat3 matrixBA(transformBA);
	// Aローカル->Bローカルへの変換の並進移動成分
	glm::vec3 offsetBA = GLMExtension::GetTranslation(transformBA);

	// 最も浅い貫通深度とそのときの分離軸
	float distanceMin = -FLT_MAX;
	// 分離軸はAを押し返す方向を向くようにセットされる
	glm::vec3 axisMin(0.0f);
	SpxSatType satType = SpxSatTypeEdgeEdge;
	// 判定に使った分離軸(Aを押し返す方向を考慮してない)とAを押し返す方向が
	// 反対を向いていたかどうかのフラグ
	bool axisFlip;

	// ~~~~~~~~~~~~~~~~ 分離軸判定 ~~~~~~~~~~~~~~~~

	int satCount = 0;

	// 凸メッシュAの面法線を分離軸にしてみる
	for (SpxUInt32 f = 0; f < convexA.m_numFacets; f++)
	{
		const SpxFacet& facet = convexA.m_facets[f];
		// 分離軸
		const glm::vec3 separatingAxis = facet.normal;

		// ConvexAを分離軸に投影
		float minA, maxA;
		SpxGetProjection(minA, maxA, &convexA, separatingAxis);

		// ConvexBを分離軸に投影
		// 判定の際の基準はAのローカル座標系。
		float minB, maxB;
		// 分離軸はAのローカル座標系->Bのローカル座標系に変換しておく。
		SpxGetProjection(minB, maxB, &convexB, matrixBA * facet.normal);
		// Aのローカル座標系におけるBの軸上の位置を計算。
		float offset = glm::dot(offsetAB, separatingAxis);
		// minBとmaxBをAのローカル座標系に変換
		minB += offset;
		maxB += offset;

		// 判定
		SPX_CHECK_MINMAX(separatingAxis, minA, maxA, minB, maxB, SpxSatTypePointBFacetA);
	}

	// 凸メッシュBの面法線を分離軸にしてみる
	for (SpxUInt32 f = 0; f < convexB.m_numFacets; f++)
	{
		const SpxFacet& facet = convexB.m_facets[f];
		// 分離軸はBのローカル座標系からAのローカル座標系に変換する
		const glm::vec3 separatingAxis = matrixAB * facet.normal;

		// ConvexAを分離軸に投影
		float minA, maxA;
		SpxGetProjection(minA, maxA, &convexA, separatingAxis);

		// ConvexBを分離軸に投影
		float minB, maxB;
		SpxGetProjection(minB, maxB, &convexB, facet.normal);
		// Aのローカル座標系におけるBの軸上の位置を計算。
		float offset = dot(offsetAB, separatingAxis);
		minB += offset;
		maxB += offset;

		// 判定
		SPX_CHECK_MINMAX(separatingAxis, minA, maxA, minB, maxB, SpxSatTypePointAFacetB);
	}

	// ConvexAとConvexBのエッジの外積を分離軸とする

	// 大外はAのエッジでループ
	for (SpxUInt32 eA = 0; eA < convexA.m_numEdges; eA++)
	{
		const SpxEdge& edgeA = convexA.m_edges[eA];

		// エッジの種類が凸じゃないならば判定しない
		if (edgeA.type != SpxEdgeTypeConvex) { continue; }

		// 凸メッシュA側のエッジのベクトルを作成
		const glm::vec3 edgeVecA = convexA.m_vertices[edgeA.vertId[1]] - convexA.m_vertices[edgeA.vertId[0]];

		// 内側はBのエッジでループ
		for (SpxUInt32 eB = 0; eB < convexB.m_numEdges; eB++)
		{
			const SpxEdge& edgeB = convexB.m_edges[eB];
			if (edgeB.type != SpxEdgeTypeConvex) continue;

			// 凸メッシュB側のエッジのベクトルを作成
			// 判定はやはりAのローカル座標系なので、ベクトルをBのローカル座標系からAのローカル座標系に変換する
			const glm::vec3 edgeVecB = matrixAB * (convexB.m_vertices[edgeB.vertId[1]] - convexB.m_vertices[edgeB.vertId[0]]);

			glm::vec3 separatingAxis = glm::cross(edgeVecA, edgeVecB);
			// 2つのベクトルの外積が0に近い(2つのベクトルがほぼ平行)ならば、そのベクトルは分離軸として使えないので
			// 判定をスキップする
			if (glm::length2(separatingAxis) < SPX_EPSILON * SPX_EPSILON) continue;

			separatingAxis = glm::normalize(separatingAxis);

			// ConvexAを分離軸に投影
			float minA, maxA;
			SpxGetProjection(minA, maxA, &convexA, separatingAxis);

			// ConvexBを分離軸に投影
			float minB, maxB;
			SpxGetProjection(minB, maxB, &convexB, matrixBA * separatingAxis);
			float offset = glm::dot(offsetAB, separatingAxis);
			minB += offset;
			maxB += offset;

			// 判定
			SPX_CHECK_MINMAX(separatingAxis, minA, maxA, minB, maxB, SpxSatTypeEdgeEdge);
		}
	}

	// ここまで到達した場合、２つの凸メッシュは交差している。
	// また、反発ベクトル(axisMin)と貫通深度(distanceMin)が求まった。
	// 反発ベクトルはＡを押しだす方向をプラスにとる。

	// ~~~~~~~~~~~~~~~~ 衝突座標検出 ~~~~~~~~~~~~~~~~

	int collCount = 0;

	float closestMinSqr = FLT_MAX;
	glm::vec3 closestPointA, closestPointB;
	// 衝突点を求めるために、形状の重なりを無くす必要がある。
	glm::vec3 separation = 1.1f * glm::abs(distanceMin) * axisMin;

	// 大外はAの面でループ
	for (SpxUInt32 fA = 0; fA < convexA.m_numFacets; fA++)
	{
		const SpxFacet& facetA = convexA.m_facets[fA];

		float checkA = glm::dot(facetA.normal, -axisMin);
		// axisFlip の意味がわからない。
		if (satType == SpxSatTypePointBFacetA && checkA < 0.99f && axisFlip)
		{
			// 判定軸が面Aの法線のとき、向きの違うAの面は判定しない
			continue;
		}

		if (checkA < 0.0f)
		{
			// 衝突面と逆に向いている面は判定しない
			continue;
		}

		// 内側はBの面でループ
		for (SpxUInt32 fB = 0; fB < convexB.m_numFacets; fB++)
		{
			const SpxFacet& facetB = convexB.m_facets[fB];

			float checkB = dot(facetB.normal, matrixBA * axisMin);
			if (satType == SpxSatTypePointAFacetB && checkB < 0.99f && !axisFlip)
			{
				// 判定軸が面Bの法線のとき、向きの違うBの面は判定しない
				continue;
			}

			if (checkB < 0.0f)
			{
				// 衝突面と逆に向いている面は判定しない
				continue;
			}

			collCount++;

			// 面Ａと面Ｂの最近接点を求める
			glm::vec3 triangleA[3] = {
				separation + convexA.m_vertices[facetA.vertId[0]],
				separation + convexA.m_vertices[facetA.vertId[1]],
				separation + convexA.m_vertices[facetA.vertId[2]],
			};

			glm::vec3 triangleB[3] = {
				offsetAB + matrixAB * convexB.m_vertices[facetB.vertId[0]],
				offsetAB + matrixAB * convexB.m_vertices[facetB.vertId[1]],
				offsetAB + matrixAB * convexB.m_vertices[facetB.vertId[2]],
			};

			// エッジ同士の最近接点算出
			for (int i = 0; i < 3; i++)
			{
				if (convexA.m_edges[facetA.edgeId[i]].type != SpxEdgeTypeConvex) continue;

				for (int j = 0; j < 3; j++)
				{
					if (convexB.m_edges[facetB.edgeId[j]].type != SpxEdgeTypeConvex) continue;

					glm::vec3 sA, sB;
					SpxGetClosestTwoSegments(
						triangleA[i], triangleA[(i + 1) % 3],
						triangleB[j], triangleB[(j + 1) % 3],
						sA, sB);

					float dSqr = glm::length2(sA - sB);
					if (dSqr < closestMinSqr)
					{
						closestMinSqr = dSqr;
						closestPointA = sA;
						closestPointB = sB;
					}
				}
			}

			// 頂点Ａ→面Ｂの最近接点算出
			for (int i = 0; i < 3; i++)
			{
				glm::vec3 s;
				SpxGetClosestPointTriangle(triangleA[i], triangleB[0], triangleB[1], triangleB[2], matrixAB * facetB.normal, s);
				float dSqr = glm::length2(triangleA[i] - s);
				if (dSqr < closestMinSqr)
				{
					closestMinSqr = dSqr;
					closestPointA = triangleA[i];
					closestPointB = s;
				}
			}

			// 頂点Ｂ→面Ａの最近接点算出
			for (int i = 0; i < 3; i++)
			{
				glm::vec3 s;
				SpxGetClosestPointTriangle(triangleB[i], triangleA[0], triangleA[1], triangleA[2], facetA.normal, s);
				float dSqr = glm::length2(triangleB[i] - s);
				if (dSqr < closestMinSqr)
				{
					closestMinSqr = dSqr;
					closestPointA = s;
					closestPointB = triangleB[i];
				}
			}
		}
	}

	normal = glm::mat3(transformA) * axisMin;
	penetrationDepth = distanceMin;
	contactPointA = closestPointA - separation;
	contactPointB = offsetBA + matrixBA * closestPointB;

	return true;
}

bool SpxConvexConvexContact(
	const SpxConvexMesh& convexA,
	const glm::mat4x3& transformA,
	const SpxConvexMesh& convexB,
	const glm::mat4x3& transformB,
	glm::vec3& normal,
	float& penetrationDepth,
	glm::vec3& contactPointA,
	glm::vec3& contactPointB)
{
	// 座標系の変換の回数を減らすために面数が多いを座標系の基準とする

	bool ret;
	if (convexA.m_numFacets >= convexB.m_numFacets)
	{
		ret = SpxConvexConvexContact_local(
			convexA, transformA,
			convexB, transformB,
			normal, penetrationDepth, contactPointA, contactPointB);
	}
	else {
		ret = SpxConvexConvexContact_local(
			convexB, transformB,
			convexA, transformA,
			normal, penetrationDepth, contactPointB, contactPointA);
		normal = -normal;
	}

	return ret;
}

};	// namespace SimplePhysics