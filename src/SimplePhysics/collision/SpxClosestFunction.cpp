#include "SpxClosestFunction.h"

namespace SimplePhysics
{

void SpxGetClosestTwoSegments(
	const glm::vec3& segmentPointA0,
	const glm::vec3& segmentPointA1,
	const glm::vec3& segmentPointB0,
	const glm::vec3& segmentPointB1,
	glm::vec3& closestPointA,
	glm::vec3& closestPointB)
{
	// 式を構成するための値
	glm::vec3 v1 = segmentPointA1 - segmentPointA0;
	glm::vec3 v2 = segmentPointB1 - segmentPointB0;
	glm::vec3 r = segmentPointA0 - segmentPointB0;

	float a = glm::dot(v1, v1);
	float b = glm::dot(v1, v2);
	float c = glm::dot(v2, v2);
	float d = glm::dot(v1, r);
	float e = glm::dot(v2, r);
	float det = -a * c + b * b;
	float s = 0.0f, t = 0.0f;

	// 逆行列があれば(行列式が0でないならば)、行列の演算によりsを計算する
	if (det * det > SPX_EPSILON)
	{
		s = (c * d - b * e) / det;
	}

	// 線分A上の最近接点を決めるパラメータsを0.0～1.0でクランプ
	s = glm::clamp(s, 0.0f, 1.0f);

	// 線分Bのtを求める
	// t = dot((segmentPointA0+s*v1) - segmentPointB0,v2) / dot(v2,v2);
	t = (e + s * b) / c;

	// 線分B上の最近接点を決めるパラメータtを0.0～1.0でクランプ
	t = glm::clamp(t, 0.0f, 1.0f);

	// 再度、線分A上の点を求める
	// s = dot((segmentPointB0+t*v2) - segmentPointA0,v1) / dot(v1,v1);
	s = (-d + t * b) / a;
	s = glm::clamp(s, 0.0f, 1.0f);

	closestPointA = segmentPointA0 + s * v1;
	closestPointB = segmentPointB0 + t * v2;
}

/**
 * @brief 点から直線への最近接点
 *
 * @param point 点
 * @param linePoint 直線の始点
 * @param lineDirection 直線の方向ベクトル
 * @param closestPoint 点と直線の最近接点(出力)
 */
inline void SpxGetClosestPointLine(
	const glm::vec3& point,
	const glm::vec3& linePoint,
	const glm::vec3& lineDirection,
	glm::vec3& closestPoint)
{
	float s = dot(point - linePoint, lineDirection) / dot(lineDirection, lineDirection);
	closestPoint = linePoint + s * lineDirection;
}

void SpxGetClosestPointTriangle(
	const glm::vec3& point,
	const glm::vec3& trianglePoint0,
	const glm::vec3& trianglePoint1,
	const glm::vec3& trianglePoint2,
	const glm::vec3& triangleNormal,
	glm::vec3& closestPoint)
{
	// ３角形面上の投影点
	glm::vec3 proj = point - glm::dot(triangleNormal, point - trianglePoint0) * triangleNormal;

	// エッジP0,P1のボロノイ領域
	glm::vec3 edgeP01 = trianglePoint1 - trianglePoint0;
	glm::vec3 edgeP01_normal = glm::cross(edgeP01, triangleNormal);

	// 投影点がボロノイ領域を囲む面の表側にあるかどうかチェックする
	float voronoiEdgeP01_check1 = glm::dot(proj - trianglePoint0, edgeP01_normal);
	float voronoiEdgeP01_check2 = glm::dot(proj - trianglePoint0, edgeP01);
	float voronoiEdgeP01_check3 = glm::dot(proj - trianglePoint1, -edgeP01);

	if (voronoiEdgeP01_check1 > 0.0f && voronoiEdgeP01_check2 > 0.0f && voronoiEdgeP01_check3 > 0.0f)
	{
		SpxGetClosestPointLine(trianglePoint0, edgeP01, proj, closestPoint);
		return;
	}

	// エッジP1,P2のボロノイ領域
	glm::vec3 edgeP12 = trianglePoint2 - trianglePoint1;
	glm::vec3 edgeP12_normal = glm::cross(edgeP12, triangleNormal);

	float voronoiEdgeP12_check1 = glm::dot(proj - trianglePoint1, edgeP12_normal);
	float voronoiEdgeP12_check2 = glm::dot(proj - trianglePoint1, edgeP12);
	float voronoiEdgeP12_check3 = glm::dot(proj - trianglePoint2, -edgeP12);

	if (voronoiEdgeP12_check1 > 0.0f && voronoiEdgeP12_check2 > 0.0f && voronoiEdgeP12_check3 > 0.0f)
	{
		SpxGetClosestPointLine(trianglePoint1, edgeP12, proj, closestPoint);
		return;
	}

	// エッジP2,P0のボロノイ領域
	glm::vec3 edgeP20 = trianglePoint0 - trianglePoint2;
	glm::vec3 edgeP20_normal = glm::cross(edgeP20, triangleNormal);

	float voronoiEdgeP20_check1 = glm::dot(proj - trianglePoint2, edgeP20_normal);
	float voronoiEdgeP20_check2 = glm::dot(proj - trianglePoint2, edgeP20);
	float voronoiEdgeP20_check3 = glm::dot(proj - trianglePoint0, -edgeP20);

	if (voronoiEdgeP20_check1 > 0.0f && voronoiEdgeP20_check2 > 0.0f && voronoiEdgeP20_check3 > 0.0f)
	{
		SpxGetClosestPointLine(trianglePoint2, edgeP20, proj, closestPoint);
		return;
	}

	// ３角形面の内側
	if (voronoiEdgeP01_check1 <= 0.0f && voronoiEdgeP12_check1 <= 0.0f && voronoiEdgeP20_check1 <= 0.0f)
	{
		closestPoint = proj;
		return;
	}

	// 頂点P0のボロノイ領域
	if (voronoiEdgeP01_check2 <= 0.0f && voronoiEdgeP20_check3 <= 0.0f)
	{
		closestPoint = trianglePoint0;
		return;
	}

	// 頂点P1のボロノイ領域
	if (voronoiEdgeP01_check3 <= 0.0f && voronoiEdgeP12_check2 <= 0.0f)
	{
		closestPoint = trianglePoint1;
		return;
	}

	// 頂点P2のボロノイ領域
	if (voronoiEdgeP20_check2 <= 0.0f && voronoiEdgeP12_check3 <= 0.0f)
	{
		closestPoint = trianglePoint2;
		return;
	}
}
};	// namespace SimplePhysics