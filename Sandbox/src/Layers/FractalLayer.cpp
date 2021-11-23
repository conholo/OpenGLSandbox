#include "Layers/FractalLayer.h"
#include <iostream>
#include <glm/glm.hpp>

FractalLayer::FractalLayer()
	:m_Camera(45.0f, 1920.0f / 1080.0f, 0.1f, 1000.0f)
{

}

FractalLayer::~FractalLayer()
{

}

void FractalLayer::OnAttach()
{
	m_Entity = Engine::CreateRef<Engine::SimpleEntity>(Engine::PrimitiveType::FullScreenQuad, "Fractal");
}

void FractalLayer::OnDetach()
{

}

void FractalLayer::OnUpdate(float deltaTime)
{
	m_Camera.Update(deltaTime);

	Engine::RenderCommand::ClearColor(m_ClearColor);
	Engine::RenderCommand::Clear(true, true);
	
	if(m_MengerCounter > m_MengerMaxScale)
		m_Up = false;
	if (m_MengerCounter < 1.0f)
		m_Up = true;

	m_MengerCounter = m_Up ? m_MengerCounter + deltaTime * 0.1f : m_MengerCounter - deltaTime * 0.1f;
	m_MengerCounter = Engine::Clamp(m_MengerCounter, 1.0f, m_MengerMaxScale);

	glm::vec3 camPosition = m_Camera.GetPosition();
	m_Entity->GetEntityRenderer()->GetShader()->Bind();
	m_Entity->GetEntityRenderer()->GetShader()->UploadUniformFloat("u_MengerScale", m_MengerCounter);
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
