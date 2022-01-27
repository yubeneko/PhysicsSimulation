#pragma once

#include <vector>
#include <string>

// 前方宣言
namespace MeshLoader
{
	struct Obj;
};

class Mesh
{
public:
	Mesh();
	~Mesh();

	template <typename LoadPolicy = MeshLoader::Obj>
	bool Load(const std::string& path);

	void Unload();
	const class VertexArray& GetVertexArray() const { return *mVertexArray; };

private:
	class VertexArray* mVertexArray;
};