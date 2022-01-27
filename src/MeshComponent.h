#pragma once

#include "Component.h"
#include <memory>
#include <glm/glm.hpp>
#include <string>

class MeshComponent : public Component, public std::enable_shared_from_this<MeshComponent>
{
public:
	struct Material
	{
		glm::vec3 mAmbient;
		glm::vec3 mDiffuse;
		glm::vec3 mSpecular;
		float mSmoothness;
		int mShaderType; // Renderer::ShaderType
	};

	static std::shared_ptr<MeshComponent> CreateComponent(
		std::weak_ptr<class Actor> owner,
		int updateOrder);

	~MeshComponent();

	void Draw(class Shader& shader, bool isDepthRendering = false);
	void SetMesh(std::weak_ptr<class Mesh> mesh) { mMesh = mesh; }
	Material& GetMaterial() { return mMaterial; }

	// テクスチャのセット
	void SetTexture(std::weak_ptr<class Texture> tex) { mTexture = tex; }

	// Renderer に自分自身を登録する
	// これを呼ばないと描画されない。また、マテリアルのシェーダーを切り替えたときにも呼ぶ必要がある。
	void RegisterToRenderer();

private:
	MeshComponent(std::weak_ptr<class Actor> owner, int updateOrder);

	std::weak_ptr<class Mesh> mMesh;
	Material mMaterial;

	// テクスチャを1つだけ持つ
	std::weak_ptr<class Texture> mTexture;
};