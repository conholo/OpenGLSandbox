#pragma once

#include <string>
#include "Engine/Rendering/EntityRenderer.h"
#include "Engine/Rendering/EntityTransform.h"
#include "Engine/Core/Memory.h"

namespace Engine
{
	class Entity
	{
	public:
		Entity(PrimitiveType primitiveType, const std::string& shaderName);

		void DrawEntity(const glm::mat4& viewProjection);

		const Ref<EntityRenderer>& GetEntityRenderer() const { return m_EntityRenderer; }
		const Ref<EntityTransform>& GetEntityTransform() const { return m_EntityTransform; }

	private:
		Ref<EntityRenderer> m_EntityRenderer;
		Ref<EntityTransform> m_EntityTransform;
	};
}

