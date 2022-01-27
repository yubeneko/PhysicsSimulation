#include "SceneLoader.h"
#include "Core.h"
#include "Actor.h"
#include "Renderer.h"
#include "CameraMove.h"

#include "MeshComponent.h"
#include "CameraComponent.h"
#include "CubeGenerator.h"
#include "PresetActor.h"

void SceneLoader::LoadScene(Core& core)
{
	auto cameraActor = core.CreateActor("camera");
	cameraActor->SetPosition(glm::vec3(0, 2, -10));
	// glm::quat q = glm::angleAxis(glm::radians(10.0f), glm::vec3(1.0f, 0.0f, 0.0f)) * cameraActor->GetRotation();
	// cameraActor->SetRotation(q);

	cameraActor->AddComponent<CameraComponent>(100);
	cameraActor->AddComponent<CameraMove>(50);

	auto cube = PresetActor::CreatePreset(
		core, "cube", PresetActor::PresetType::Cube);
	cube->SetPosition(glm::vec3(-3.0f, 0.5f, 3.0f));
	auto meshComponent = cube->GetComponent<MeshComponent>();
	MeshComponent::Material& mat = meshComponent.lock()->GetMaterial();
	mat.mAmbient = glm::vec3(0.1f, 0.1f, 0.9f);
	mat.mDiffuse = glm::vec3(0.1f, 0.1f, 0.9f);
	mat.mSpecular = glm::vec3(0.9f, 0.9f, 0.9f);
	mat.mSmoothness = 100.0f;

	auto sphere = PresetActor::CreatePreset(
		core, "sphere", PresetActor::PresetType::Sphere);
	sphere->SetPosition(glm::vec3(3.0f, 1.0f, 0.0f));
	meshComponent = sphere->GetComponent<MeshComponent>();
	MeshComponent::Material& sphereMat = meshComponent.lock()->GetMaterial();
	sphereMat.mAmbient = glm::vec3(0.9f, 0.1f, 0.9f);
	sphereMat.mDiffuse = glm::vec3(0.9f, 0.1f, 0.9f);
	sphereMat.mSpecular = glm::vec3(0.9f, 0.9f, 0.9f);
	sphereMat.mSmoothness = 40.0f;

	auto plane = PresetActor::CreatePreset(core, "plane", PresetActor::PresetType::Plane);
	plane->SetScale(glm::vec3(5.0f, 0.0f, 5.0f));
}