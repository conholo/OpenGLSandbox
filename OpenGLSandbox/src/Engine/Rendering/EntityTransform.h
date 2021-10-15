#pragma once

#include <glm/glm.hpp>

namespace Engine
{
	class EntityTransform
	{
	public:
		glm::vec3& GetPosition() { return m_Position; }
		glm::vec3& GetRotation() { return m_Rotation; }
		glm::vec3& GetScale() { return m_Scale; }

		void SetPosition(const glm::vec3& position) { m_Position = position; }
		void SetRotation(const glm::vec3& rotation) { m_Rotation = rotation; }
		void SetScale(const glm::vec3& scale) { m_Scale = scale; }

		glm::mat4 Transform();
	private:
		glm::vec3 m_Position{ 0.0f };
		glm::vec3 m_Rotation{ 0.0f };
		glm::vec3 m_Scale{ 1.0f };
	};
}

