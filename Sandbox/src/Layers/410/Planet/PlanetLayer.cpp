#include "Layers/410/Planet/PlanetLayer.h"
#include <imgui/imgui.h>

#include <glm/gtx/string_cast.hpp>

PlanetLayer::PlanetLayer()
{

}

PlanetLayer::~PlanetLayer()
{

}

void PlanetLayer::OnAttach()
{
	Engine::ShaderLibrary::Load("assets/410 shaders/Planet.shader");
	Engine::ShaderLibrary::Load("assets/410 shaders/PlanetAtmosphereComposite.shader");

	Engine::ShaderLibrary::Load("assets/410 shaders/WorleyGenerator.shader");
	Engine::ShaderLibrary::Load("assets/410 shaders/Perlin2D.shader");

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
	m_Light->GetLightTransform()->SetPosition({ 0.0f, 20.0f, 0.0f });

	m_FSQ = Engine::CreateRef<Engine::SimpleEntity>(Engine::PrimitiveType::FullScreenQuad, "PlanetAtmosphereComposite");

	m_PerlinSettings = Engine::CreateRef<WorleyPerlinSettings>();
	m_ShapeSettings = Engine::CreateRef<BaseShapeWorleySettings>();
	m_ShapeSettings->UpdateAllChannels(m_PerlinSettings);
}

void PlanetLayer::OnDetach()
{

}

void PlanetLayer::OnUpdate(float deltaTime)
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
	m_FSQ->GetEntityRenderer()->GetShader()->Bind();
	m_FBO->BindColorAttachment(0);
	m_ShapeSettings->BaseShapeTexture->BindToSamplerSlot(1);
	m_FBO->BindDepthTexture(2);
	m_FSQ->GetEntityRenderer()->GetShader()->UploadUniformInt("u_SceneTexture", 0);
	m_FSQ->GetEntityRenderer()->GetShader()->UploadUniformInt("u_ShapeTexture", 1);
	m_FSQ->GetEntityRenderer()->GetShader()->UploadUniformInt("u_DepthTexture", 2);
	m_FSQ->GetEntityRenderer()->GetShader()->UploadUniformFloat("u_NearClip", m_Camera.GetNearClip());
	m_FSQ->GetEntityRenderer()->GetShader()->UploadUniformFloat("u_FarClip", m_Camera.GetFarClip());

	m_FSQ->GetEntityRenderer()->GetShader()->UploadUniformBool("u_Animate", m_AnimateClouds);
	m_FSQ->GetEntityRenderer()->GetShader()->UploadUniformFloat("u_AnimationSpeed", m_AnimationSpeed);
	m_FSQ->GetEntityRenderer()->GetShader()->UploadUniformFloat("u_ElapsedTime", Engine::Time::Elapsed());
	m_FSQ->GetEntityRenderer()->GetShader()->UploadUniformFloat("u_TimeScale", m_TimeScale);

	m_FSQ->GetEntityRenderer()->GetShader()->UploadUniformInt("u_DensitySteps", m_DensitySteps);
	m_FSQ->GetEntityRenderer()->GetShader()->UploadUniformInt("u_LightSteps", m_LightSteps);

	m_FSQ->GetEntityRenderer()->GetShader()->UploadUniformFloat3("u_Light.Position", m_Light->GetLightTransform()->GetPosition());
	m_FSQ->GetEntityRenderer()->GetShader()->UploadUniformFloat3("u_Light.Color", m_Light->GetLightColor());
	m_FSQ->GetEntityRenderer()->GetShader()->UploadUniformFloat("u_Light.Intensity", m_Light->GetLightIntensity());

	m_FSQ->GetEntityRenderer()->GetShader()->UploadUniformFloat("u_LuminanceMultiplier", m_LuminanceMultiplier);

	m_FSQ->GetEntityRenderer()->GetShader()->UploadUniformFloat("u_ViewerAttenuationFactor", m_ViewerAttenuationFactor);
	m_FSQ->GetEntityRenderer()->GetShader()->UploadUniformFloat("u_LowerCloudOffsetPct", m_LowerCloudOffsetPct);
	m_FSQ->GetEntityRenderer()->GetShader()->UploadUniformFloat("u_UpperCloudOffsetPct", m_UpperCloudOffsetPct);

	m_FSQ->GetEntityRenderer()->GetShader()->UploadUniformFloat("u_CloudScale", m_CloudScale);
	m_FSQ->GetEntityRenderer()->GetShader()->UploadUniformFloat("u_DensityMultiplier", m_DensityMultiplier);
	m_FSQ->GetEntityRenderer()->GetShader()->UploadUniformFloat("u_DensityThreshold", m_DensityThreshold);
	m_FSQ->GetEntityRenderer()->GetShader()->UploadUniformFloat4("u_ShapeNoiseWeights", m_ShapeNoiseWeights);

	m_FSQ->GetEntityRenderer()->GetShader()->UploadUniformFloat("u_AtmosphereStrength", m_AtmosphereStrength);
	m_FSQ->GetEntityRenderer()->GetShader()->UploadUniformFloat("u_AtmosphereRadius", m_Planet->GetProperties().AtmosphereRadius);
	m_FSQ->GetEntityRenderer()->GetShader()->UploadUniformFloat("u_PlanetRadius", m_Planet->GetProperties().Radius);
	m_FSQ->GetEntityRenderer()->GetShader()->UploadUniformFloat3("u_PlanetPosition", m_Planet->GetProperties().Position);
	m_FSQ->GetEntityRenderer()->GetShader()->UploadUniformMat4("u_InverseProjection", glm::inverse(m_Camera.GetProjection()));
	m_FSQ->GetEntityRenderer()->GetShader()->UploadUniformMat4("u_InverseView", glm::inverse(m_Camera.GetView()));
	m_FSQ->GetEntityRenderer()->GetShader()->UploadUniformFloat3("u_CameraPosition", m_Camera.GetPosition());
	m_FSQ->DrawEntity(m_Camera.GetViewProjection());
}

