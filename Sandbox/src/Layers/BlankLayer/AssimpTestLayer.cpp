#include "Layers/Testing/AssimpTestLayer.h"

AssimpTestLayer::AssimpTestLayer()
{

}

AssimpTestLayer::~AssimpTestLayer()
{

}

void AssimpTestLayer::OnAttach()
{
	Engine::Application::GetApplication().GetWindow().ToggleMaximize(true);
	m_Sphere = Engine::CreateRef<Engine::SimpleEntity>(Engine::PrimitiveType::Sphere, "FlatColor");
	m_WhiteTexture = Engine::Texture2D::CreateWhiteTexture();
}

void AssimpTestLayer::OnDetach()
{

}

void AssimpTestLayer::OnUpdate(float deltaTime)
{
	m_Camera.Update(deltaTime);

	Engine::RenderCommand::ClearColor({ 0.0f, 0.0f, 0.0f, 0.0f });
	Engine::RenderCommand::Clear(true, true);

	m_Sphere->GetEntityRenderer()->GetShader()->Bind();
	m_WhiteTexture->BindToSamplerSlot(0);
	m_Sphere->GetEntityRenderer()->GetShader()->UploadUniformFloat3("u_Color", { 1.0f, 0.0f, 0.0f });
	m_Sphere->DrawEntity(m_Camera.GetViewProjection());
}

void AssimpTestLayer::OnEvent(Engine::Event& e)
{
	m_Camera.OnEvent(e);
}

void AssimpTestLayer::OnImGuiRender()
{

}
