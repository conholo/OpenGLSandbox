#include "Layers/LineLayer.h"
#include <vector>
#include <iostream>
#define PI 3.14159265358979


LineLayer::LineLayer()
	:m_Camera(45.0f, 1920.0f / 1080.0f, 0.1f, 1000.0f)
{

}

LineLayer::~LineLayer()
{

}

void LineLayer::OnAttach()
{
	std::vector<Engine::LineVertex> vertices;

	m_Line = Engine::CreateRef<Engine::Line>(vertices, "LineShader");
	m_Cube = Engine::CreateRef<Engine::SimpleEntity>(Engine::PrimitiveType::Cube, "FlatColor");
	m_WhiteTexture = Engine::Texture2D::CreateWhiteTexture();
}

void LineLayer::OnDetach()
{

}

void LineLayer::OnUpdate(float deltaTime)
{
	m_Camera.Update(deltaTime);

	Engine::RenderCommand::ClearColor(m_ClearColor);
	Engine::RenderCommand::Clear(true, true);

	m_Cube->GetEntityRenderer()->GetShader()->Bind();
	m_Cube->GetEntityRenderer()->GetShader()->UploadUniformFloat3("u_Color", { 1.0f, 1.0f, 1.0f });
	m_Cube->DrawEntity(m_Camera.GetViewProjection());

	m_Line->Draw(m_Camera.GetViewProjection());
}

void LineLayer::OnEvent(Engine::Event& event)
{
	m_Camera.OnEvent(event);

	Engine::EventDispatcher dispatcher(event);

	dispatcher.Dispatch<Engine::KeyPressedEvent>(BIND_FN(LineLayer::OnKeyPressed));
	dispatcher.Dispatch<Engine::MouseButtonReleasedEvent>(BIND_FN(LineLayer::OnMouseButtonReleased));
}

void LineLayer::OnImGuiRender()
{

}

bool LineLayer::OnKeyPressed(Engine::KeyPressedEvent& keyPressedEvent)
{
	if (keyPressedEvent.GetKeyCode() == Engine::Key::Space)
		m_Line->Clear();
	if (keyPressedEvent.GetKeyCode() == Engine::Key::O)
		m_Camera.SetOrthographic();
	if (keyPressedEvent.GetKeyCode() == Engine::Key::P)
		m_Camera.SetPerspective();


	return false;
}

bool LineLayer::OnMouseButtonReleased(Engine::MouseButtonReleasedEvent& releasedEvent)
{
	if (releasedEvent.GetButton() == Engine::Mouse::ButtonLeft)
	{
		glm::vec2 mousePosition = Engine::Input::GetMousePosition();
		glm::vec3 worldCoordinate = m_Camera.ScreenToWorldPoint({ mousePosition.x, mousePosition.y, 0.0f });
		m_Line->AddPoint({ {worldCoordinate} });
	}

	return false;
}
