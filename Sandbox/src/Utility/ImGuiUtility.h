#pragma once

#include <string>
#include <glm/glm.hpp>

class ImGuiUtility
{
public:

	static void DrawVec3Controls(const std::string& label, glm::vec3& values, float resetValue = 0.0f, float columnWidth = 100.0f);
};