void PlanetLayer::OnEvent(Engine::Event& e)
{
	m_Camera.OnEvent(e);
}

void PlanetLayer::OnImGuiRender()
{
	ImGui::Begin("Planet Settings");
	ImGui::Checkbox("Wireframe", &m_Wireframe);
	if (ImGui::TreeNodeEx("Material Properties"))
	{
		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
		ImGui::ColorEdit3("Diffuse Color", &m_Planet->GetMaterialProperties().DiffuseColor.x, ImGuiColorEditFlags_NoInputs);
		ImGui::PopStyleVar();
		ImGui::DragFloat("Diffuse Strength", &m_Planet->GetMaterialProperties().DiffuseStrength, 0.01f);
		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
		ImGui::ColorEdit3("Ambient Color", &m_Planet->GetMaterialProperties().AmbientColor.x, ImGuiColorEditFlags_NoInputs);
		ImGui::PopStyleVar();
		ImGui::DragFloat("Ambient Strength", &m_Planet->GetMaterialProperties().AmbientStrength, 0.01f);
		ImGui::DragFloat("Specular Strength", &m_Planet->GetMaterialProperties().SpecularStrength, 0.01f);
		ImGui::DragFloat("Shininess", &m_Planet->GetMaterialProperties().Shininess, 0.01f);

		ImGui::TreePop();
	}

	float radius = m_Planet->GetProperties().Radius;
	if (ImGui::DragFloat("Planet Radius", &radius, 0.1))
		m_Planet->SetRadius(radius);
	ImGui::DragFloat("Atmospheric Radius", &m_Planet->GetProperties().AtmosphereRadius, 0.1);
	ImGui::DragFloat("Atmospheric Strength", &m_AtmosphereStrength, 0.01);

	ImGui::DragInt("Density Steps", &m_DensitySteps, 0.1);
	ImGui::DragInt("Light Steps", &m_LightSteps, 0.1);

	glm::vec3 scale = m_Planet->GetTransform()->GetScale();
	std::string scaleText = "Planet Scale: " + glm::to_string(scale);
	ImGui::Text(scaleText.c_str());
	std::string camPositionText = "Camera Position: " + glm::to_string(m_Camera.GetPosition());
	ImGui::Text(camPositionText.c_str());

	ImGui::DragFloat("Lower Cloud Offset %", &m_LowerCloudOffsetPct, 0.01);
	ImGui::DragFloat("Upper Cloud Offset %", &m_UpperCloudOffsetPct, 0.01);
	ImGui::DragFloat("Viewer Attenuation Factor %", &m_ViewerAttenuationFactor, 0.01);
	ImGui::DragFloat("Cloud Scale", &m_CloudScale, 0.1);
	ImGui::DragFloat("Luminance Multiplier", &m_LuminanceMultiplier, 0.01);
	ImGui::DragFloat("Density Multiplier", &m_DensityMultiplier, 0.01);
	ImGui::DragFloat("Density Threshold", &m_DensityThreshold, 0.01);
	ImGui::DragFloat4("Shape Noise Weights", &m_ShapeNoiseWeights.x, 0.01);


	ImGui::Checkbox("Animate", &m_AnimateClouds);
	ImGui::DragFloat("Animation Speed", &m_AnimationSpeed, 0.01);
	ImGui::DragFloat("Time Scale", &m_TimeScale, 0.001);

	ImGui::DragFloat3("Light Position", &m_Light->GetLightTransform()->GetPosition().x, 0.1f);
	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
	ImGui::ColorEdit3("Light Color", &m_Light->GetLightColor().x, ImGuiColorEditFlags_NoInputs);
	ImGui::PopStyleVar();


	if (ImGui::DragInt("Mesh Resolution", &m_MeshResolution, 0.1, 0, 6))
		m_Planet->UpdateMesh(m_MeshResolution);
	ImGui::End();
}
