#pragma once

#include <memory>

class Component
{
public:
	virtual ~Component();

	virtual void ProcessInput(const struct InputState& state);
	virtual void Update(float deltaTime);
	virtual void OnUpdateWorldTransform();

	std::weak_ptr<class Actor> GetOwner() const { return mOwner; }
	int GetUpdateOrder() const { return mUpdateOrder; }
	bool GetEnabled() const { return mIsEnabled; }
	void SetEnabled(bool enabled) { mIsEnabled = enabled; }

protected:
	Component(std::weak_ptr<class Actor> owner,
			  int updateOrder);

protected:
	std::weak_ptr<class Actor> mOwner;
	int mUpdateOrder;
	bool mIsEnabled = true;
};