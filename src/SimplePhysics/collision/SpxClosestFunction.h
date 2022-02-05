#pragma once

#include "../SpxBase.h"

namespace SimplePhysics
{

/**
 * @brief 2つの線分の最近接点の検出
 *
 * @param segmentPointA0 線分Aの始点
 * @param segmentPointA1 線分Aの終点
 * @param segmentPointB0 線分Bの始点
 * @param segmentPointB1 線分Bの終点
 * @param closestPointA 線分A上の最近接点(出力)
 * @param closestPointB 線分B上の最近接点(出力)
 */
void SpxGetClosestTwoSegments(
	const glm::vec3& segmentPointA0,
	const glm::vec3& segmentPointA1,
	const glm::vec3& segmentPointB0,
	const glm::vec3& segmentPointB1,
	glm::vec3& closestPointA,
	glm::vec3& closestPointB);

/**
 * @brief 頂点から3角形面への最近接点の検出
 *
 * @param point 頂点
 * @param trianglePoint0 ３角形面の頂点0
 * @param trianglePoint1 ３角形面の頂点1
 * @param trianglePoint2 ３角形面の頂点2
 * @param triangleNormal ３角形面の法線ベクトル
 * @param closestPoint 3角形面上の最近接点
 */
void SpxGetClosestPointTriangle(
	const glm::vec3& point,
	const glm::vec3& trianglePoint0,
	const glm::vec3& trianglePoint1,
	const glm::vec3& trianglePoint2,
	const glm::vec3& triangleNormal,
	glm::vec3& closestPoint);

};	// namespace SimplePhysics