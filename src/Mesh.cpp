#include "Mesh.h"
#include "VertexArray.h"
#include "MeshLoader.h"

#include <SDL.h>

Mesh::Mesh()
  : mVertexArray(nullptr)
{
}

Mesh::~Mesh()
{
}

template <typename LoadPolicy>
bool Mesh::Load(const std::string& path)
{
	// 選択したポリシーのメンバ関数で
	// 頂点の配列、法線の配列、インデックスの配列を作って戻す
	unsigned int numVertices;
	std::vector<float> vertices;
	std::vector<unsigned int> indices;
	if (!LoadPolicy::LoadMesh(path, numVertices, vertices, indices))
	{
		return false;
	}

	// VAO を作成
	mVertexArray = new VertexArray(
		vertices.data(),
		numVertices,
		vertices.size() / numVertices,
		indices.data(),
		indices.size());

	return true;
}

void Mesh::Unload()
{
	delete mVertexArray;
	mVertexArray = nullptr;
}

// 明治的なインスタンス化を行うことでメンバテンプレートをソースファイルに書く
template bool Mesh::Load<MeshLoader::Obj>(const std::string& path);