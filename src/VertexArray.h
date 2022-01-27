#pragma once

class VertexArray
{
public:
	/**
	 * @brief Construct a new Vertex Array object
	 *
	 * @param verts 頂点データを格納した配列
	 * @param numVerts 頂点数
	 * @param numVertElements 1つ頂点の要素数
	 * @param indices インデックスバッファに渡す配列
	 * @param numIndices インデックスの数
	 */
	VertexArray(
		const float* verts,
		unsigned int numVerts,
		unsigned int numVertElements,
		const unsigned int* indices,
		unsigned int numIndices);

	~VertexArray();

	/**
	 * @brief 保持している頂点配列オブジェクトを有効にする
	 */
	void SetActive() const;

	unsigned int GetNumIndices() const { return mNumIndices; }
	unsigned int GetNumVerts() const { return mNumVerts; }

private:
	unsigned int mNumVerts;
	unsigned int mNumIndices;
	unsigned int mVertexBufferID;
	unsigned int mIndexBufferID;
	unsigned int mVertexArrayID;
};