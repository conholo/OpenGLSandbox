#include "MainCloudRenderPass.h"

#include "Engine/Core/Application.h"
#include "Engine/Core/Memory.h"
#include "Engine/Scene/SimpleECS/Light.h"
#include "Engine/Scene/SimpleECS/SimpleEntity.h"
#include "Engine/Rendering/RenderCommand.h"
#include "Engine/Rendering/FrameBuffer.h"
#include "Engine/Rendering/Camera.h"

#include <iostream>
#include <glm/gtx/string_cast.hpp>
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
	m_WaterFBO = Engine::CreateRef<Engine::Framebuffer>(cloudFboSpec);

	Engine::Texture2DSpecification blueNoiseSpec =
	{
		Engine::ImageUtils::WrapMode::Repeat,
		Engine::ImageUtils::WrapMode::Repeat,
		Engine::ImageUtils::FilterMode::Linear,
		Engine::ImageUtils::FilterMode::Linear,
		Engine::ImageUtils::ImageInternalFormat::FromImage,
		Engine::ImageUtils::ImageDataLayout::FromImage,
		Engine::ImageUtils::ImageDataType::UByte
	};

	m_BlueNoiseTexture = Engine::CreateRef<Engine::Texture2D>("assets/textures/BlueNoise.png", blueNoiseSpec);
	m_BlackTexture = Engine::Texture2D::CreateBlackTexture();

	m_EditorGrid = Engine::CreateRef<Engine::EditorGrid>();
	Engine::ShaderLibrary::Load("assets/Clouds/WaterPlane.shader");
	m_EditorGrid->SetShader("WaterPlane");
}

MainCloudRenderPass::~MainCloudRenderPass()
{

}

static int GetTextureDisplayIndex(CloudUIType activeUIType)
{
	switch (activeUIType)
	{
	case CloudUIType::Curl:
	case CloudUIType::Perlin:		return 0;
	case CloudUIType::DetailShape:
	case CloudUIType::BaseShape:	return 1;
	}

	return -1;
}


