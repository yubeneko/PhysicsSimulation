#pragma once

#include "../SpxBase.h"

namespace SimplePhysics
{

// 拘束ソルバーの必要なパラメータを抽出してキャッシュするためのデータ構造
struct SpxSolverBody
{
	glm::vec3 deltaLinearVelocity;	 // 拘束の演算の結果、拘束力により更新される並進速度の差分
	glm::vec3 deltaAngularVelocity;	 //拘束の演算の結果、拘束力により更新される回転速度の差分
	glm::quat orientation;			 // 姿勢
	glm::mat3 inertiaInv;			 // 慣性テンソルの逆行列
	float massInv;					 // 質量の逆数
};

};	// namespace SimplePhysics