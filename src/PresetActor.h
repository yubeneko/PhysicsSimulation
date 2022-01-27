#pragma once

#include <memory>
#include <string>

class PresetActor
{
public:
	enum PresetType
	{
		Cube,
		Sphere,
		Plane,
		Quad,
	};

	static std::shared_ptr<class Actor> CreatePreset(class Core& core, const std::string& id, PresetType type);
};