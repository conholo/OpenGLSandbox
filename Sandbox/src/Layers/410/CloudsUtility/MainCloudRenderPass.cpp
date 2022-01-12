#include "Layers/410/CloudsUtility/MainCloudRenderPass.h"


#include "Engine/Core/Application.h"
#include "Engine/Core/Memory.h"
#include "Engine/Scene/SimpleECS/Light.h"
#include "Engine/Scene/SimpleECS/SimpleEntity.h"
#include "Engine/Rendering/RenderCommand.h"
#include "Engine/Rendering/FrameBuffer.h"
#include "Engine/Rendering/Camera.h"

#include <glm/glm.hpp>

MainCloudRenderPass::MainCloudRenderPass()
{
	Engine::FramebufferSpecification cloudFboSpec =
	{
		Engine::Application::GetApplication().GetWindow().GetWidth(),
		Engine::Application::GetApplication().GetWindow().GetHeight(),
		{ Engine::FramebufferTextureFormat::RGBA32F }
	};

	m_CloudQuad = Engine::CreateRef<Engine::SimpleEntity>(Engine::PrimitiveType::FullScreenQuad, "Clouds");
	m_CloudFBO = Engine::CreateRef<Engine::Framebuffer>(cloudFboSpec);
}

MainCloudRenderPass::~MainCloudRenderPass()
{

}

static int GetTextureDisplayIndex(CloudUIType activeUIType)
{
	switch (activeUIType)
	{
	case CloudUIType::Perlin:		return 0;
	case CloudUIType::DetailShape:
	case CloudUIType::BaseShape:	return 1;
	}

	return -1;
}

