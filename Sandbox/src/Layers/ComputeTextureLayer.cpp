#include "Layers/ComputeTextureLayer.h"

#include <glad/glad.h>
#include <glm/gtc/matrix_transform.hpp>

ComputeTextureLayer::ComputeTextureLayer()
	:Engine::Layer("Test Layer"), m_Camera(45.0f, 1920.0f / 1080.0f, 0.1f, 1000.0f)
{
}

ComputeTextureLayer::~ComputeTextureLayer()
{

}

void ComputeTextureLayer::OnAttach()
{
	Engine::Texture2DSpecification computeTextureSpecification
	{
		Engine::ImageUtils::WrapMode::ClampToEdge,
		Engine::ImageUtils::WrapMode::ClampToEdge,
		Engine::ImageUtils::FilterMode::Linear,
		Engine::ImageUtils::FilterMode::Linear,
		Engine::ImageUtils::ImageInternalFormat::RGBA32F,
		Engine::ImageUtils::ImageDataLayout::RGBA,
		Engine::ImageUtils::ImageDataType::Float,
		1024, 1024
	};

	m_TestComputeTexture = Engine::CreateRef<Engine::Texture2D>(computeTextureSpecification);
	m_TestComputeTexture->BindToImageSlot(0, 0, Engine::ImageUtils::TextureAccessLevel::WriteOnly, Engine::ImageUtils::TextureShaderDataFormat::RGBA32F);

	m_Sphere = Engine::CreateRef<Engine::Entity>(Engine::PrimitiveType::Sphere, "FlatColor");
	m_Sphere->GetEntityTransform()->SetPosition(glm::vec3(3.5f, 0.0f, 0.0f));
	m_Cube = Engine::CreateRef<Engine::Entity>(Engine::PrimitiveType::Cube, "FlatColor");
	m_Cube->GetEntityTransform()->SetPosition(glm::vec3(-3.0f, 0.0f, 0.0f));
	m_Quad = Engine::CreateRef<Engine::Entity>(Engine::PrimitiveType::Quad, "FlatColor");
	m_Quad->GetEntityTransform()->SetPosition(glm::vec3(0.0f, 0.0f, 0.0f));
	m_Quad->GetEntityTransform()->SetScale(glm::vec3(4.0f, 2.0f, 0.0f));

	glm::vec2 windowSize = glm::vec2(Engine::Application::GetApplication().GetWindow().GetWidth(), Engine::Application::GetApplication().GetWindow().GetHeight());

	m_CurrentLacunarity = m_MinLacunarity;
	m_CurrentPersistence = m_MinPersistence;
	m_CurrentScale = m_MinScale;
}

void ComputeTextureLayer::OnDetach()
{

}

void ComputeTextureLayer::OnUpdate(float deltaTime)
{
	glm::vec2 mousePosition = Engine::Input::GetMousePosition();
	glm::vec2 windowSize = glm::vec2(Engine::Application::GetApplication().GetWindow().GetWidth(), Engine::Application::GetApplication().GetWindow().GetHeight());
	glm::vec2 mouseDirection = glm::vec2((mousePosition.x / windowSize.x) * 2.0 - 1, ((mousePosition.y) / windowSize.y) * 2.0 - 1);

	float xPercent = 1 - abs((mousePosition.x / (float)windowSize.x) * 2.0 - 1);
	float yPercent = 1 - abs((mousePosition.y / (float)windowSize.y) * 2.0 - 1);

	float elapsed = Engine::Time::Elapsed();

	float rotationPercent = sin(elapsed) * 0.5f + 0.5f;

	m_CubeRotation = Engine::Lerp(0, 360, rotationPercent);
	m_Cube->GetEntityTransform()->SetRotation(glm::vec3(m_CubeRotation));
	m_Sphere->GetEntityTransform()->SetRotation(glm::vec3(m_CubeRotation));

	m_NoiseColor.x = sin(elapsed / 2) * 0.5 + 0.5;
	m_NoiseColor.y = cos(elapsed * 2) * 0.5 + 0.5;
	m_NoiseColor.z = sin(elapsed) * 0.5 + 0.5;

	// Zoom
	m_Offset += glm::vec2(mouseDirection.x, -mouseDirection.y) * deltaTime * m_Speed;

	// Change parameters with zoom percent.
	m_CurrentScale += ((xPercent + yPercent) / 2.0f) * deltaTime;
	float scalePercent = Engine::Lerp(0, 1, m_CurrentScale / m_MaxScale);
	m_CurrentPersistence = Engine::Lerp(m_MaxPersistence, m_MinPersistence, scalePercent);
	m_CurrentLacunarity = Engine::Lerp(m_MaxLacunarity, m_MinLacunarity, scalePercent);

	Engine::RenderCommand::Clear(true, true);
	Engine::RenderCommand::ClearColor(m_ClearColor);
	if (m_CameraOrbitModeActive)
	{
		m_CameraYawRotate += deltaTime * 30.0f;
		m_Camera.Orbit({ 0.0f, 0.0f, 5.0f }, glm::vec3(0.0f), { 0.0f, m_CameraYawRotate, 0.0f });
	}
	else
	{
		m_Camera.Update(deltaTime);
	}


	Engine::ShaderLibrary::Get("TestCompute")->Bind();
	Engine::ShaderLibrary::Get("TestCompute")->UploadUniformFloat4("u_Params", glm::vec4(m_CurrentPersistence, m_CurrentLacunarity, 8, m_CurrentScale));
	Engine::ShaderLibrary::Get("TestCompute")->UploadUniformFloat2("u_Offset", m_Offset);
	Engine::ShaderLibrary::Get("TestCompute")->UploadUniformFloat4("u_NoiseColor", m_NoiseColor);
	Engine::ShaderLibrary::Get("TestCompute")->UploadUniformFloat("u_Time", elapsed);
	Engine::ShaderLibrary::Get("TestCompute")->DispatchCompute(m_TestComputeTexture->GetWidth(), m_TestComputeTexture->GetHeight(), 1);

	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

	m_TestComputeTexture->BindToSamplerSlot(0);
	m_Sphere->GetEntityRenderer()->GetShader()->Bind();
	m_Sphere->GetEntityRenderer()->GetShader()->UploadUniformFloat3("u_Color", glm::vec3(1.0f));
	m_Sphere->DrawEntity(m_Camera.GetViewProjection());
	m_Cube->GetEntityRenderer()->GetShader()->Bind();
	m_Cube->GetEntityRenderer()->GetShader()->UploadUniformFloat3("u_Color", glm::vec3(1.0f));
	m_Cube->DrawEntity(m_Camera.GetViewProjection());
	m_Quad->GetEntityRenderer()->GetShader()->Bind();
	m_Quad->GetEntityRenderer()->GetShader()->UploadUniformFloat3("u_Color", glm::vec3(1.0f));
	m_Quad->DrawEntity(m_Camera.GetViewProjection());
}

void ComputeTextureLayer::OnEvent(Engine::Event& event)
{
	Engine::EventDispatcher dispatcher(event);

	dispatcher.Dispatch<Engine::KeyPressedEvent>(BIND_FN(ComputeTextureLayer::OnKeyPressed));

	m_Camera.OnEvent(event);
}

bool ComputeTextureLayer::OnKeyPressed(Engine::KeyPressedEvent& event)
{
	if (event.GetKeyCode() == Engine::Key::Space)
	{
		m_CameraOrbitModeActive = !m_CameraOrbitModeActive;
		m_Camera.ToggleIsLocked(m_CameraOrbitModeActive);
	}

	return true;
}
