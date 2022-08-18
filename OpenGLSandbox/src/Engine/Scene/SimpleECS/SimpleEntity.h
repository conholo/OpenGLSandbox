#pragma once

#include "Engine/Scene/SimpleECS/EntityRenderer.h"
#include "Engine/Scene/SimpleECS/EntityTransform.h"
#include "Engine/Core/Memory.h"
#include "Engine/Rendering/Camera.h"

namespace Engine
{
	class SimpleEntity
	{
	public:
		SimpleEntity(PrimitiveType primitiveType, const std::string& shaderName, const std::string& entityName = "Entity");
		SimpleEntity(const Ref<Mesh>& mesh, const std::string& shaderName, const std::string& entityName = "Entity");
		SimpleEntity(const Ref<Mesh>& mesh);

		void Draw(const Camera& Camera, const Ref<Light>& Light) const;
		void DrawEntity(const glm::mat4& viewProjection) const;
		void DrawEntityPoints(const glm::mat4& viewProjection) const;

		const Ref<EntityRenderer>& GetEntityRenderer() const { return m_EntityRenderer; }
		const Ref<EntityTransform>& GetEntityTransform() const { return m_EntityTransform; }

		const std::string& GetName() const { return m_EntityName; }
	private:
		std::string m_EntityName;
		Ref<EntityRenderer> m_EntityRenderer;
		Ref<EntityTransform> m_EntityTransform;
	};
}

