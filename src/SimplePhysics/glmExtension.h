#pragma once

#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>

namespace GLMExtension
{

// 全てテスト済み

// 2つのベクトルのそれぞれの要素を比較して、大きい方の要素を採用した3次元ベクトルを作る
inline glm::vec3 MaxPerElem(const glm::vec3& v0, const glm::vec3& v1)
{
	return glm::vec3(
		v0.x > v1.x ? v0.x : v1.x,
		v0.y > v1.y ? v0.y : v1.y,
		v0.z > v1.z ? v0.z : v1.z);
}

// 2つのベクトルのそれぞれの要素を比較して、小さい方の要素を採用した3次元ベクトルを作る
inline glm::vec3 MinPerElem(const glm::vec3& v0, const glm::vec3& v1)
{
	return glm::vec3(
		v0.x < v1.x ? v0.x : v1.x,
		v0.y < v1.y ? v0.y : v1.y,
		v0.z < v1.z ? v0.z : v1.z);
}

// 受け取った3x3行列の要素の絶対値を要素とする3x3行列をえる
inline glm::mat3 AbsPerElem(const glm::mat3& mat)
{
	return glm::mat3(
		glm::abs(glm::vec3(mat[0][0], mat[0][1], mat[0][2])),
		glm::abs(glm::vec3(mat[1][0], mat[1][1], mat[1][2])),
		glm::abs(glm::vec3(mat[2][0], mat[2][1], mat[2][2])));
}

// 3行4列のアフィン変換を作る
// 3x3の部分が回転を表し、一番左の一列は並進ベクトルを表す
inline glm::mat4x3 To3x4TransformMat(const glm::quat& q, const glm::vec3& v)
{
	// 3x3の回転行列
	glm::mat3 rot = glm::toMat3(q);
	glm::mat4x3 T(rot);
	T[3][0] = v.x;
	T[3][1] = v.y;
	T[3][2] = v.z;
	return T;
}

// アフィン変換同士の掛け算
inline glm::mat4x3 AffineTransformMultiply(const glm::mat4x3& A,
									const glm::mat4x3& B)
{
	glm::vec3 col0 = glm::mat3(A) * glm::vec3(B[0][0], B[0][1], B[0][2]);
	glm::vec3 col1 = glm::mat3(A) * glm::vec3(B[1][0], B[1][1], B[1][2]);
	glm::vec3 col2 = glm::mat3(A) * glm::vec3(B[2][0], B[2][1], B[2][2]);
	glm::vec3 col3 = glm::mat3(A) * glm::vec3(B[3][0], B[3][1], B[3][2]) +
					 glm::vec3(A[3][0], A[3][1], A[3][2]);

	return glm::mat4x3(col0, col1, col2, col3);
}

// アフィン変換の並進ベクトル成分を得る
inline glm::vec3 GetTranslation(const glm::mat4x3& T)
{
	return glm::vec3(T[3][0], T[3][1], T[3][2]);
}

// アフィン変換の逆行列を作る
// 左の3x3成分が直行行列(回転成分しかない)場合限定で使える
inline glm::mat4x3 OrthoInverse(const glm::mat4x3 transform)
{
	// 回転行列の成分の転置行列
	glm::mat3 rot_transpose = glm::transpose(glm::mat3((transform)));
	glm::vec3 translation = -1.0f * rot_transpose * GetTranslation(transform);
	return glm::mat4x3(
		glm::vec3(rot_transpose[0][0], rot_transpose[0][1], rot_transpose[0][2]),
		glm::vec3(rot_transpose[1][0], rot_transpose[1][1], rot_transpose[1][2]),
		glm::vec3(rot_transpose[2][0], rot_transpose[2][1], rot_transpose[2][2]),
		translation);
}

// 外積行列(歪対称行列)を作る
inline glm::mat3 CrossMatrix(const glm::vec3& vec)
{
	return glm::mat3(
		glm::vec3(0.0f, vec.z, -vec.y),
		glm::vec3(-vec.z, 0.0f, vec.x),
		glm::vec3(vec.y, -vec.x, 0.0f));
}

};	// namespace GLMExtension