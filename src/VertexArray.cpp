#include "VertexArray.h"
#include <GL/glew.h>

VertexArray::VertexArray(
	const float* verts,
	unsigned int numVerts,
	unsigned int numVertElements,
	const unsigned int* indices,
	unsigned int numIndices)

  : mNumVerts(numVerts),
	mNumIndices(numIndices)
{
	glGenVertexArrays(1, &mVertexArrayID);
	glBindVertexArray(mVertexArrayID);

	glGenBuffers(1, &mVertexBufferID);
	glBindBuffer(GL_ARRAY_BUFFER, mVertexBufferID);
	glBufferData(GL_ARRAY_BUFFER, numVerts * numVertElements * sizeof(float), verts, GL_STATIC_DRAW);

	glGenBuffers(1, &mIndexBufferID);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mIndexBufferID);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, numIndices * sizeof(int), indices, GL_STATIC_DRAW);

	// 頂点属性その1: 3次元頂点座標
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * numVertElements, 0);

	// 追加で頂点属性があるかどうか調べる
	switch (numVertElements)
	{
		case 5:	 // 頂点座標 + UV座標
			glEnableVertexAttribArray(1);
			glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(float) * numVertElements,
								  reinterpret_cast<void*>(sizeof(float) * 3));
			break;
		case 6:	 // 頂点座標 + 法線ベクトル
			glEnableVertexAttribArray(1);
			glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(float) * numVertElements,
								  reinterpret_cast<void*>(sizeof(float) * 3));
			break;
		case 8:	 // 頂点座標 + 法線ベクトル + UV座標
			glEnableVertexAttribArray(1);
			glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(float) * numVertElements,
								  reinterpret_cast<void*>(sizeof(float) * 3));

			glEnableVertexAttribArray(2);
			glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(float) * numVertElements,
								  reinterpret_cast<void*>(sizeof(float) * 6));
			break;
		default:
			break;
	}
}

VertexArray::~VertexArray()
{
	glDeleteBuffers(1, &mVertexBufferID);
	glDeleteBuffers(1, &mIndexBufferID);
	glDeleteVertexArrays(1, &mVertexArrayID);
}

void VertexArray::SetActive() const
{
	glBindVertexArray(mVertexArrayID);
}