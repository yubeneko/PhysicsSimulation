#include "PresetActor.h"
#include "Core.h"
#include "Actor.h"
#include "Renderer.h"
#include "MeshComponent.h"

std::shared_ptr<class Actor> PresetActor::CreatePreset(Core& core, const std::string& id, PresetType type)
{
	auto actor = core.CreateActor(id);
	Renderer& renderer = core.GetRenderer();
	auto meshComponent = actor->AddComponent<MeshComponent>();

	MeshComponent::Material& mat = meshComponent->GetMaterial();
	mat.mAmbient = glm::vec3(1.0f, 1.0f, 1.0f);
	mat.mDiffuse = glm::vec3(1.0f, 1.0f, 1.0f);
	mat.mSpecular = glm::vec3(0.3f, 0.3f, 0.3f);
	mat.mSmoothness = 1.0f;

	switch (type)
	{
		case PresetType::Cube:
			meshComponent->SetMesh(renderer.GetMesh("Assets/cube.obj"));
			mat.mShaderType = Renderer::ShaderType::ShadowMapping;
			break;

		case PresetType::Sphere:
			meshComponent->SetMesh(renderer.GetMesh("Assets/sphere.obj"));
			mat.mShaderType = Renderer::ShaderType::ShadowMapping;
			break;

		case PresetType::Plane:
			meshComponent->SetMesh(renderer.GetMesh("Assets/plane.obj"));
			mat.mShaderType = Renderer::ShaderType::ShadowMapping;
			break;

		case PresetType::Quad:
			meshComponent->SetMesh(renderer.GetMesh("Assets/quad.obj"));
			mat.mShaderType = Renderer::ShaderType::ShadowMapping;
			break;

		default:
			break;
	}
	meshComponent->RegisterToRenderer();

	return actor;
}