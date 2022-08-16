#include "Engine/Scene/SimpleECS/SimpleEntity.h"

namespace Engine
{
	SimpleEntity::SimpleEntity(PrimitiveType primitiveType, const std::string& shaderName, const std::string& entityName)
		:m_EntityName(entityName)
	{
		m_EntityRenderer = CreateRef<EntityRenderer>(primitiveType, shaderName, entityName);
		m_EntityTransform = CreateRef<EntityTransform>();
	}

	SimpleEntity::SimpleEntity(const Ref<Mesh>& mesh, const std::string& shaderName, const std::string& entityName)
		:m_EntityName(entityName)
	{
		m_EntityRenderer = CreateRef<EntityRenderer>(mesh, shaderName, entityName);
		m_EntityTransform = CreateRef<EntityTransform>();
	}

	SimpleEntity::SimpleEntity(const Ref<Mesh>& mesh)
		:m_EntityName("Entity")
	{
		m_EntityRenderer = CreateRef<EntityRenderer>(mesh);
		m_EntityTransform = CreateRef<EntityTransform>();
	}

	void SimpleEntity::Draw(const Camera& Camera, const Ref<Light>& Light) const
	{
		m_EntityRenderer->Draw(Camera, Light, m_EntityTransform);
	}

	void SimpleEntity::DrawEntity(const glm::mat4& viewProjection) const
	{
		m_EntityRenderer->Begin();
		m_EntityRenderer->GetShader()->UploadUniformMat4("u_MVP", viewProjection * m_EntityTransform->Transform());
		m_EntityRenderer->Draw();
		m_EntityRenderer->End();
	}

	void SimpleEntity::DrawEntityPoints(const glm::mat4& viewProjection) const
	{
		m_EntityRenderer->Begin();
		m_EntityRenderer->GetShader()->UploadUniformMat4("u_MVP", viewProjection * m_EntityTransform->Transform());
		m_EntityRenderer->DrawPoints();
		m_EntityRenderer->End();
	}
}

