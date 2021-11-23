#include "TestBezierSurfaceLayer.h"
#include <imgui/imgui.h>

TestBezierSurfaceLayer::TestBezierSurfaceLayer()
{

}

TestBezierSurfaceLayer::~TestBezierSurfaceLayer()
{

}

void TestBezierSurfaceLayer::OnAttach()
{
	Engine::RenderCommand::SetPointSize(10.0f);
	m_Surface = Engine::CreateRef<Engine::BezierSurface>();
	m_Cube = Engine::CreateRef<Engine::SimpleEntity>(Engine::PrimitiveType::Cube, "FlatColor");
	m_Texture = Engine::Texture2D::CreateWhiteTexture();

	Engine::LightSpecification spec =
	{
		Engine::LightType::Directional,
		{1.0f, 1.0f, 1.0f},
		1.0f
	};

	m_Light = Engine::CreateRef<Engine::Light>(spec);
	m_Light->GetLightTransform()->SetPosition({ 2.0f, 30.0f, 10.0f });

	m_Surface->SetScale({ 1.5f, 1.0f, 1.0f });
}

void TestBezierSurfaceLayer::OnDetach()
{

}

void TestBezierSurfaceLayer::OnUpdate(float deltaTime)
{
	m_Camera.Update(deltaTime);
	m_Surface->AnimateControls();
	m_Surface->UpdateDragCurve(m_Camera);
	Engine::RenderCommand::ClearColor(m_ClearColor);
	Engine::RenderCommand::Clear(true, true);
	m_Surface->GetShader()->Bind();
	m_Surface->GetShader()->UploadUniformFloat3("u_CameraPosition", m_Camera.GetPosition());
	m_Surface->GetShader()->UploadUniformFloat3("u_LightPosition", m_Light->GetLightTransform()->GetPosition());
	m_Surface->GetShader()->UploadUniformFloat3("u_LightColor", m_Light->GetLightColor());
	m_Surface->GetShader()->UploadUniformFloat("u_LightIntensity", m_Light->GetLightIntensity());

	m_Surface->GetShader()->UploadUniformFloat("u_AmbientStrength", m_Properties.AmbientStrength);
	m_Surface->GetShader()->UploadUniformFloat("u_DiffuseStrength", m_Properties.DiffuseStrength);
	m_Surface->GetShader()->UploadUniformFloat("u_SpecularStrength", m_Properties.SpecularStrength);
	m_Surface->GetShader()->UploadUniformFloat("u_Shininess", m_Properties.Shininess);
	m_Surface->GetShader()->UploadUniformFloat3("u_AmbientColor", m_Properties.AmbientColor);
	m_Surface->GetShader()->UploadUniformFloat3("u_DiffuseColor", m_Properties.DiffuseColor);
	m_Surface->Draw(m_Camera.GetViewProjection());
}

void TestBezierSurfaceLayer::OnEvent(Engine::Event& event)
{
	m_Camera.OnEvent(event);

	Engine::EventDispatcher dispatcher(event);

	dispatcher.Dispatch<Engine::MouseButtonReleasedEvent>(BIND_FN(TestBezierSurfaceLayer::OnMouseButtonReleased));
	dispatcher.Dispatch<Engine::MouseButtonPressedEvent>(BIND_FN(TestBezierSurfaceLayer::OnMouseButtonPressed));
	dispatcher.Dispatch<Engine::KeyPressedEvent>(BIND_FN(TestBezierSurfaceLayer::OnKeyPressed));
}

void TestBezierSurfaceLayer::OnImGuiRender()
{
	ImGui::Begin("Stats");
	ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
	ImGui::End();

}

bool TestBezierSurfaceLayer::OnKeyPressed(Engine::KeyPressedEvent& pressedEvent)
{
	if (pressedEvent.GetKeyCode() == Engine::Key::O)
		m_Camera.SetOrthographic();
	if (pressedEvent.GetKeyCode() == Engine::Key::P)
		m_Camera.SetPerspective();

	if (pressedEvent.GetKeyCode() == Engine::Key::Space)
		m_Surface->ToggleDrawCurves();

	return false;
}

bool TestBezierSurfaceLayer::OnMouseButtonPressed(Engine::MouseButtonPressedEvent& pressedEvent)
{
	m_Surface->StartPicking(pressedEvent.GetButton(), m_Camera);
	return false;
}

bool TestBezierSurfaceLayer::OnMouseButtonReleased(Engine::MouseButtonReleasedEvent& releasedEvent)
{
	m_Surface->StopPicking(releasedEvent.GetButton());
	return false;
}

