#include "VolumetricCloudsAtmosphereDemoLayer.h"
#include <imgui.h>

#include <glm/gtx/string_cast.hpp>

VolumetricCloudsAtmosphereDemoLayer::VolumetricCloudsAtmosphereDemoLayer()
{

}

VolumetricCloudsAtmosphereDemoLayer::~VolumetricCloudsAtmosphereDemoLayer()
{

}

void VolumetricCloudsAtmosphereDemoLayer::OnAttach()
{
	Engine::ShaderLibrary::Load("assets/shaders/Clouds/Planet.glsl");
	Engine::ShaderLibrary::Load("assets/shaders/Clouds/PlanetAtmosphereComposite.glsl");

	Engine::ShaderLibrary::Load("assets/shaders/Clouds/WorleyGenerator.glsl");
	Engine::ShaderLibrary::Load("assets/shaders/Clouds/NormalizeWorley.glsl");
	Engine::ShaderLibrary::Load("assets/shaders/Clouds/Perlin2D.glsl");
	Engine::ShaderLibrary::Load("assets/shaders/Clouds/Curl.glsl");

	Engine::Application::GetApplication().GetWindow().ToggleMaximize(true);


	Engine::FramebufferSpecification fboSpec =
	{
		Engine::Application::GetApplication().GetWindow().GetWidth(), Engine::Application::GetApplication().GetWindow().GetHeight(),
		{Engine::FramebufferTextureFormat::Depth, Engine::FramebufferTextureFormat::RGBA32F},
	};

	m_FBO = Engine::CreateRef<Engine::Framebuffer>(fboSpec);


	PlanetProperties properties =
	{
		"Test Planet",
		{0.0f, 0.0f, 0.0f},
		{1.0f, 1.0f, 1.0f},
		100.0f, 
		1.0f,
		5.0f
	};

	m_Planet = Engine::CreateRef<Planet>(properties);

	Engine::LightSpecification spec =
	{
		Engine::LightType::Point,
		{1.0f, 1.0f, 1.0f},
		1.0f
	};
	m_Light = Engine::CreateRef<Engine::Light>(spec);
	m_Light->GetLightTransform()->SetPosition({ -30.0f, 75.0f, -15.0f });

	m_FSQ = Engine::CreateRef<Engine::SimpleEntity>(Engine::PrimitiveType::FullScreenQuad, "PlanetAtmosphereComposite");

	m_CloudSettings = Engine::CreateRef<CloudSettings>();
	m_CloudAnimationSettings = Engine::CreateRef<CloudAnimationSettings>();

	m_PerlinSettings = Engine::CreateRef<WorleyPerlinSettings>();
	m_CurlSettings = Engine::CreateRef<CurlSettings>();
	m_BaseShapeSettings = Engine::CreateRef<BaseShapeWorleySettings>();
	m_BaseShapeSettings->UpdateAllChannels(m_PerlinSettings);
	m_DetailShapeSettings = Engine::CreateRef<DetailShapeWorleySettings>();
	m_DetailShapeSettings->UpdateAllChannels();


	m_Camera.SetPosition({ 0.0f, 0.0f, 46.0f });
	m_Planet->SetRadius(8.0);
	m_Planet->GetProperties().AtmosphereRadius = 15.0f;
}

void VolumetricCloudsAtmosphereDemoLayer::OnDetach()
{

}

