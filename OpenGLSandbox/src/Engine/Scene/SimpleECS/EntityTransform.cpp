#include "Engine/Scene/SimpleECS/EntityTransform.h"

#include <glm/gtc/matrix_transform.hpp>


namespace Engine
{

	void EntityTransform::LookAt(const glm::vec3& target)
	{
		const glm::vec3 directionToTarget = glm::normalize(m_Position - target);
		m_Rotation.x = -glm::degrees(glm::asin(directionToTarget.y));
		m_Rotation.y = glm::degrees(std::atan2(directionToTarget.x, directionToTarget.z));
	}

	glm::quat EntityTransform::Orientation() const
	{
		return glm::quat(glm::radians(glm::vec3(m_Rotation.x, m_Rotation.y, 0.0f)));
	}

	glm::vec3 EntityTransform::Up() const
	{
		return glm::rotate(Orientation(), glm::vec3(0.0f, 1.0f, 0.0f));
	}

	glm::vec3 EntityTransform::Right() const
	{
		return glm::rotate(Orientation(), glm::vec3(1.0f, 0.0f, 0.0f));
	}

	glm::vec3 EntityTransform::Forward() const
	{
		return glm::rotate(Orientation(), glm::vec3(0.0f, 0.0f, -1.0f));
	}

	glm::mat4 EntityTransform::Transform() const
	{
		const glm::mat4 transform =
			glm::translate(glm::mat4(1.0f), m_Position) *
			glm::toMat4(glm::quat(glm::radians(m_Rotation))) *
			glm::scale(glm::mat4(1.0f), m_Scale);

		return transform;
	}
}
