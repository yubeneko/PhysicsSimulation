#include "SpxContact.h"
#include "../glmExtension.h"

namespace SimplePhysics
{

// 全てコピペ修正

const float SPX_CONTACT_SAME_POINT = 0.01f;
const float SPX_CONTACT_THRESHOLD_NORMAL = 0.01f;	 // 衝突点の閾値(法線方向)
const float SPX_CONTACT_THRESHOLD_TANGENT = 0.002f;	 // 衝突点の閾値(平面上)

/**
 * @brief 4つの点からなる領域の面積を求める。
 * ただし、頂点のインデックスがバラバラなので、全ての組み合わせの中で一番最大となるものを採用する
 *
 * @param p0
 * @param p1
 * @param p2
 * @param p3
 * @return 面積
 */
static inline float CalcArea4Points(const glm::vec3& p0, const glm::vec3& p1, const glm::vec3& p2, const glm::vec3& p3)
{
	float areaSqrA = glm::length2(cross(p0 - p1, p2 - p3));
	float areaSqrB = glm::length2(cross(p0 - p2, p1 - p3));
	float areaSqrC = glm::length2(cross(p0 - p3, p1 - p2));
	return glm::max(glm::max(areaSqrA, areaSqrB), areaSqrC);
}

int SpxContact::FindNearestContactPoint(const glm::vec3& newPointA, const glm::vec3& newPointB, const glm::vec3& newNormal)
{
	int nearestIdx = -1;

	float minDiff = SPX_CONTACT_SAME_POINT;
	for (SpxUInt32 i = 0; i < m_numContacts; i++)
	{
		float diffA = glm::length2(m_contactPoints[i].pointA - newPointA);
		float diffB = glm::length2(m_contactPoints[i].pointB - newPointB);
		if (diffA < minDiff && diffB < minDiff && glm::dot(newNormal, m_contactPoints[i].normal) > 0.99f)
		{
			minDiff = glm::max(diffA, diffB);
			nearestIdx = i;
		}
	}

	return nearestIdx;
}

int SpxContact::Sort4ContactPoints(const glm::vec3& newPoint, float newDistance)
{
	int maxPenetrationIndex = -1;
	float maxPenetration = newDistance;

	// 最も深い衝突点は排除対象からはずす
	for (SpxUInt32 i = 0; i < m_numContacts; i++)
	{
		if (m_contactPoints[i].distance < maxPenetration)
		{
			maxPenetrationIndex = i;
			maxPenetration = m_contactPoints[i].distance;
		}
	}

	float res[4] = {0.0f};

	// 各点を除いたときの衝突点が作る面積のうち、最も大きくなるものを選択
	glm::vec3 newp(newPoint);
	glm::vec3 p[4] = {
		m_contactPoints[0].pointA,
		m_contactPoints[1].pointA,
		m_contactPoints[2].pointA,
		m_contactPoints[3].pointA,
	};

	if (maxPenetrationIndex != 0)
	{
		res[0] = CalcArea4Points(newp, p[1], p[2], p[3]);
	}

	if (maxPenetrationIndex != 1)
	{
		res[1] = CalcArea4Points(newp, p[0], p[2], p[3]);
	}

	if (maxPenetrationIndex != 2)
	{
		res[2] = CalcArea4Points(newp, p[0], p[1], p[3]);
	}

	if (maxPenetrationIndex != 3)
	{
		res[3] = CalcArea4Points(newp, p[0], p[1], p[2]);
	}

	int maxIndex = 0;
	float maxVal = res[0];

	if (res[1] > maxVal)
	{
		maxIndex = 1;
		maxVal = res[1];
	}

	if (res[2] > maxVal)
	{
		maxIndex = 2;
		maxVal = res[2];
	}

	if (res[3] > maxVal)
	{
		maxIndex = 3;
		maxVal = res[3];
	}

	return maxIndex;
}

void SpxContact::Reset()
{
	m_numContacts = 0;
	for (int i = 0; i < SPX_NUM_CONTACTS; i++)
	{
		m_contactPoints[i].Reset();
	}
}

void SpxContact::RemoveContactPoint(int i)
{
	m_contactPoints[i] = m_contactPoints[m_numContacts - 1];
	m_numContacts--;
}

void SpxContact::Refresh(const glm::vec3& pA, const glm::quat& qA, const glm::vec3& pB, const glm::quat& qB)
{
	// 衝突点の更新
	// 両衝突点間の距離が閾値（CONTACT_THRESHOLD）を超えたら消去
	for (int i = 0; i < (int)m_numContacts; i++)
	{
		glm::vec3 normal = m_contactPoints[i].normal;
		glm::vec3 cpA = pA + glm::rotate(qA, m_contactPoints[i].pointA);
		glm::vec3 cpB = pB + glm::rotate(qB, m_contactPoints[i].pointB);

		// 貫通深度がプラスに転じたかどうかをチェック
		float distance = glm::dot(normal, cpA - cpB);
		if (distance > SPX_CONTACT_THRESHOLD_NORMAL)
		{
			RemoveContactPoint(i);
			i--;
			continue;
		}
		m_contactPoints[i].distance = distance;

		// 深度方向を除去して両点の距離をチェック
		cpA = cpA - m_contactPoints[i].distance * normal;
		float distanceAB = glm::length2(cpA - cpB);
		if (distanceAB > SPX_CONTACT_THRESHOLD_TANGENT)
		{
			RemoveContactPoint(i);
			i--;
			continue;
		}
	}
}

void SpxContact::Merge(const SpxContact& contact)
{
	for (SpxUInt32 i = 0; i < contact.m_numContacts; i++)
	{
		SpxContactPoint& cp = m_contactPoints[i];

		int id = FindNearestContactPoint(cp.pointA, cp.pointB, cp.normal);

		if (id >= 0)
		{
			if (glm::abs(glm::dot(cp.normal, m_contactPoints[id].normal)) > 0.99f)
			{
				// 同一点を発見、蓄積された情報を引き継ぐ
				m_contactPoints[id].distance = cp.distance;
				m_contactPoints[id].pointA = cp.pointA;
				m_contactPoints[id].pointB = cp.pointB;
				m_contactPoints[id].normal = cp.normal;
			}
			else {
				// 同一点ではあるが法線が違うため更新
				m_contactPoints[id] = cp;
			}
		}
		else if (m_numContacts < SPX_NUM_CONTACTS) {
			// 衝突点を新規追加
			m_contactPoints[m_numContacts] = cp;
			m_numContacts++;
		}
		else {
			// ソート
			id = Sort4ContactPoints(cp.pointA, cp.distance);

			// コンタクトポイント入れ替え(新しいやつに交換)
			m_contactPoints[id] = cp;
		}
	}
}

void SpxContact::AddContact(
	float penetrationDepth,
	const glm::vec3& normal,
	const glm::vec3& contactPointA,
	const glm::vec3& contactPointB)
{
	int id = FindNearestContactPoint(contactPointA, contactPointB, normal);

	if (id < 0 && m_numContacts < SPX_NUM_CONTACTS)
	{
		// 衝突点を新規追加
		id = m_numContacts++;
		m_contactPoints[id].Reset();
	}
	else if (id < 0) {
		// すでにこの衝突情報は4つの衝突情報を保持しているため、破棄する衝突点を1つ選択して、それと交換する
		id = Sort4ContactPoints(contactPointA, penetrationDepth);
		m_contactPoints[id].Reset();
	}

	m_contactPoints[id].distance = penetrationDepth;
	m_contactPoints[id].pointA = contactPointA;
	m_contactPoints[id].pointB = contactPointB;
	m_contactPoints[id].normal = normal;
}

};	// namespace SimplePhysics