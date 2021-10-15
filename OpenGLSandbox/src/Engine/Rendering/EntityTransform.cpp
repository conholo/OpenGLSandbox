#include "Engine/Rendering/EntityTransform.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>


namespace Engine
{
	glm::mat4 EntityTransform::Transform()
	{
		glm::mat4 transform =
			glm::translate(glm::mat4(1.0f), m_Position) *
			glm::toMat4(glm::quat(glm::radians(m_Rotation))) *
			glm::scale(glm::mat4(1.0f), m_Scale);

		return transform;
	}
}
