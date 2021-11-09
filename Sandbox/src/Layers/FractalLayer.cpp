#include "Layers/FractalLayer.h"
#include <iostream>

FractalLayer::FractalLayer()
	:m_Camera(45.0f, 1920.0f / 1080.0f, 0.1f, 1000.0f)
{

}

FractalLayer::~FractalLayer()
{

}

void FractalLayer::OnAttach()
{
	Engine::ShaderLibrary::Load("assets/shaders/TestPhong.shader");

	m_Entity = Engine::CreateRef<Engine::SimpleEntity>(Engine::PrimitiveType::FullScreenQuad, "Fractal");
	//m_Entity = Engine::CreateRef<Engine::SimpleEntity>(Engine::PrimitiveType::Sphere, "TestPhong");
}

void FractalLayer::OnDetach()
{

}

void FractalLayer::TestPhong()
{
	m_Entity->GetEntityRenderer()->GetShader()->Bind();
	m_Entity->GetEntityRenderer()->GetShader()->UploadUniformFloat3("u_LightPosition", { 5.0f, 5.0f, 0.0f });
	m_Entity->GetEntityRenderer()->GetShader()->UploadUniformFloat3("u_SpecularColor", { 0, 0, 1 });
	m_Entity->GetEntityRenderer()->GetShader()->UploadUniformFloat3("u_Color", { 0, 1, .2 });

	m_Entity->GetEntityRenderer()->GetShader()->UploadUniformFloat("u_AmbientStrength", 0.3f);
	m_Entity->GetEntityRenderer()->GetShader()->UploadUniformFloat("u_DiffuseStrength", 0.5f);
	m_Entity->GetEntityRenderer()->GetShader()->UploadUniformFloat("u_SpecularStrength", 1.0f);
	m_Entity->GetEntityRenderer()->GetShader()->UploadUniformFloat("u_Shininess", 32.0f);
	glm::mat4 modelView = m_Camera.GetView() * m_Entity->GetEntityTransform()->Transform();
	m_Entity->GetEntityRenderer()->GetShader()->UploadUniformMat4("u_ModelViewMatrix", modelView);
	glm::mat4 normalMatrix = glm::transpose(glm::inverse(modelView));
	m_Entity->GetEntityRenderer()->GetShader()->UploadUniformMat4("u_NormalMatrix", normalMatrix);

	m_Entity->DrawEntity(m_Camera.GetViewProjection());
}

void FractalLayer::OnUpdate(float deltaTime)
{
	m_Camera.Update(deltaTime);

	Engine::RenderCommand::ClearColor(m_ClearColor);
	Engine::RenderCommand::Clear(true, true);

	glm::vec3 camPosition = m_Camera.GetPosition();
	m_Entity->GetEntityRenderer()->GetShader()->Bind();
	m_Entity->GetEntityRenderer()->GetShader()->UploadUniformFloat("u_Elapsed", Engine::Time::Elapsed());
	m_Entity->GetEntityRenderer()->GetShader()->UploadUniformFloat("u_DeltaTime", Engine::Time::DeltaTime());
	m_Entity->GetEntityRenderer()->GetShader()->UploadUniformFloat3("u_CameraPosition", {0.0f, 5.0f, -1.5f});
	m_Entity->GetEntityRenderer()->GetShader()->UploadUniformFloat3("u_LightPosition", m_LightPosition);
	glm::vec2 windowSize = glm::vec2(Engine::Application::GetApplication().GetWindow().GetWidth(), Engine::Application::GetApplication().GetWindow().GetHeight());
	m_Entity->GetEntityRenderer()->GetShader()->UploadUniformFloat2("u_ScreenResolution", windowSize);
	glm::vec2 mousePosition = Engine::Input::GetMousePosition();
	m_Entity->GetEntityRenderer()->GetShader()->UploadUniformFloat2("u_MousePosition", mousePosition);
	
	m_Entity->DrawEntity(m_Camera.GetViewProjection());
}

void FractalLayer::OnImGuiRender()
{

}

void FractalLayer::OnEvent(Engine::Event& event)
{
	m_Camera.OnEvent(event);

	Engine::EventDispatcher dispatcher(event);

	dispatcher.Dispatch<Engine::KeyPressedEvent>(BIND_FN(FractalLayer::OnKeyPressed));
}

bool FractalLayer::OnKeyPressed(Engine::KeyPressedEvent& keyPressedEvent)
{
	return false;
}
