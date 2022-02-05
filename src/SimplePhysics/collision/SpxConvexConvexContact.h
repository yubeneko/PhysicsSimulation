#pragma once

#include "../SpxBase.h"
#include "../elements/SpxConvexMesh.h"

namespace SimplePhysics
{
	/**
	 * @brief 2つの凸メッシュの衝突検出
	 *
	 * @param convexA 凸メッシュA
	 * @param transformA Aのワールド変換行列(3行4列)
	 * @param convexB 凸メッシュB
	 * @param transformB Bのワールド変換行列(3行4列)
	 * @param normal 衝突点の法線ベクトル(ワールド座標系)
	 * @param penetrationDepth 貫通深度
	 * @param contactPointA 衝突点(剛体Aのローカル座標系)
	 * @param contactPointB 衝突点(剛体Bのローカル座標系)
	 * @return 衝突が検出されたら true
	 */
	bool SpxConvexConvexContact(
		const SpxConvexMesh& convexA,
		const glm::mat4x3& transformA,
		const SpxConvexMesh& convexB,
		const glm::mat4x3& transformB,
		glm::vec3& normal,
		float& penetrationDepth,
		glm::vec3& contactPointA,
		glm::vec3& contactPointB);
};	// namespace SimplePhysics