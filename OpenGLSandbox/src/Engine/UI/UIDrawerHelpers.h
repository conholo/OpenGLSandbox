#pragma once


namespace Engine
{
	namespace UI
	{
		void DrawVector3Field(const std::string& label, glm::vec3& value, float resetValue = 0.0f);
		void DrawVector3FieldTable(const std::string& label, glm::vec3& value, float resetValue = 0.0f);
		void DrawVector4Field(const std::string& label, glm::vec4& value, float resetValue = 0.0f);
		void DrawVector4Field2(const std::string& label, glm::vec4& value, float resetValue = 1.0f);;
	}
}