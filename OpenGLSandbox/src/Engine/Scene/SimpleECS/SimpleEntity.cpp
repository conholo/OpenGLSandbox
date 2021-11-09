#include "Engine/Scene/SimpleECS/SimpleEntity.h"

namespace Engine
{
	SimpleEntity::SimpleEntity(PrimitiveType primitiveType, const std::string& shaderName)
	{
		m_EntityRenderer = CreateRef<EntityRenderer>(primitiveType, shaderName);
		m_EntityTransform = CreateRef<EntityTransform>();
	}

	SimpleEntity::SimpleEntity(const Ref<Mesh>& mesh, const std::string& shaderName)
	{
		m_EntityRenderer = CreateRef<EntityRenderer>(mesh, shaderName);
		m_EntityTransform = CreateRef<EntityTransform>();
	}

	void SimpleEntity::DrawEntity(const glm::mat4& viewProjection)
	{
		m_EntityRenderer->Begin();
		m_EntityRenderer->GetShader()->UploadUniformMat4("u_MVP", viewProjection * m_EntityTransform->Transform());
		m_EntityRenderer->Draw();
		m_EntityRenderer->End();
	}
}

