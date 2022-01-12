#include "Layers/410/CloudsUtility/CloudsSceneRenderPass.h"

#include "Engine/Core/Application.h"
#include "Engine/Rendering/RenderCommand.h"

CloudsSceneRenderPass::CloudsSceneRenderPass()
{
	m_WhiteTexture = Engine::Texture2D::CreateWhiteTexture();

	Engine::LightSpecification sunSpec =
	{
		Engine::LightType::Point,
		glm::vec3(1.0f),
		1.0f
	};

	m_Sun = Engine::CreateRef<Engine::Light>(sunSpec);
	m_Sun->GetLightTransform()->SetPosition({ 30.0f, 300.0f, 0.0f });

	m_GroundPlane = Engine::CreateRef<Engine::SimpleEntity>(Engine::PrimitiveType::Plane, "FlatColor");
	m_GroundPlane->GetEntityTransform()->SetScale({ 10.0f, 1.0f, 10.0f });
	m_GroundPlane->GetEntityTransform()->SetPosition({ 0.0f, -2.0f, 0.0f });

	Engine::FramebufferSpecification fboSpec =
	{
		Engine::Application::GetApplication().GetWindow().GetWidth(),
		Engine::Application::GetApplication().GetWindow().GetHeight(),
		{ Engine::FramebufferTextureFormat::Depth, Engine::FramebufferTextureFormat::RGBA32F }
	};

	m_FBO = Engine::CreateRef<Engine::Framebuffer>(fboSpec);
}

CloudsSceneRenderPass::~CloudsSceneRenderPass()
{

}

void CloudsSceneRenderPass::DrawSceneEntities(const Engine::Camera& camera)
{
	m_WhiteTexture->BindToSamplerSlot(0);
	m_GroundPlane->GetEntityRenderer()->GetShader()->Bind();
	m_GroundPlane->GetEntityRenderer()->GetShader()->UploadUniformFloat3("u_Color", { 1.0f, 0.8f, 0.8f });
	m_GroundPlane->DrawEntity(camera.GetViewProjection());
}

void CloudsSceneRenderPass::ExecutePass(const Engine::Camera& camera)
{
	m_FBO->Bind();
	Engine::RenderCommand::ClearColor(m_ClearColor);
	Engine::RenderCommand::Clear(true, true);
	DrawSceneEntities(camera);
	m_FBO->Unbind();
}

void CloudsSceneRenderPass::Resize(uint32_t width, uint32_t height)
{
	m_FBO->Resize(width, height);
}