void MainCloudRenderPass::ExecutePass(const Engine::Camera& camera, const Engine::Ref<MainCloudPassData>& passData)
{
	m_CloudFBO->Bind();
	Engine::RenderCommand::ClearColor(m_ClearColor);
	Engine::RenderCommand::Clear(true, false);
	Engine::RenderCommand::SetFaceCullMode(Engine::FaceCullMode::None);

	passData->SceneRenderPass->BindMainColorAttachment(0);
	passData->SceneRenderPass->BindDepthAttachment(1);
	passData->BaseShapeSettings->BaseShapeTexture->BindToSamplerSlot(2);
	passData->PerlinSettings->PerlinTexture->BindToSamplerSlot(3);
	passData->DetailShapeSettings->DetailShapeTexture->BindToSamplerSlot(4);

	m_CloudQuad->GetEntityRenderer()->GetShader()->Bind();
	m_CloudQuad->GetEntityRenderer()->GetShader()->UploadUniformInt("u_SceneTexture", 0);
	m_CloudQuad->GetEntityRenderer()->GetShader()->UploadUniformInt("u_DepthTexture", 1);
	m_CloudQuad->GetEntityRenderer()->GetShader()->UploadUniformInt("u_BaseShapeTexture", 2);
	m_CloudQuad->GetEntityRenderer()->GetShader()->UploadUniformInt("u_WeatherMap", 3);
	m_CloudQuad->GetEntityRenderer()->GetShader()->UploadUniformInt("u_DetailShapeTexture", 4);

	m_CloudQuad->GetEntityRenderer()->GetShader()->UploadUniformFloat("u_NearClip", camera.GetNearClip());
	m_CloudQuad->GetEntityRenderer()->GetShader()->UploadUniformFloat("u_FarClip", camera.GetFarClip());
	m_CloudQuad->GetEntityRenderer()->GetShader()->UploadUniformFloat3("u_WorldSpaceCameraPosition", camera.GetPosition());
	m_CloudQuad->GetEntityRenderer()->GetShader()->UploadUniformMat4("u_InverseProjection", glm::inverse(camera.GetProjection()));
	m_CloudQuad->GetEntityRenderer()->GetShader()->UploadUniformMat4("u_InverseView", glm::inverse(camera.GetView()));


	m_CloudQuad->GetEntityRenderer()->GetShader()->UploadUniformFloat3("u_Sun.LightPosition", passData->SceneRenderPass->GetSunLight()->GetLightTransform()->GetPosition());
	m_CloudQuad->GetEntityRenderer()->GetShader()->UploadUniformFloat("u_Sun.Intensity", passData->SceneRenderPass->GetSunLight()->GetLightIntensity());
	m_CloudQuad->GetEntityRenderer()->GetShader()->UploadUniformFloat3("u_Sun.LightColor", passData->SceneRenderPass->GetSunLight()->GetLightColor());

	m_CloudQuad->GetEntityRenderer()->GetShader()->UploadUniformFloat("u_ElapsedTime", Engine::Time::Elapsed());
	m_CloudQuad->GetEntityRenderer()->GetShader()->UploadUniformFloat("u_TimeScale", passData->AnimationSettings->TimeScale);
	m_CloudQuad->GetEntityRenderer()->GetShader()->UploadUniformFloat3("u_ShapeTextureOffset", passData->AnimationSettings->ShapeTextureOffset);
	m_CloudQuad->GetEntityRenderer()->GetShader()->UploadUniformFloat("u_CloudOffsetScrollSpeed", passData->AnimationSettings->CloudScrollOffsetSpeed);
	m_CloudQuad->GetEntityRenderer()->GetShader()->UploadUniformFloat("u_AnimationSpeed", passData->AnimationSettings->AnimationSpeed);
	m_CloudQuad->GetEntityRenderer()->GetShader()->UploadUniformBool("u_Animate", passData->AnimationSettings->AnimateClouds);

	m_CloudQuad->GetEntityRenderer()->GetShader()->UploadUniformFloat3("u_SkyColorA", passData->MainSettings->SkyColorA);
	m_CloudQuad->GetEntityRenderer()->GetShader()->UploadUniformFloat3("u_SkyColorB", passData->MainSettings->SkyColorB);
	m_CloudQuad->GetEntityRenderer()->GetShader()->UploadUniformInt("u_DensitySteps", passData->MainSettings->DensitySteps);
	m_CloudQuad->GetEntityRenderer()->GetShader()->UploadUniformInt("u_LightSteps", passData->MainSettings->LightSteps);
	m_CloudQuad->GetEntityRenderer()->GetShader()->UploadUniformFloat("u_ContainerEdgeFadeDistance", passData->MainSettings->ContainerEdgeFadeDistance);
	m_CloudQuad->GetEntityRenderer()->GetShader()->UploadUniformFloat("u_PhaseBlend", passData->MainSettings->PhaseBlend);
	m_CloudQuad->GetEntityRenderer()->GetShader()->UploadUniformFloat("u_CloudScale", passData->MainSettings->CloudScale);
	m_CloudQuad->GetEntityRenderer()->GetShader()->UploadUniformFloat("u_SilverLiningConstant", passData->MainSettings->SilverLiningConstant);
	m_CloudQuad->GetEntityRenderer()->GetShader()->UploadUniformFloat4("u_PhaseParams", passData->MainSettings->PhaseParams);
	m_CloudQuad->GetEntityRenderer()->GetShader()->UploadUniformFloat("u_PowderConstant", passData->MainSettings->PowderConstant);
	m_CloudQuad->GetEntityRenderer()->GetShader()->UploadUniformFloat("u_DensityThreshold", passData->MainSettings->DensityThreshold);
	m_CloudQuad->GetEntityRenderer()->GetShader()->UploadUniformFloat("u_DensityMultiplier", passData->MainSettings->DensityMultiplier);
	m_CloudQuad->GetEntityRenderer()->GetShader()->UploadUniformFloat("u_DetailNoiseWeight", passData->MainSettings->DetailNoiseWeight);
	m_CloudQuad->GetEntityRenderer()->GetShader()->UploadUniformFloat4("u_ShapeNoiseWeights", passData->MainSettings->ShapeNoiseWeights);
	m_CloudQuad->GetEntityRenderer()->GetShader()->UploadUniformFloat3("u_DetailNoiseWeights", passData->MainSettings->DetailNoiseWeights);
	m_CloudQuad->GetEntityRenderer()->GetShader()->UploadUniformFloat3("u_BoundsMin", passData->MainSettings->CloudContainerPosition - passData->MainSettings->CloudContainerScale / 2.0f);
	m_CloudQuad->GetEntityRenderer()->GetShader()->UploadUniformFloat3("u_BoundsMax", passData->MainSettings->CloudContainerPosition + passData->MainSettings->CloudContainerScale / 2.0f);

	// Debug
	m_CloudQuad->GetEntityRenderer()->GetShader()->UploadUniformFloat2("u_ScreenResolution", { Engine::Application::GetApplication().GetWindow().GetWidth(), Engine::Application::GetApplication().GetWindow().GetHeight() });
	m_CloudQuad->GetEntityRenderer()->GetShader()->UploadUniformFloat("u_PercentOfScreen", passData->UI->GetTextureDisplaySettings()->PercentScreenTextureDisplay);
	m_CloudQuad->GetEntityRenderer()->GetShader()->UploadUniformInt("u_DisplayIndex", GetTextureDisplayIndex(passData->UI->GetActiveUIType()));
	m_CloudQuad->GetEntityRenderer()->GetShader()->UploadUniformInt("u_DisplayTexture3D", passData->UI->GetActiveShapeType() == ActiveDebugShapeType::BaseShape ?  2 : 4);
	m_CloudQuad->GetEntityRenderer()->GetShader()->UploadUniformInt("u_DisplayTexture2D", 3);
	m_CloudQuad->GetEntityRenderer()->GetShader()->UploadUniformBool("u_ShowAlpha", passData->UI->GetDisplayAlpha());
	m_CloudQuad->GetEntityRenderer()->GetShader()->UploadUniformBool("u_ShowAllChannels", passData->UI->GetShowAllChannels());
	m_CloudQuad->GetEntityRenderer()->GetShader()->UploadUniformBool("u_GreyScale", passData->UI->GetEnableGreyScale());
	m_CloudQuad->GetEntityRenderer()->GetShader()->UploadUniformFloat("u_DepthSlice", passData->UI->GetDepthSlice());
	m_CloudQuad->GetEntityRenderer()->GetShader()->UploadUniformFloat4("u_ChannelWeights", passData->UI->GetChannelWeights());

	m_CloudQuad->DrawEntity(camera.GetViewProjection());
	m_CloudFBO->Unbind();
	Engine::RenderCommand::SetFaceCullMode(Engine::FaceCullMode::Back);
}

void MainCloudRenderPass::Resize(uint32_t width, uint32_t height)
{
	m_CloudFBO->Resize(width, height);
}
