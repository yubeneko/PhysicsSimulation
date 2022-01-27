#pragma once

#include "Component.h"

template <typename T>
std::shared_ptr<T> Actor::AddComponent(int updateOrder)
{
	auto comp = T::CreateComponent(
		std::weak_ptr<Actor>(shared_from_this()),
		updateOrder);

	auto iter = mComponents.begin();
	for (; iter != mComponents.end(); ++iter)
	{
		if (updateOrder < (*iter)->GetUpdateOrder())
		{
			break;
		}
	}

	mComponents.insert(iter, comp);
	return comp;
}

template <typename T>
std::weak_ptr<T> Actor::GetComponent()
{
	for (auto comp : mComponents)
	{
		auto rawComp = comp.get();
		if (typeid(*rawComp) == typeid(T))
		{
			return std::dynamic_pointer_cast<T>(comp);
		}
	}

	return std::weak_ptr<T>();
}