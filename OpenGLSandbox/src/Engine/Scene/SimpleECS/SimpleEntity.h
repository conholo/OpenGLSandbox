#pragma once

#include <string>
#include "Engine/Scene/SimpleECS/EntityRenderer.h"
#include "Engine/Scene/SimpleECS/EntityTransform.h"
#include "Engine/Core/Memory.h"

namespace Engine
{
	class SimpleEntity
	{
	public:
		SimpleEntity(PrimitiveType primitiveType, const std::string& shaderName);
		SimpleEntity(const Ref<Mesh>& mesh, const std::string& shaderName);

		void DrawEntity(const glm::mat4& viewProjection);
		void DrawEntityPoints(const glm::mat4& viewProjection);

		const Ref<EntityRenderer>& GetEntityRenderer() const { return m_EntityRenderer; }
		const Ref<EntityTransform>& GetEntityTransform() const { return m_EntityTransform; }

	private:
		Ref<EntityRenderer> m_EntityRenderer;
		Ref<EntityTransform> m_EntityTransform;
	};
}