void VolumetricCloudsAtmosphereDemoLayer::OnUpdate(float deltaTime)
{
	m_Camera.Update(deltaTime);

	m_FBO->Bind();
	Engine::RenderCommand::SetFaceCullMode(Engine::FaceCullMode::Back);
	Engine::RenderCommand::SetDrawMode(m_Wireframe ? Engine::DrawMode::WireFrame : Engine::DrawMode::Fill);
	Engine::RenderCommand::ClearColor({ 0.0f, 0.0f, 0.0f, 0.0f });
	Engine::RenderCommand::Clear(true, true);
	m_Planet->GetRenderer()->GetShader()->Bind();
	m_Planet->GetRenderer()->GetShader()->UploadUniformFloat3("u_CameraPosition", m_Camera.GetPosition());
	m_Planet->GetRenderer()->GetShader()->UploadUniformFloat3("u_Light.Position", m_Light->GetLightTransform()->GetPosition());
	m_Planet->GetRenderer()->GetShader()->UploadUniformFloat3("u_Light.Color", m_Light->GetLightColor());
	m_Planet->GetRenderer()->GetShader()->UploadUniformFloat("u_Light.Intensity", m_Light->GetLightIntensity());
	m_Planet->Draw(m_Camera.GetViewProjection());
	Engine::RenderCommand::SetDrawMode(Engine::DrawMode::Fill);
	m_FBO->Unbind();


	Engine::RenderCommand::ClearColor({ 0.0f, 0.0f, 0.0f, 0.0f });
	Engine::RenderCommand::Clear(true, true);
	m_FSQ->GetEntityRenderer()->GetMaterial().GetShader()->Bind();
	m_FBO->BindColorAttachment(0);
	m_BaseShapeSettings->BaseShapeTexture->BindToSamplerSlot(1);
	m_DetailShapeSettings->DetailShapeTexture->BindToSamplerSlot(2);
	m_PerlinSettings->PerlinTexture->BindToSamplerSlot(3);
	m_CurlSettings->CurlTexture->BindToSamplerSlot(4);
	m_FBO->BindDepthTexture(5);
	m_FSQ->GetEntityRenderer()->GetMaterial().GetShader()->UploadUniformInt("u_SceneTexture", 0);
	m_FSQ->GetEntityRenderer()->GetMaterial().GetShader()->UploadUniformInt("u_ShapeTexture", 1);
	m_FSQ->GetEntityRenderer()->GetMaterial().GetShader()->UploadUniformInt("u_DetailTexture", 2);
	m_FSQ->GetEntityRenderer()->GetMaterial().GetShader()->UploadUniformInt("u_WeatherMap", 3);
	m_FSQ->GetEntityRenderer()->GetMaterial().GetShader()->UploadUniformInt("u_CurlNoise", 4);
	m_FSQ->GetEntityRenderer()->GetMaterial().GetShader()->UploadUniformInt("u_DepthTexture", 5);
	m_FSQ->GetEntityRenderer()->GetMaterial().GetShader()->UploadUniformFloat("u_NearClip", m_Camera.GetNearClip());
	m_FSQ->GetEntityRenderer()->GetMaterial().GetShader()->UploadUniformFloat("u_FarClip", m_Camera.GetFarClip());
	m_FSQ->GetEntityRenderer()->GetMaterial().GetShader()->UploadUniformFloat("u_SunSizeAttenuation", m_SunSizeAttenuation);


	m_FSQ->GetEntityRenderer()->GetMaterial().GetShader()->UploadUniformBool("u_Animate", m_AnimateClouds);
	m_FSQ->GetEntityRenderer()->GetMaterial().GetShader()->UploadUniformFloat("u_AnimationSpeed", m_AnimationSpeed);
	m_FSQ->GetEntityRenderer()->GetMaterial().GetShader()->UploadUniformFloat("u_ElapsedTime", Engine::Time::Elapsed());
	m_FSQ->GetEntityRenderer()->GetMaterial().GetShader()->UploadUniformFloat("u_TimeScale", m_TimeScale);

	m_FSQ->GetEntityRenderer()->GetMaterial().GetShader()->UploadUniformInt("u_DensitySteps", m_DensitySteps);
	m_FSQ->GetEntityRenderer()->GetMaterial().GetShader()->UploadUniformInt("u_LightSteps", m_LightSteps);
	m_FSQ->GetEntityRenderer()->GetMaterial().GetShader()->UploadUniformInt("u_AtmosphericOpticalDepthPoints", m_AtmosphericOpticalDepthPoints);

	m_FSQ->GetEntityRenderer()->GetMaterial().GetShader()->UploadUniformFloat3("u_AtmosphereColor", m_AtmosphereColor);
	m_FSQ->GetEntityRenderer()->GetMaterial().GetShader()->UploadUniformFloat3("u_CloudTintColor", m_CloudTintColor);

	glm::vec4 phaseParams = glm::vec4(m_CloudSettings->ForwardScattering, m_CloudSettings->BackScattering, m_CloudSettings->BaseBrightness, m_CloudSettings->PhaseFactor);
	m_FSQ->GetEntityRenderer()->GetMaterial().GetShader()->UploadUniformFloat4("u_PhaseParams", phaseParams);

	m_FSQ->GetEntityRenderer()->GetMaterial().GetShader()->UploadUniformFloat3("u_Light.Position", m_Light->GetLightTransform()->GetPosition());
	m_FSQ->GetEntityRenderer()->GetMaterial().GetShader()->UploadUniformFloat3("u_Light.Color", m_Light->GetLightColor());
	m_FSQ->GetEntityRenderer()->GetMaterial().GetShader()->UploadUniformFloat("u_Light.Intensity", m_Light->GetLightIntensity());

	m_FSQ->GetEntityRenderer()->GetMaterial().GetShader()->UploadUniformFloat("u_LuminanceMultiplier", m_LuminanceMultiplier);
	m_FSQ->GetEntityRenderer()->GetMaterial().GetShader()->UploadUniformFloat("u_ForwardScatteringConstant", m_ForwardScatteringConstant);
	m_FSQ->GetEntityRenderer()->GetMaterial().GetShader()->UploadUniformFloat("u_ExtinctionFactor", m_ExtinctionFactor);
	m_FSQ->GetEntityRenderer()->GetMaterial().GetShader()->UploadUniformFloat("u_AtmosphereDensityFalloff", m_AtmosphereDensityFalloff);
	m_FSQ->GetEntityRenderer()->GetMaterial().GetShader()->UploadUniformFloat("u_SilverLiningConstant", m_SilverLiningConstant);
	m_FSQ->GetEntityRenderer()->GetMaterial().GetShader()->UploadUniformFloat("u_PowderConstant", m_PowderConstant);

	m_FSQ->GetEntityRenderer()->GetMaterial().GetShader()->UploadUniformFloat("u_ViewerAttenuationFactor", m_ViewerAttenuationFactor);
	m_FSQ->GetEntityRenderer()->GetMaterial().GetShader()->UploadUniformFloat("u_LowerCloudOffsetPct", m_LowerCloudOffsetPct);
	m_FSQ->GetEntityRenderer()->GetMaterial().GetShader()->UploadUniformFloat("u_UpperCloudOffsetPct", m_UpperCloudOffsetPct);
	m_FSQ->GetEntityRenderer()->GetMaterial().GetShader()->UploadUniformFloat("u_DetailHeightModifier", m_BaseShapeErosionFactor);
	m_FSQ->GetEntityRenderer()->GetMaterial().GetShader()->UploadUniformFloat("u_TypeWeightMultiplier", m_TypeWeightMultiplier);

	m_FSQ->GetEntityRenderer()->GetMaterial().GetShader()->UploadUniformFloat("u_CloudScale", m_CloudScale);
	m_FSQ->GetEntityRenderer()->GetMaterial().GetShader()->UploadUniformFloat("u_DensityMultiplier", m_DensityMultiplier);
	m_FSQ->GetEntityRenderer()->GetMaterial().GetShader()->UploadUniformFloat("u_DensityThreshold", m_DensityThreshold);
	m_FSQ->GetEntityRenderer()->GetMaterial().GetShader()->UploadUniformFloat4("u_ShapeNoiseWeights", m_ShapeNoiseWeights);
	m_FSQ->GetEntityRenderer()->GetMaterial().GetShader()->UploadUniformFloat3("u_DetailNoiseWeights", m_DetailNoiseWeights);
	m_FSQ->GetEntityRenderer()->GetMaterial().GetShader()->UploadUniformFloat3("u_CloudTypeWeights", m_CloudTypeWeights);

	m_FSQ->GetEntityRenderer()->GetMaterial().GetShader()->UploadUniformFloat("u_AtmosphereStrength", m_AtmosphereStrength);

	float scatterR = glm::pow(400.0 / m_WaveLengths.r, 4) * m_ScatteringStrength;
	float scatterG = glm::pow(400.0 / m_WaveLengths.g, 4) * m_ScatteringStrength;
	float scatterB = glm::pow(400.0 / m_WaveLengths.b, 4) * m_ScatteringStrength;

	m_FSQ->GetEntityRenderer()->GetMaterial().GetShader()->UploadUniformFloat3("u_AtmosphereScatteringCoefficient", {scatterR, scatterG, scatterB});
	m_FSQ->GetEntityRenderer()->GetMaterial().GetShader()->UploadUniformFloat("u_AtmosphereRadius", m_Planet->GetProperties().AtmosphereRadius);
	m_FSQ->GetEntityRenderer()->GetMaterial().GetShader()->UploadUniformFloat("u_PlanetRadius", m_Planet->GetProperties().Radius);
	m_FSQ->GetEntityRenderer()->GetMaterial().GetShader()->UploadUniformFloat3("u_PlanetPosition", m_Planet->GetProperties().Position);
	m_FSQ->GetEntityRenderer()->GetMaterial().GetShader()->UploadUniformMat4("u_InverseProjection", glm::inverse(m_Camera.GetProjection()));
	m_FSQ->GetEntityRenderer()->GetMaterial().GetShader()->UploadUniformMat4("u_InverseView", glm::inverse(m_Camera.GetView()));
	m_FSQ->GetEntityRenderer()->GetMaterial().GetShader()->UploadUniformFloat3("u_CameraPosition", m_Camera.GetPosition());
	m_FSQ->DrawEntity(m_Camera.GetViewProjection());
}

