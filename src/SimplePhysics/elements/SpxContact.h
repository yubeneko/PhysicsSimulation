#pragma once

#include "../SpxBase.h"
#include "SpxConstraint.h"

namespace SimplePhysics
{
// 衝突点
struct SpxContactPoint
{
	float distance;				   // 貫通深度
	glm::vec3 pointA;			   // 衝突点(剛体Aのローカル座標系)
	glm::vec3 pointB;			   // 衝突点(剛体Bのローカル座標系)
	glm::vec3 normal;			   // 衝突点の法線ベクトル(ワールド座標系)
	SpxConstraint constraints[3];  // 拘束(1つの衝突点に対して3つの拘束)

	void Reset()
	{
		constraints[0].accumImpulse = 0.0f;
		constraints[1].accumImpulse = 0.0f;
		constraints[2].accumImpulse = 0.0f;
	}
};

const SpxUInt8 SPX_NUM_CONTACTS = 4;

// 衝突検出により計算された衝突点の詳細情報
struct SpxContact
{
	SpxUInt32 m_numContacts;							// 衝突の数
	float m_friction;									// 摩擦
	SpxContactPoint m_contactPoints[SPX_NUM_CONTACTS];	// 衝突点の配列(最大で4つ)

	/**
	 * @brief 同一衝突点を探す
	 *
	 * @param newPointA 衝突点(剛体Aのローカル座標系)
	 * @param newPointB 衝突点(剛体Bのローカル座標系)
	 * @param newNormal 衝突点の法線ベクトル(ワールド座標系)
	 * @return 同じ衝突点を見つけた場合はそのインデックスを戻す。
	 * もし見つからなかった場合は -1 を戻す。
	 */
	int FindNearestContactPoint(const glm::vec3& newPointA, const glm::vec3& newPointB, const glm::vec3& newNormal);

	/**
	 * @brief 衝突点を入れ替える
	 *
	 * @param newPoint 衝突点(剛体Aのローカル座標系)
	 * @param newDistance 貫通深度
	 * @return int 破棄する衝突点のインデックスを戻す(0,1,2,3)
	 */
	int Sort4ContactPoints(const glm::vec3& newPoint, float newDistance);

	/**
	 * @brief 衝突点を破棄する
	 *
	 * @param i 破棄する衝突点のインデックス
	 */
	void RemoveContactPoint(int i);

	void Reset();

	/**
	 * @brief 衝突点をリフレッシュする
	 *
	 * キャッシュされている衝突情報が有効かどうか確認し、無効になっている衝突点があれば破棄する。
	 *
	 * @param pA 剛体Aの位置
	 * @param qA 剛体Aの姿勢
	 * @param pB 剛体Bの位置
	 * @param qB 剛体Bの姿勢
	 */
	void Refresh(const glm::vec3& pA, const glm::quat& qA, const glm::vec3& pB, const glm::quat& qB);

	/**
	 * @brief 衝突点をマージする
	 *
	 * @param contact 合成する衝突情報
	 */
	void Merge(const SpxContact& contact);

	/**
	 * @brief 衝突点を追加する
	 *
	 * @param penetrationDepth 貫通深度
	 * @param normal 衝突点の法線ベクトル(ワールド座標)
	 * @param contactPointA 衝突点(剛体Aのローカル座標)
	 * @param contactPointB 衝突点(剛体Bのローカル座標)
	 */
	void AddContact(
		float penetrationDepth,
		const glm::vec3& normal,
		const glm::vec3& contactPointA,
		const glm::vec3& contactPointB);
};

};	// namespace SimplePhysics