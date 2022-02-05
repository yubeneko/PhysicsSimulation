#include "SceneLoader.h"
#include "Core.h"
#include "Actor.h"
#include "Renderer.h"
#include "CameraMove.h"

#include "MeshComponent.h"
#include "CameraComponent.h"
#include "CubeGenerator.h"
#include "PresetActor.h"
#include "RigidBody.h"

void SceneLoader::LoadScene(Core& core)
{
	auto cameraActor = core.CreateActor("camera");
	cameraActor->SetPosition(glm::vec3(0, 2, -10));
	// glm::quat q = glm::angleAxis(glm::radians(10.0f), glm::vec3(1.0f, 0.0f, 0.0f)) * cameraActor->GetRotation();
	// cameraActor->SetRotation(q);

	cameraActor->AddComponent<CameraComponent>(100);
	cameraActor->AddComponent<CameraMove>(50);

	for (int i = 0; i < 5; i++)
	{
		auto cube = PresetActor::CreatePreset(
			core, "cube", PresetActor::PresetType::Cube);
		cube->SetPosition(glm::vec3(0.0f, 4.0f + i * 1.5f, 3.0f));
		auto meshComponent = cube->GetComponent<MeshComponent>();
		MeshComponent::Material& mat = meshComponent.lock()->GetMaterial();
		mat.mAmbient = glm::vec3(0.1f, 0.1f, 0.9f);
		mat.mDiffuse = glm::vec3(0.1f, 0.1f, 0.9f);
		mat.mSpecular = glm::vec3(0.9f, 0.9f, 0.9f);
		mat.mSmoothness = 100.0f;

		auto rb = cube->AddComponent<RigidBody>();
	}

	{
		auto plane = PresetActor::CreatePreset(core, "plane", PresetActor::PresetType::Cube);
		plane->SetScale(glm::vec3(20.0f, 1.0f, 20.0f));
		auto rb = plane->AddComponent<RigidBody>();
		rb->SetMotionType(SimplePhysics::SpxMotionType::SpxMotionTypeStatic);
	}

	// 軸の確認
	// {
	// 	auto axisObj = core.CreateActor("axis");
	// 	auto meshComponent = axisObj->AddComponent<MeshComponent>();

	// 	MeshComponent::Material& mat = meshComponent->GetMaterial();
	// 	mat.mAmbient = glm::vec3(1.0f, 1.0f, 1.0f);
	// 	mat.mDiffuse = glm::vec3(1.0f, 1.0f, 1.0f);
	// 	mat.mSpecular = glm::vec3(0.3f, 0.3f, 0.3f);
	// 	mat.mSmoothness = 1.0f;
	// 	meshComponent->SetMesh(core.GetRenderer().GetMesh("Assets/axis.obj"));
	// 	mat.mShaderType = Renderer::ShaderType::ShadowMapping;
	// 	meshComponent->RegisterToRenderer();

	// 	axisObj->SetPosition(glm::vec3(0.0f, 2.0f, 0.0f));
	// }
}