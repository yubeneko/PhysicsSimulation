#pragma once

#include "../SpxBase.h"

namespace SimplePhysics
{

/**
 * @brief 1つのベクトルをベースにして2つの基底ベクトルを作る
 *
 * @param normal 元になるベクトル
 * @param tangent1 生成されるベクトルその1
 * @param tangent2 生成されるベクトルその2
 */
static inline void SpxCalcTangentVector(const glm::vec3& normal, glm::vec3& tangent1, glm::vec3& tangent2)
{
	// normal と異なる適当なベクトルを1つとる
	glm::vec3 vec(1.0f, 0.0f, 0.0f);
	glm::vec3 n(normal);
	n.x = 0.0f;
	// n.x を0にした結果、ベクトルの大きさの2乗がほぼ0になった -> normal が (1,0,0) だったということになる。
	// normal と vec は異なるベクトルでなければならないので、変える
	if (glm::length2(n) < SPX_EPSILON)
	{
		vec = glm::vec3(0.0f, 1.0f, 0.0f);
	}
	// 2つの基底ベクトルを作る
	tangent1 = normalize(cross(normal, vec));
	tangent2 = normalize(cross(tangent1, normal));
}
};	// namespace SimplePhysics