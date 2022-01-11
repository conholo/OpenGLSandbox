#include "Layers/410/SkyVolumeGeneratorLayer.h"

#include <imgui/imgui.h>

SkyVolumeGeneratorLayer::SkyVolumeGeneratorLayer()
{

}

SkyVolumeGeneratorLayer::~SkyVolumeGeneratorLayer()
{

}

void SkyVolumeGeneratorLayer::OnAttach()
{
	Engine::Application::GetApplication().GetWindow().ToggleMaximize(true);
	m_WhiteTexture = Engine::Texture2D::CreateWhiteTexture();

	Engine::ShaderLibrary::Load("assets/410 shaders/SkyVolume.shader");

	Engine::TextureSpecification skyBoxSpec =
	{
		Engine::ImageUtils::Usage::Storage,
		Engine::ImageUtils::WrapMode::ClampToEdge,
		Engine::ImageUtils::FilterMode::Linear,
		Engine::ImageUtils::ImageInternalFormat::RGBA32F,
		Engine::ImageUtils::ImageDataLayout::RGBA,
		Engine::ImageUtils::ImageDataType::Float,
		1024, 1024
	};

	m_TextureCube = Engine::CreateRef<Engine::TextureCube>(skyBoxSpec, nullptr);
	m_TextureCube->BindToImageSlot(0, 0, Engine::ImageUtils::TextureAccessLevel::WriteOnly, Engine::ImageUtils::TextureShaderDataFormat::RGBA32F);
	m_CubeMap = Engine::CreateRef<Engine::CubeMap>(m_TextureCube, Engine::ShaderLibrary::Get("Skybox"));
}

void SkyVolumeGeneratorLayer::OnDetach()
{

}

void SkyVolumeGeneratorLayer::OnUpdate(float deltaTime)
{
	m_Camera.Update(deltaTime);

	Engine::RenderCommand::ClearColor({0.0f, 0.0f, 0.0f, 0.0f});
	Engine::RenderCommand::Clear(true, true);

	Engine::ShaderLibrary::Get("SkyVolume")->Bind();
	Engine::ShaderLibrary::Get("SkyVolume")->UploadUniformFloat3("u_TAI", m_TAI);
	Engine::ShaderLibrary::Get("SkyVolume")->DispatchCompute(m_TextureCube->GetWidth() / 32, m_TextureCube->GetHeight() / 32, 6);
	Engine::ShaderLibrary::Get("SkyVolume")->EnableShaderImageAccessBarrierBit();

	m_TextureCube->BindToSamplerSlot(0);
	Engine::ShaderLibrary::Get("Skybox")->Bind();
	Engine::ShaderLibrary::Get("Skybox")->UploadUniformInt("u_Texture", 0);
	Engine::ShaderLibrary::Get("Skybox")->UploadUniformFloat("u_TextureLOD", 0);
	Engine::ShaderLibrary::Get("Skybox")->UploadUniformFloat("u_Intensity", 1.0f);
	glm::mat4 inverseViewProjection = glm::mat4(glm::inverse(m_Camera.GetViewProjection()));
	m_CubeMap->Submit(inverseViewProjection);
}

void SkyVolumeGeneratorLayer::OnEvent(Engine::Event& event)
{
	m_Camera.OnEvent(event);
}

void SkyVolumeGeneratorLayer::OnImGuiRender()
{
	ImGui::Begin("Sky Properties");
	ImGui::DragFloat("Inclination", &m_TAI.z, 0.1, -3.1415f, 3.1415f);
	ImGui::DragFloat("Turbidity", &m_TAI.x, 0.1, 1.0f, 100.0f);
	ImGui::End();
}