void VolumetricCloudsAtmosphereDemoLayer::OnEvent(Engine::Event& e)
{
	m_Camera.OnEvent(e);
}

void VolumetricCloudsAtmosphereDemoLayer::OnImGuiRender()
{
	ImGui::Begin("Planet/Cloud Editor");
	if (ImGui::TreeNodeEx("Planet Material Properties"))
	{
		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
		ImGui::ColorEdit3("Diffuse Color", &m_Planet->GetMaterialProperties().Diffuse.x, ImGuiColorEditFlags_NoInputs);
		ImGui::PopStyleVar();
		ImGui::DragFloat("Diffuse Strength", &m_Planet->GetMaterialProperties().Diffuse.w, 0.01f);
		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
		ImGui::ColorEdit3("Ambient Color", &m_Planet->GetMaterialProperties().Ambient.x, ImGuiColorEditFlags_NoInputs);
		ImGui::PopStyleVar();
		ImGui::DragFloat("Ambient Strength", &m_Planet->GetMaterialProperties().Ambient.w, 0.01f);
		ImGui::DragFloat("Specular Strength", &m_Planet->GetMaterialProperties().SpecularStrength, 0.01f);
		ImGui::DragFloat("Shininess", &m_Planet->GetMaterialProperties().Shininess, 0.01f);

		ImGui::TreePop();
	}

	if (ImGui::TreeNodeEx("Planet Physical Properties"))
	{
		ImGui::Checkbox("Wireframe", &m_Wireframe);
		if (ImGui::DragInt("Mesh Resolution", &m_MeshResolution, 0.1, 0, 6))
			m_Planet->UpdateMesh(m_MeshResolution);

		glm::vec3 scale = m_Planet->GetTransform()->GetScale();
		std::string scaleText = "Planet Scale: " + glm::to_string(scale);
		ImGui::Text(scaleText.c_str());
		std::string camPositionText = "Camera Position: " + glm::to_string(m_Camera.GetPosition());
		ImGui::Text(camPositionText.c_str());

		float radius = m_Planet->GetProperties().Radius;
		if (ImGui::DragFloat("Planet Radius", &radius, 0.1))
			m_Planet->SetRadius(radius);
		ImGui::DragFloat("Atmospheric Radius", &m_Planet->GetProperties().AtmosphereRadius, 0.1);
		ImGui::DragFloat("Lower Cloud Offset %", &m_LowerCloudOffsetPct, 0.01);
		ImGui::DragFloat("Upper Cloud Offset %", &m_UpperCloudOffsetPct, 0.01);
		ImGui::DragFloat("Viewer Distance Attenuation %", &m_ViewerAttenuationFactor, 0.01);

		ImGui::TreePop();
	}

	if (ImGui::TreeNodeEx("Cloud Density Settings"))
	{
		ImGui::DragInt("Density Steps", &m_DensitySteps, 0.1);
		ImGui::DragFloat("Cloud Scale", &m_CloudScale, 0.1);
		ImGui::DragFloat("Density Multiplier", &m_DensityMultiplier, 0.01);
		ImGui::DragFloat("Density Threshold", &m_DensityThreshold, 0.01);
		ImGui::DragFloat("Base Shape Erosion", &m_BaseShapeErosionFactor, 0.01);
		ImGui::DragFloat4("Shape Noise Weights", &m_ShapeNoiseWeights.x, 0.01);
		ImGui::DragFloat3("Detail Noise Weights", &m_DetailNoiseWeights.x, 0.01);
		ImGui::DragFloat3("Cloud Type Weights", &m_CloudTypeWeights.x, 0.01);
		ImGui::DragFloat("Type Weight Multiplier", &m_TypeWeightMultiplier, 0.01);

		ImGui::TreePop();
	}

	if (ImGui::TreeNodeEx("Cloud Lighting Settings"))
	{
		ImGui::DragInt("Light Steps", &m_LightSteps, 0.1);
		ImGui::DragFloat("Luminance Multiplier", &m_LuminanceMultiplier, 0.01);
		ImGui::DragFloat("Extinction Factor", &m_ExtinctionFactor, 0.01);
		ImGui::DragFloat("Forward Scattering Constant", &m_ForwardScatteringConstant, 0.01);
		ImGui::DragFloat("Silver Lining Constant", &m_SilverLiningConstant, 0.01);
		ImGui::DragFloat("Powder Constant", &m_PowderConstant, 0.01);
		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
		ImGui::ColorEdit3("Cloud Tint Color", &m_CloudTintColor.x, ImGuiColorEditFlags_NoInputs);
		ImGui::PopStyleVar();

		ImGui::TreePop();
	}

	if (ImGui::TreeNodeEx("Atmosphere Lighting Settings"))
	{
		ImGui::DragInt("Atmospheric Optical Depth Points", &m_AtmosphericOpticalDepthPoints, 0.01);
		ImGui::DragFloat("Atmospheric Strength", &m_AtmosphereStrength, 0.01);
		ImGui::DragFloat("Atmosphere Density Falloff", &m_AtmosphereDensityFalloff, 0.01);
		ImGui::DragFloat3("Scatter Wavelengths", &m_WaveLengths.x, 0.1);
		ImGui::DragFloat("Scattering Strength", &m_ScatteringStrength, 0.01);
		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
		ImGui::ColorEdit3("Atmosphere Color", &m_AtmosphereColor.x, ImGuiColorEditFlags_NoInputs);
		ImGui::PopStyleVar();
		ImGui::TreePop();
	}

	if (ImGui::TreeNodeEx("Animation"))
	{
		ImGui::Checkbox("Animate", &m_AnimateClouds);
		ImGui::DragFloat("Animation Speed", &m_AnimationSpeed, 0.01);
		ImGui::DragFloat("Time Scale", &m_TimeScale, 0.001);

		ImGui::TreePop();
	}

	if (ImGui::TreeNodeEx("Sun Settings"))
	{
		ImGui::DragFloat("Sun Size", &m_SunSizeAttenuation, 0.01f);
		ImGui::DragFloat3("Light Position", &m_Light->GetLightTransform()->GetPosition().x, 0.1f);
		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
		ImGui::ColorEdit3("Light Color", &m_Light->GetLightColor().x, ImGuiColorEditFlags_NoInputs);
		ImGui::PopStyleVar();
		ImGui::TreePop();
	}


	ImGui::End();
}