void MainCloudRenderPass::ExecutePass(const Engine::Camera& camera, const Engine::Ref<MainCloudPassData>& passData)
{
	if (passData->WaterData->DrawWater)
	{
		m_WaterFBO->Bind();
		Engine::RenderCommand::Clear(true, false);
		Engine::RenderCommand::SetFaceCullMode(Engine::FaceCullMode::None);
		Engine::ShaderLibrary::Get("WaterPlane")->Bind();
		Engine::ShaderLibrary::Get("WaterPlane")->UploadUniformFloat("u_SeaAmplitude", passData->WaterData->SeaAmplitude);
		Engine::ShaderLibrary::Get("WaterPlane")->UploadUniformFloat("u_SeaFrequency", passData->WaterData->SeaFrequency);
		Engine::ShaderLibrary::Get("WaterPlane")->UploadUniformFloat("u_SeaChoppy", passData->WaterData->SeaChoppy);
		Engine::ShaderLibrary::Get("WaterPlane")->UploadUniformFloat("u_SeaHeight", passData->WaterData->SeaHeight);
		Engine::ShaderLibrary::Get("WaterPlane")->UploadUniformFloat("u_AnimationTime", Engine::Time::Elapsed());
		Engine::ShaderLibrary::Get("WaterPlane")->UploadUniformInt("u_Octaves", passData->WaterData->OceanOctaves);
		Engine::ShaderLibrary::Get("WaterPlane")->UploadUniformInt("u_Steps", passData->WaterData->OceanSteps);
		Engine::ShaderLibrary::Get("WaterPlane")->UploadUniformFloat3("u_CameraPosition", camera.GetPosition());
		Engine::ShaderLibrary::Get("WaterPlane")->UploadUniformFloat2("u_ScreenResolution", Engine::Application::GetApplication().GetWindow().GetDimensions());
		Engine::ShaderLibrary::Get("WaterPlane")->UploadUniformFloat3("u_Sun.Color", passData->SceneRenderPass->GetSunLight()->GetLightColor());
		Engine::ShaderLibrary::Get("WaterPlane")->UploadUniformFloat3("u_Sun.Position", passData->SceneRenderPass->GetSunLight()->GetLightTransform()->GetPosition());
		Engine::ShaderLibrary::Get("WaterPlane")->UploadUniformFloat("u_Sun.Intensity", passData->SceneRenderPass->GetSunLight()->GetLightIntensity());
		Engine::ShaderLibrary::Get("WaterPlane")->UploadUniformFloat("u_FarClip", camera.GetFarClip());
		Engine::ShaderLibrary::Get("WaterPlane")->UploadUniformFloat("u_NearClip", camera.GetNearClip());
		Engine::ShaderLibrary::Get("WaterPlane")->UploadUniformMat4("u_InverseProjection", glm::inverse(camera.GetProjection()));
		Engine::ShaderLibrary::Get("WaterPlane")->UploadUniformMat4("u_InverseView", glm::inverse(camera.GetView()));
		Engine::ShaderLibrary::Get("WaterPlane")->UploadUniformMat4("u_MVP", camera.GetViewProjection() * m_CloudQuad->GetEntityTransform()->Transform());

		m_EditorGrid->Draw(camera);
		m_WaterFBO->Unbind();
	}

	m_CloudFBO->Bind();
	Engine::RenderCommand::ClearColor(m_ClearColor);
	Engine::RenderCommand::Clear(true, false);
	Engine::RenderCommand::SetFaceCullMode(Engine::FaceCullMode::None);

	passData->SceneRenderPass->BindMainColorAttachment(0);
	passData->SceneRenderPass->BindDepthAttachment(1);
	passData->BaseShapeSettings->BaseShapeTexture->BindToSamplerSlot(2);
	passData->PerlinSettings->PerlinTexture->BindToSamplerSlot(3);
	passData->DetailShapeSettings->DetailShapeTexture->BindToSamplerSlot(4);
	passData->CurlSettings->CurlTexture->BindToSamplerSlot(5);
	m_BlueNoiseTexture->BindToSamplerSlot(6);
	if (passData->WaterData->DrawWater)
		m_WaterFBO->BindColorAttachment(0, 7);
	else
		m_BlackTexture->BindToSamplerSlot(7);

	// Textures
	m_CloudQuad->GetEntityRenderer()->GetShader()->Bind();
	m_CloudQuad->GetEntityRenderer()->GetShader()->UploadUniformBool("u_DrawClouds", passData->MainSettings->DrawClouds);
	m_CloudQuad->GetEntityRenderer()->GetShader()->UploadUniformInt("u_SceneTexture", 0);
	m_CloudQuad->GetEntityRenderer()->GetShader()->UploadUniformInt("u_DepthTexture", 1);
	m_CloudQuad->GetEntityRenderer()->GetShader()->UploadUniformInt("u_BaseShapeTexture", 2);
	m_CloudQuad->GetEntityRenderer()->GetShader()->UploadUniformInt("u_WeatherMap", 3);
	m_CloudQuad->GetEntityRenderer()->GetShader()->UploadUniformInt("u_DetailShapeTexture", 4);
	m_CloudQuad->GetEntityRenderer()->GetShader()->UploadUniformInt("u_CurlNoise", 5);
	m_CloudQuad->GetEntityRenderer()->GetShader()->UploadUniformInt("u_BlueNoiseTexture", 6);
	m_CloudQuad->GetEntityRenderer()->GetShader()->UploadUniformInt("u_WaterTexture", 7);

	// Camera Uniforms
	m_CloudQuad->GetEntityRenderer()->GetShader()->UploadUniformFloat("u_NearClip", camera.GetNearClip());
	m_CloudQuad->GetEntityRenderer()->GetShader()->UploadUniformFloat("u_FarClip", camera.GetFarClip());
	m_CloudQuad->GetEntityRenderer()->GetShader()->UploadUniformFloat3("u_WorldSpaceCameraPosition", camera.GetPosition());

	m_CloudQuad->GetEntityRenderer()->GetShader()->UploadUniformMat4("u_InverseProjection", glm::inverse(camera.GetProjection()));
	m_CloudQuad->GetEntityRenderer()->GetShader()->UploadUniformMat4("u_InverseView", glm::inverse(camera.GetView()));

	// Sky / Sun Uniforms
	m_CloudQuad->GetEntityRenderer()->GetShader()->UploadUniformFloat3("u_SkyColorA", passData->MainSettings->SkyColorA);
	m_CloudQuad->GetEntityRenderer()->GetShader()->UploadUniformFloat3("u_SkyColorB", passData->MainSettings->SkyColorB);
	m_CloudQuad->GetEntityRenderer()->GetShader()->UploadUniformFloat3("u_Sun.LightPosition", passData->SceneRenderPass->GetSunLight()->GetLightTransform()->GetPosition());
	m_CloudQuad->GetEntityRenderer()->GetShader()->UploadUniformFloat("u_Sun.Intensity", passData->SceneRenderPass->GetSunLight()->GetLightIntensity());
	m_CloudQuad->GetEntityRenderer()->GetShader()->UploadUniformFloat3("u_Sun.LightColor", passData->SceneRenderPass->GetSunLight()->GetLightColor());
	m_CloudQuad->GetEntityRenderer()->GetShader()->UploadUniformFloat("u_SunSize", passData->MainSettings->SunSize);

	// Animation Uniforms
	m_CloudQuad->GetEntityRenderer()->GetShader()->UploadUniformFloat("u_ElapsedTime", Engine::Time::Elapsed());
	m_CloudQuad->GetEntityRenderer()->GetShader()->UploadUniformFloat("u_TimeScale", passData->AnimationSettings->TimeScale);
	m_CloudQuad->GetEntityRenderer()->GetShader()->UploadUniformFloat3("u_ShapeTextureOffset", passData->AnimationSettings->ShapeTextureOffset);
	m_CloudQuad->GetEntityRenderer()->GetShader()->UploadUniformFloat("u_CloudOffsetScrollSpeed", passData->AnimationSettings->CloudScrollOffsetSpeed);
	m_CloudQuad->GetEntityRenderer()->GetShader()->UploadUniformFloat("u_AnimationSpeed", passData->AnimationSettings->AnimationSpeed);
	m_CloudQuad->GetEntityRenderer()->GetShader()->UploadUniformBool("u_Animate", passData->AnimationSettings->AnimateClouds);

	// Cloud Lighting Uniforms
	m_CloudQuad->GetEntityRenderer()->GetShader()->UploadUniformFloat("u_PowderConstant", passData->MainSettings->PowderConstant);
	m_CloudQuad->GetEntityRenderer()->GetShader()->UploadUniformFloat("u_SilverLiningConstant", passData->MainSettings->SilverLiningConstant);
	glm::vec4 phaseParams = glm::vec4(passData->MainSettings->ForwardScattering, passData->MainSettings->BackScattering, passData->MainSettings->BaseBrightness, passData->MainSettings->PhaseFactor);
	m_CloudQuad->GetEntityRenderer()->GetShader()->UploadUniformFloat4("u_PhaseParams", phaseParams);
	m_CloudQuad->GetEntityRenderer()->GetShader()->UploadUniformFloat("u_PhaseBlend", passData->MainSettings->PhaseBlend);

	// Bounds / Container Uniforms
	m_CloudQuad->GetEntityRenderer()->GetShader()->UploadUniformFloat("u_CloudScale", passData->MainSettings->CloudScale);
	m_CloudQuad->GetEntityRenderer()->GetShader()->UploadUniformFloat3("u_BoundsMin", passData->MainSettings->CloudContainerPosition - passData->MainSettings->CloudContainerScale / 2.0f);
	m_CloudQuad->GetEntityRenderer()->GetShader()->UploadUniformFloat3("u_BoundsMax", passData->MainSettings->CloudContainerPosition + passData->MainSettings->CloudContainerScale / 2.0f);
	m_CloudQuad->GetEntityRenderer()->GetShader()->UploadUniformFloat("u_CloudScaleFactor", passData->MainSettings->CloudScaleFactor);
	m_CloudQuad->GetEntityRenderer()->GetShader()->UploadUniformFloat("u_ContainerEdgeFadeDistance", passData->MainSettings->ContainerEdgeFadeDistance);


	// Density Uniforms
	m_CloudQuad->GetEntityRenderer()->GetShader()->UploadUniformFloat("u_DensityThreshold", passData->MainSettings->DensityThreshold);
	m_CloudQuad->GetEntityRenderer()->GetShader()->UploadUniformFloat("u_DensityMultiplier", passData->MainSettings->DensityMultiplier);

	m_CloudQuad->GetEntityRenderer()->GetShader()->UploadUniformFloat4("u_ShapeNoiseWeights", passData->MainSettings->ShapeNoiseWeights);

	m_CloudQuad->GetEntityRenderer()->GetShader()->UploadUniformFloat3("u_DetailNoiseWeights", passData->MainSettings->DetailNoiseWeights);
	m_CloudQuad->GetEntityRenderer()->GetShader()->UploadUniformFloat("u_DetailNoiseWeight", passData->MainSettings->DetailNoiseWeight);

	m_CloudQuad->GetEntityRenderer()->GetShader()->UploadUniformFloat("u_CurlIntensity", passData->MainSettings->CurlIntensity);
	m_CloudQuad->GetEntityRenderer()->GetShader()->UploadUniformFloat("u_ExtinctionFactor", passData->MainSettings->ExtinctionFactor);

	// Marching Uniforms
	m_CloudQuad->GetEntityRenderer()->GetShader()->UploadUniformInt("u_DensitySteps", passData->MainSettings->DensitySteps);
	m_CloudQuad->GetEntityRenderer()->GetShader()->UploadUniformInt("u_LightSteps", passData->MainSettings->LightSteps);
	m_CloudQuad->GetEntityRenderer()->GetShader()->UploadUniformFloat("u_RandomOffsetStrength", passData->MainSettings->RandomOffsetStrength);

	m_CloudQuad->GetEntityRenderer()->GetShader()->UploadUniformFloat3("u_CloudTypeWeights", passData->MainSettings->CloudTypeWeights);
	m_CloudQuad->GetEntityRenderer()->GetShader()->UploadUniformFloat("u_CloudTypeWeightStrength", passData->MainSettings->CloudTypeWeightStrength);

	// Debug
	m_CloudQuad->GetEntityRenderer()->GetShader()->UploadUniformFloat2("u_ScreenResolution", { Engine::Application::GetApplication().GetWindow().GetWidth(), Engine::Application::GetApplication().GetWindow().GetHeight() });
	m_CloudQuad->GetEntityRenderer()->GetShader()->UploadUniformFloat("u_PercentOfScreen", passData->UI->GetTextureDisplaySettings()->PercentScreenTextureDisplay);
	m_CloudQuad->GetEntityRenderer()->GetShader()->UploadUniformInt("u_DisplayIndex", GetTextureDisplayIndex(passData->UI->GetActiveUIType()));
	m_CloudQuad->GetEntityRenderer()->GetShader()->UploadUniformInt("u_DisplayTexture3D", passData->UI->GetActiveShapeType() == ActiveDebugShapeType::BaseShape ?  2 : 4);
	m_CloudQuad->GetEntityRenderer()->GetShader()->UploadUniformInt("u_DisplayTexture2D",passData->UI->GetActiveUIType() == CloudUIType::Perlin ?  3 : 5);
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
