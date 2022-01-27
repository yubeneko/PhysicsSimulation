#include "Component.h"

Component::Component(
	std::weak_ptr<Actor> owner,
	int updateOrder)
  : mOwner(owner),
	mUpdateOrder(updateOrder)
{
}

Component::~Component()
{
}

void Component::ProcessInput(const InputState& state) {}
void Component::Update(float deltaTime) {}
void Component::OnUpdateWorldTransform() {}