#include "StarMapLayer.h"

StarMapLayer::StarMapLayer()
{

}

StarMapLayer::~StarMapLayer()
{

}

void StarMapLayer::OnAttach()
{
	Engine::Application::GetApplication().GetWindow().ToggleMaximize(true);
	Engine::ShaderLibrary::Load("assets/shaders/Testing/NoTransform.shader");
	m_WhiteTexture = Engine::Texture2D::CreateWhiteTexture();
	m_Plane = Engine::CreateRef<Engine::SimpleEntity>(Engine::PrimitiveType::Plane, "FlatColor");
	m_Plane->GetEntityTransform()->SetScale({ 10.0, 1.0, 10.0 });
	m_Plane->GetEntityTransform()->SetPosition({ 0.0, -2.0, 0.0});

	m_Sphere = Engine::CreateRef<Engine::SimpleEntity>(Engine::PrimitiveType::Sphere, "NoTransform");

	std::vector<Engine::Vertex> vertices;
	vertices.resize(1000);

}

void StarMapLayer::OnDetach()
{

}

void StarMapLayer::OnUpdate(float deltaTime)
{
	m_Camera.Update(deltaTime);

	Engine::RenderCommand::ClearColor({ 0.1f, 0.1f, 0.1f, 0.1f });
	Engine::RenderCommand::Clear(true, true);

	m_Plane->GetEntityRenderer()->GetShader()->Bind();
	m_Plane->GetEntityRenderer()->GetShader()->UploadUniformFloat3("u_Color", { 1.0f, 1.0f, 1.0f });
	m_Plane->DrawEntity(m_Camera.GetViewProjection());

	m_Sphere->GetEntityRenderer()->GetShader()->Bind();
	m_Sphere->GetEntityRenderer()->GetShader()->UploadUniformFloat3("u_Color", { 1.0f, 1.0f, 1.0f });
	m_Sphere->DrawEntityPoints(m_Camera.GetViewProjection());
}

void StarMapLayer::OnEvent(Engine::Event& event)
{
	m_Camera.OnEvent(event);
}

void StarMapLayer::OnImGuiRender()
{

}

bool StarMapLayer::OnKeyPressed(Engine::KeyPressedEvent& keyPressedEvent)
{
	return false;
}
