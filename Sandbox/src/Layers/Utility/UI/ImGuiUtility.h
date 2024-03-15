#pragma once

#include <string>
#include <glm/glm.hpp>

#include "Engine/Core/Memory.h"
#include "Engine/Rendering/Material.h"

class ImGuiUtility
{
public:

	static void DrawVec3Controls(const std::string& label, glm::vec3& values, float resetValue = 0.0f, float columnWidth = 100.0f);
	static void DrawMaterialInspector(const Engine::Ref<Engine::Material>& Material);
};
