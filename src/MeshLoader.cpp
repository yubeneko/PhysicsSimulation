#include "MeshLoader.h"

#include <vector>
#include <unordered_map>
#include <fstream>
#include <sstream>

bool MeshLoader::Obj::LoadMesh(const std::string& path,
							   unsigned int& numVerts,
							   std::vector<float>& outVertices,
							   std::vector<unsigned int>& outIndices)
{
	std::ifstream ifs(path);
	if (!ifs) { return false; }

	// 頂点数
	unsigned int vertCount = 0;
	// 読み込んだ頂点
	std::vector<Vector3> vertices;
	// 読み込んだ頂点法線
	std::vector<Vector3> vertNormals;
	// 読み込んだ頂点UV
	std::vector<UV> vertUVs;
	// 頂点のインデックスと頂点法線のインデックスの連想配列(インデックスは1から始まる)
	std::unordered_map<unsigned int, unsigned int> vertexNormalDict;
	// 頂点のインデックスとUV座標のインデックスの連想配列(インデックスは1から始まる)
	std::unordered_map<unsigned int, unsigned int> vertexUVDict;

	// 読み込んだ1行の格納先
	std::string oneline;

	while (!ifs.eof())
	{
		// 1行ずつ読み込む
		std::getline(ifs, oneline);

		if (oneline.length() == 0) { continue; }
		if (oneline[0] == '#') { continue; }

		// 1行に含まれる文字列をスペース区切りで分割する
		std::istringstream iss(oneline);
		std::string bufferSegment;
		std::vector<std::string> values;
		values.reserve(4);
		while (std::getline(iss, bufferSegment, ' '))
		{
			values.emplace_back(bufferSegment);
		}

		if (values[0] == "v")
		{
			// 頂点
			Vector3 v{std::stof(values[1]), std::stof(values[2]),
					  std::stof(values[3])};
			vertices.push_back(v);
			++vertCount;
		}
		else if (values[0] == "vn") {
			// 頂点法線
			Vector3 n{std::stof(values[1]), std::stof(values[2]),
					  std::stof(values[3])};
			vertNormals.push_back(n);
		}
		else if (values[0] == "vt") {
			// UV
			// Vについては、OpenGLは特殊でVの0が上から始まっている。
			// しかし、一般的にはVは下から始まっているので、V要素を逆転させる必要がある。
			UV uv{std::stof(values[1]), 1.0f - std::stof(values[2])};
			vertUVs.push_back(uv);
		}
		else if (values[0] == "f") {
			// 面
			for (int i = 1; i < 4; i++)
			{
				// パターン
				// "10/3/4", // 全部あり(頂点ID/UV/法線)
				// "1//2", // テクスチャなし(頂点ID//法線)
				// "5/9", // 法線なし(頂点ID/UV)
				// "0", // 頂点だけ

				// 1行に含まれる文字列をスラッシュ(/)区切りで分割する
				std::istringstream iss(values[i]);
				std::string buf;
				std::vector<std::string> indices;
				while (std::getline(iss, buf, '/'))
				{
					indices.emplace_back(buf);
				}

				unsigned int vertexIndex = std::stoi(indices[0]);

				// 法線だけがない場合
				if (indices.size() == 2)
				{
					vertexUVDict[vertexIndex] = std::stoi(indices[1]);
				}
				// テクスチャだけがない場合
				else if (indices.size() == 3 && indices[1] == "")
				{
					vertexNormalDict[vertexIndex] = std::stoi(indices[2]);
				}
				// どっちもある場合
				else
				{
					vertexUVDict[vertexIndex] = std::stoi(indices[1]);
					vertexNormalDict[vertexIndex] = std::stoi(indices[2]);
				}

				// インデックスバッファに追加
				outIndices.push_back(vertexIndex - 1);
			}
		}
	}

	// 頂点配列の構築
	for (unsigned int i = 1; i <= vertices.size(); i++)
	{
		Vector3 v = vertices[i - 1];
		outVertices.push_back(v.x);
		outVertices.push_back(v.y);
		outVertices.push_back(v.z);

		// モデルが三角ポリゴンで構成されていないとこの辺でエラーが出るぞ!
		if (!vertNormals.empty())
		{
			unsigned int normIndex = vertexNormalDict[i];
			Vector3 n = vertNormals[normIndex - 1];
			outVertices.push_back(n.x);
			outVertices.push_back(n.y);
			outVertices.push_back(n.z);
		}

		if (!vertUVs.empty())
		{
			unsigned int uvIndex = vertexUVDict[i];
			UV uv = vertUVs[uvIndex - 1];
			outVertices.push_back(uv.u);
			outVertices.push_back(uv.v);
		}
	}

	numVerts = vertCount;

	return true;
}