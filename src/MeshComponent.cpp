#include "MeshComponent.h"
#include "Shader.h"
#include "Actor.h"
#include "VertexArray.h"
#include "Mesh.h"
#include "Core.h"
#include "Renderer.h"
#include "Texture.h"

#include <glm/gtc/type_ptr.hpp>

MeshComponent::MeshComponent(
	std::weak_ptr<class Actor> owner,
	int updateOrder)
  : Component(owner, updateOrder)
{
}

std::shared_ptr<MeshComponent> MeshComponent::CreateComponent(
	std::weak_ptr<class Actor> owner,
	int updateOrder)
{
	return std::shared_ptr<MeshComponent>(new MeshComponent(owner, updateOrder));
}

MeshComponent::~MeshComponent()
{
}

void MeshComponent::Draw(Shader& shader, bool isDepthRendering)
{
	std::shared_ptr<Mesh> mesh = mMesh.lock();
	if (!mesh) { return; }

	// モデル行列のセット
	std::shared_ptr<Actor> owner = mOwner.lock();
	shader.SetMatrixUniform("uModelMat", glm::value_ptr(owner->GetWorldTransform()));

	// 深度のレンダリングじゃなければマテリアルやテクスチャのセットを行う
	if (!isDepthRendering)
	{
		// 特定のシェーダーならばマテリアルをセットする
		if (mMaterial.mShaderType == static_cast<int>(Renderer::ShaderType::ShadowMapping))
		{
			shader.SetVector3Uniform("uMaterial.color", glm::value_ptr(mMaterial.mAmbient));
			shader.SetVector3Uniform("uMaterial.specularColor", glm::value_ptr(mMaterial.mSpecular));
			shader.SetFloatUniform("uMaterial.smoothness", mMaterial.mSmoothness);
		}

		auto tex = mTexture.lock();
		if (tex)
		{
			tex->SetActive();
		}
	}

	// 頂点配列有効化
	const VertexArray& vao = mesh->GetVertexArray();
	vao.SetActive();

	// 描画
	glDrawElements(GL_TRIANGLES, vao.GetNumIndices(), GL_UNSIGNED_INT, nullptr);
}

void MeshComponent::RegisterToRenderer()
{
	mOwner.lock()->GetCore().GetRenderer().AddMeshComponent(weak_from_this());
}