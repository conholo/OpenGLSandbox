#include "ComputeTest.h"
#include <imgui/imgui.h>
#include <iostream>

#include <glad/glad.h>

ComputeTestLayer::ComputeTestLayer()
{

}

ComputeTestLayer::~ComputeTestLayer()
{

}

void ComputeTestLayer::OnAttach()
{
	Engine::Application::GetApplication().GetWindow().ToggleMaximize(true);
	uint32_t width = Engine::Application::GetApplication().GetWindow().GetWidth();
	uint32_t height = Engine::Application::GetApplication().GetWindow().GetHeight();

	Engine::ShaderLibrary::Load("assets/shaders/Testing/ComputeTesting.shader");

	Engine::Texture2DSpecification computeSpec =
	{
		Engine::ImageUtils::WrapMode::ClampToEdge,
		Engine::ImageUtils::WrapMode::ClampToEdge,
		Engine::ImageUtils::FilterMode::Nearest,
		Engine::ImageUtils::FilterMode::Nearest,
		Engine::ImageUtils::ImageInternalFormat::RGBA32F,
		Engine::ImageUtils::ImageDataLayout::RGBA,
		Engine::ImageUtils::ImageDataType::Float,
		16, 16
	};

	Engine::Texture2DSpecification fromFileSpec =
	{
		Engine::ImageUtils::WrapMode::Repeat,
		Engine::ImageUtils::WrapMode::Repeat,
		Engine::ImageUtils::FilterMode::LinearMipLinear,
		Engine::ImageUtils::FilterMode::Linear,
		Engine::ImageUtils::ImageInternalFormat::FromImage,
		Engine::ImageUtils::ImageDataLayout::FromImage,
		Engine::ImageUtils::ImageDataType::UByte,
	};

	m_ComputeTexture = Engine::CreateRef<Engine::Texture2D>(computeSpec);
	m_FromImageTexture = Engine::CreateRef<Engine::Texture2D>("assets/textures/grass.jpg", fromFileSpec);

	m_WhiteTexture = Engine::Texture2D::CreateWhiteTexture();
	m_Plane = Engine::CreateRef<Engine::SimpleEntity>(Engine::PrimitiveType::Plane, "FlatColor");
	m_Plane->GetEntityTransform()->SetPosition({ 0.0f, -2.0f, 0.0f });
	m_Plane->GetEntityTransform()->SetScale({ 10.0f, 1.0f, 10.0f });

	m_DisplayQuad = Engine::CreateRef<Engine::SimpleEntity>(Engine::PrimitiveType::Quad, "FlatColor");
	m_DisplayQuad->GetEntityTransform()->SetPosition({ 0.0f, 2.0f, 0.0f });
	m_DisplayQuad->GetEntityTransform()->SetScale({ 2.0f, 2.0f, 0.0f });

	m_Values.resize(16);

	for (int i = 0; i < 16; i++)
		m_Values[i] = i;

	Resize(width, height);
}

void ComputeTestLayer::OnDetach()
{

}

void ComputeTestLayer::OnUpdate(float deltaTime)
{
	uint32_t width = Engine::Application::GetApplication().GetWindow().GetWidth();
	uint32_t height = Engine::Application::GetApplication().GetWindow().GetHeight();

	if (width != m_ViewportWidth || height != m_ViewportHeight)
		Resize(width, height);

	m_Camera.Update(deltaTime);
	Simple2DTextureTest();
}

void ComputeTestLayer::OnEvent(Engine::Event& event)
{
	m_Camera.OnEvent(event);

	Engine::EventDispatcher dispatcher(event);

	dispatcher.Dispatch<Engine::KeyPressedEvent>(BIND_FN(ComputeTestLayer::OnKeyPressed));
}

void ComputeTestLayer::OnImGuiRender()
{
	ImGui::Begin("Compute Test");
	ImGui::End();
}

void ComputeTestLayer::Simple2DTextureTest()
{
	if (Engine::Time::Elapsed() - m_Counter > m_SwitchTime)
	{
		m_Counter = Engine::Time::Elapsed();
		m_ValueX = glm::floor(Engine::Random::RandomRange(0.0f, 16.0f));
		m_ValueY = glm::floor(Engine::Random::RandomRange(0.0f, 16.0f));
	}

	Engine::RenderCommand::ClearColor({ 0.0f, 0.0f, 0.0f, 0.0f });
	Engine::RenderCommand::Clear(true, true);
	m_Plane->GetEntityRenderer()->GetShader()->Bind();
	m_FromImageTexture->BindToSamplerSlot(0);
	m_Plane->GetEntityRenderer()->GetShader()->UploadUniformInt("u_Texture", 0);
	m_Plane->GetEntityRenderer()->GetShader()->UploadUniformFloat3("u_Color", { 1.0f, 1.0f, 1.0f });
	m_Plane->DrawEntity(m_Camera.GetViewProjection());


	Engine::ShaderLibrary::Get("ComputeTesting")->Bind();
	Engine::ShaderLibrary::Get("ComputeTesting")->UploadUniformIntArray("u_Values", 16, m_Values.data());
	Engine::ShaderLibrary::Get("ComputeTesting")->UploadUniformInt("u_ValueX", m_ValueX);
	Engine::ShaderLibrary::Get("ComputeTesting")->UploadUniformInt("u_ValueY", m_ValueY);
	m_ComputeTexture->BindToImageSlot(0, 0, Engine::ImageUtils::TextureAccessLevel::WriteOnly, Engine::ImageUtils::TextureShaderDataFormat::RGBA32F);
	Engine::ShaderLibrary::Get("ComputeTesting")->DispatchCompute(4, 4, 1);

	m_DisplayQuad->GetEntityRenderer()->GetShader()->Bind();
	m_ComputeTexture->BindToSamplerSlot(0);
	m_DisplayQuad->GetEntityRenderer()->GetShader()->UploadUniformInt("u_Texture", 0);
	m_DisplayQuad->GetEntityRenderer()->GetShader()->UploadUniformFloat3("u_Color", { 1.0f, 1.0f, 1.0f });
	m_DisplayQuad->DrawEntity(m_Camera.GetViewProjection());
}

void ComputeTestLayer::Resize(uint32_t width, uint32_t height)
{
	m_ViewportWidth = width;
	m_ViewportHeight = height;
}

bool ComputeTestLayer::OnKeyPressed(Engine::KeyPressedEvent& keyPressedEvent)
{
	return false;
}
