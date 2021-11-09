#include "Engine/Scene/SimpleECS/Light.h"
#include "Engine/Rendering/Mesh.h"
#include "Engine/Rendering/Shader.h"

namespace Engine
{
	Light::Light(const LightSpecification& specification)
		:m_Specification(specification)
	{
		m_Transform = CreateRef<EntityTransform>();
		m_Transform->SetScale({ 0.25f, 0.25f, 0.25f });
		m_DebugRenderer = CreateRef<EntityRenderer>(specification.DebugShape, "FlatColor");
		m_WhiteTexture = Texture2D::CreateWhiteTexture();
	}

	const glm::vec3& Light::GetViewSpaceVector(const glm::mat4& viewMatrix) const
	{
		float w = m_Specification.Type == LightType::Directional ? 0.0f : 1.0;
		return glm::vec3(viewMatrix * glm::vec4(m_Transform->GetPosition(), w));
	}

	void Light::DrawDebug(const glm::mat4& viewProjection)
	{
		m_WhiteTexture->BindToSamplerSlot(0);
		m_DebugRenderer->Begin();
		m_DebugRenderer->GetShader()->UploadUniformFloat3("u_Color", m_Specification.LightColor);
		m_DebugRenderer->GetShader()->UploadUniformMat4("u_MVP", viewProjection * m_Transform->Transform());
		m_DebugRenderer->Draw();
		m_DebugRenderer->End();
		m_WhiteTexture->Unbind();
	}
}

