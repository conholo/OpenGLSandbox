#include "CurlDemoLayer.h"

#include <imgui.h>

CurlDemoLayer::CurlDemoLayer()
{

}

CurlDemoLayer::~CurlDemoLayer()
{

}

void CurlDemoLayer::OnAttach()
{
	Engine::Application::GetApplication().GetWindow().ToggleMaximize(true);
	Engine::ShaderLibrary::Load("assets/Clouds/Curl.glsl");

	Engine::Texture2DSpecification curlSpec =
	{
		Engine::ImageUtils::WrapMode::Repeat,
		Engine::ImageUtils::WrapMode::Repeat,
		Engine::ImageUtils::FilterMode::Linear,
		Engine::ImageUtils::FilterMode::Linear,
		Engine::ImageUtils::ImageInternalFormat::RGBA32F,
		Engine::ImageUtils::ImageDataLayout::RGBA,
		Engine::ImageUtils::ImageDataType::Float,
		1024, 1024
	};

	m_CurlTexture = Engine::CreateRef<Engine::Texture2D>(curlSpec);
	m_CurlTexture->BindToImageSlot(0, 0, Engine::ImageUtils::TextureAccessLevel::WriteOnly, Engine::ImageUtils::TextureShaderDataFormat::RGBA32F);

	m_Quad = Engine::CreateRef<Engine::SimpleEntity>(Engine::PrimitiveType::Quad, "FlatColor");

	m_Quad->GetEntityTransform()->SetPosition({ 0.0f, 0.0f, 0.0f });
	m_Quad->GetEntityTransform()->SetScale({ 1.5f, 1.5f, 1.0f });

	m_Camera.SetOrthographic();
}

void CurlDemoLayer::OnDetach()
{

}

void CurlDemoLayer::OnUpdate(float deltaTime)
{
	m_Camera.Update(deltaTime);

	uint32_t threadGroups = glm::ceil(m_CurlTexture->GetWidth() / 8.0f);

	Engine::ShaderLibrary::Get("Curl")->Bind();
	Engine::ShaderLibrary::Get("Curl")->UploadUniformFloat("u_ElapsedTime", Engine::Time::Elapsed());
	Engine::ShaderLibrary::Get("Curl")->UploadUniformFloat("u_Strength", m_Strength);
	Engine::ShaderLibrary::Get("Curl")->UploadUniformFloat("u_Persistence", m_Persistence);
	Engine::ShaderLibrary::Get("Curl")->UploadUniformFloat("u_Tiling", m_Tiling);
	Engine::ShaderLibrary::Get("Curl")->UploadUniformFloat("u_TilingSpeed", m_TilingSpeed);
	Engine::ShaderLibrary::Get("Curl")->UploadUniformFloat2("u_TilingOffset", m_TilingOffset);
	Engine::ShaderLibrary::Get("Curl")->UploadUniformFloat2("u_Weights", m_Weights);
	Engine::ShaderLibrary::Get("Curl")->DispatchCompute(threadGroups, threadGroups, 1);
	Engine::ShaderLibrary::Get("Curl")->EnableShaderImageAccessBarrierBit();

	Engine::RenderCommand::ClearColor({0.0f, 0.0f, 0.0f, 0.0f});
	Engine::RenderCommand::Clear(true, true);

	m_CurlTexture->BindToSamplerSlot(0);
	m_Quad->GetEntityRenderer()->GetShader()->Bind();
	m_Quad->GetEntityRenderer()->GetShader()->UploadUniformFloat3("u_Color", {1.0f, 1.0f, 1.0f});
	m_Quad->GetEntityRenderer()->GetShader()->UploadUniformInt("u_Texture", 0);
	m_Quad->DrawEntity(m_Camera.GetViewProjection());
}

void CurlDemoLayer::OnEvent(Engine::Event& e)
{
	m_Camera.OnEvent(e);
}

void CurlDemoLayer::OnImGuiRender()
{
	ImGui::Begin("Curl Settings");
	ImGui::DragFloat("Strength", &m_Strength, 0.001);
	ImGui::DragFloat("Persistence", &m_Persistence, 0.001);
	ImGui::DragFloat("Tiling", &m_Tiling, 0.1);
	ImGui::DragFloat2("Tiling Offset", &m_TilingOffset.x);
	ImGui::DragFloat2("Weights", &m_Weights.x, 0.001f);
	ImGui::End();
}
