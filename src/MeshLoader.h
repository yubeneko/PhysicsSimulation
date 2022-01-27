#pragma once

#include <string>
#include <vector>

namespace MeshLoader
{
	struct UV
	{
		float u;
		float v;
	};

	struct Vector3
	{
		float x;
		float y;
		float z;
	};

	struct Obj
	{
		/**
		 * @brief Objファイルをロードする。ただし、三角ポリゴンで構成されたメッシュを想定するので、
		 * モデリングソフトからエクスポートする時に三角ポリゴンとして吐き出させるようにすること。
		 *
		 * @param path ファイルパス
		 * @param outVertices 頂点バッファ(頂点座標、法線ベクトル、UV座標が連続で来る。法線ベクトル、UV座標はないこともある)
		 * @param outIndices インデックスバッファ
		 * @return true
		 * @return false
		 */
		static bool LoadMesh(
			const std::string& path,
			unsigned int& numVerts,
			std::vector<float>& outVertices,
			std::vector<unsigned int>& outIndices);

		// 単純な正方形の場合の頂点バッファとインデックスバッファ
		// 頂点バッファの並び
		// vx, vy, vz, nx, ny, nz, u, v の順。最大で8個。
		// float vertices[] = {
		// 	-0.5f, 0.5f, 0.f, 0.f, 0.f, 0.0f, 0.f, 0.f,	 // top left
		// 	0.5f, 0.5f, 0.f, 0.f, 0.f, 0.0f, 1.f, 0.f,	 // top right
		// 	0.5f, -0.5f, 0.f, 0.f, 0.f, 0.0f, 1.f, 1.f,	 // bottom right
		// 	-0.5f, -0.5f, 0.f, 0.f, 0.f, 0.0f, 0.f, 1.f	 // bottom left
		// };

		// unsigned int indices[] = {
		// 	0, 1, 2,
		// 	2, 3, 0};
	};
};