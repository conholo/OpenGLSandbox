#include "Planet.h"

Planet::Planet(const PlanetProperties& properties)
	:m_Properties(properties)
{
	m_Transform = Engine::CreateRef<Engine::EntityTransform>();
	m_Transform->SetScale(m_Properties.Scale * m_Properties.Radius);

	m_PlanetMesh = Engine::MeshFactory::Icosphere(6, 1.0f);

	m_Renderer = Engine::CreateRef<Engine::EntityRenderer>(m_PlanetMesh, "Planet");
	m_MaterialPropertiesUBO = Engine::CreateRef<Engine::UniformBuffer>(sizeof(PlanetMaterialProperties), 0);

	SetRadius(properties.Radius);

}

Planet::~Planet()
{

}

void Planet::SetRadius(float radius)
{
	m_Properties.Radius = radius;
	m_Transform->SetScale({ radius, radius, radius});
}

void Planet::UpdateMesh(int resolution)
{
	m_PlanetMesh = Engine::MeshFactory::Icosphere(resolution, 1.0f);
	m_Renderer = Engine::CreateRef<Engine::EntityRenderer>(m_PlanetMesh, "Planet");
}

void Planet::Draw(const glm::mat4& viewProjection)
{
	m_Renderer->Begin();
	m_Renderer->GetShader()->Bind();
	m_MaterialPropertiesUBO->SetData(&m_MaterialProperties, sizeof(PlanetMaterialProperties));
	m_Renderer->GetShader()->UploadUniformMat4("u_MVP", viewProjection * m_Transform->Transform());
	m_Renderer->GetShader()->UploadUniformMat4("u_Transform", m_Transform->Transform());
	m_Renderer->GetShader()->UploadUniformMat4("u_NormalMatrix", glm::transpose(glm::inverse(m_Transform->Transform())));
	m_Renderer->GetShader()->UploadUniformFloat3("u_Color", {0.5f, 0.5f, 0.5f});
	m_Renderer->Draw();
	m_Renderer->End();
}
