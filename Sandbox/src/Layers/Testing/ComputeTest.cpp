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

	Engine::FramebufferSpecification fboSpec =
	{
		width, height,
		{Engine::FramebufferTextureFormat::RGBA32F, Engine::FramebufferTextureFormat::Depth}
	};

	m_FBO = Engine::CreateRef<Engine::Framebuffer>(fboSpec);

	Engine::ShaderLibrary::Load("assets/shaders/Testing/TestPostProcessing.shader");
	Engine::ShaderLibrary::Load("assets/shaders/Testing/TextureCopy.shader");

	Engine::Texture2DSpecification specA =
	{
		Engine::ImageUtils::WrapMode::ClampToEdge,
		Engine::ImageUtils::WrapMode::ClampToEdge,
		Engine::ImageUtils::FilterMode::Linear,
		Engine::ImageUtils::FilterMode::Linear,
		Engine::ImageUtils::ImageInternalFormat::RGBA32F,
		Engine::ImageUtils::ImageDataLayout::RGBA,
		Engine::ImageUtils::ImageDataType::Float, 
		2, 2
	};

	Engine::Texture2DSpecification specB =
	{
		Engine::ImageUtils::WrapMode::Repeat,
		Engine::ImageUtils::WrapMode::Repeat,
		Engine::ImageUtils::FilterMode::LinearMipLinear,
		Engine::ImageUtils::FilterMode::Linear,
		Engine::ImageUtils::ImageInternalFormat::FromImage,
		Engine::ImageUtils::ImageDataLayout::FromImage,
		Engine::ImageUtils::ImageDataType::UByte,
	};

	m_TextureA = Engine::CreateRef<Engine::Texture2D>(specA);
	m_FromImageTexture = Engine::CreateRef<Engine::Texture2D>("assets/textures/grass.jpg", specB);
	m_TextureA->Resize(m_FromImageTexture->GetWidth(), m_FromImageTexture->GetHeight());


	m_WhiteTexture = Engine::Texture2D::CreateWhiteTexture();
	m_Plane = Engine::CreateRef<Engine::SimpleEntity>(Engine::PrimitiveType::Plane, "FlatColor");
	m_Plane->GetEntityTransform()->SetPosition({0.0f, -2.0f, 0.0f});
	m_Plane->GetEntityTransform()->SetScale({ 10.0f, 1.0f, 10.0f });

	m_FullScreenQuad = Engine::CreateRef<Engine::SimpleEntity>(Engine::PrimitiveType::FullScreenQuad, "TestPostProcessing");


	glGenTextures(1, &m_ImageViewID);
	glTextureView(m_ImageViewID, GL_TEXTURE_2D, m_TextureA->GetID(), GL_RGBA32F, 0, 1, 0, 1);

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
	m_FBO->Bind();
	Engine::RenderCommand::ClearColor({ 0.0f, 0.0f, 0.0f, 0.0f });
	Engine::RenderCommand::Clear(true, true);
	m_Plane->GetEntityRenderer()->GetShader()->Bind();
	m_FromImageTexture->BindToSamplerSlot(0);
	m_Plane->GetEntityRenderer()->GetShader()->UploadUniformInt("u_Texture", 0);
	m_Plane->GetEntityRenderer()->GetShader()->UploadUniformFloat3("u_Color", {1.0f, 1.0f, 1.0f});
	m_Plane->DrawEntity(m_Camera.GetViewProjection());
	m_FBO->Unbind();

	Engine::ShaderLibrary::Get("TextureCopy")->Bind();
	m_FromImageTexture->BindToSamplerSlot(0);
	Engine::ShaderLibrary::Get("TextureCopy")->UploadUniformInt("u_ReadTexture", 0);
	Engine::ShaderLibrary::Get("TextureCopy")->UploadUniformFloat("u_LOD", m_CurrentMip);
	m_TextureA->BindToImageSlot(0, 0, Engine::ImageUtils::TextureAccessLevel::WriteOnly, Engine::ImageUtils::TextureShaderDataFormat::RGBA32F);
	auto [mipWidth, mipHeight] = m_TextureA->GetMipSize(0);
	uint32_t workGroupsX = (uint32_t)glm::ceil((float)mipWidth / (float)m_WorkGroupSize);
	uint32_t workGroupsY = (uint32_t)glm::ceil((float)mipHeight / (float)m_WorkGroupSize);
	Engine::ShaderLibrary::Get("TextureCopy")->DispatchCompute(workGroupsX, workGroupsY, 1);

	Engine::RenderCommand::ClearColor({ 0.0f, 0.0f, 0.0f, 0.0f });
	Engine::RenderCommand::Clear(true, true);
	m_FullScreenQuad->GetEntityRenderer()->GetShader()->Bind();
	m_FBO->BindColorAttachment(0);
	m_FullScreenQuad->GetEntityRenderer()->GetShader()->UploadUniformInt("u_SceneTexture", 0);
	m_FullScreenQuad->DrawEntity(m_Camera.GetViewProjection());
}

void ComputeTestLayer::OnEvent(Engine::Event& event)
{
	m_Camera.OnEvent(event);

	Engine::EventDispatcher dispatcher(event);

	dispatcher.Dispatch<Engine::KeyPressedEvent>(BIND_FN(ComputeTestLayer::OnKeyPressed));
}

void ComputeTestLayer::OnImGuiRender()
{
	ImGui::Begin("Texture Data");
	ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
	
	if (ImGui::TreeNodeEx("Textures"))
	{
		float aspect = Engine::Application::GetApplication().GetWindow().GetAspectRatio();

		ImVec2 size = { 200 * aspect, 200 };
		ImGui::Image((void*)m_FBO->GetColorAttachmentID(0), size, { 0, 1 }, { 1, 0 });
		ImGui::Image((void*)m_FromImageTexture->GetID(), size, { 0, 1 }, { 1, 0 });
		ImGui::Image((void*)m_TextureA->GetID(), size, { 0, 1 }, { 1, 0 });
		ImGui::Image((void*)m_ImageViewID, size, { 0, 1 }, { 1, 0 });
		ImGui::TreePop();
	}

	ImGui::End();
}

void ComputeTestLayer::Resize(uint32_t width, uint32_t height)
{
	m_ViewportWidth = width;
	m_ViewportHeight = height;

	m_FBO->Resize(width, height);
}

bool ComputeTestLayer::OnKeyPressed(Engine::KeyPressedEvent& keyPressedEvent)
{
	return false;
}
