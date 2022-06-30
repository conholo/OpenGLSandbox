#include "Layers/VolumetricLayer.h"
#include <imgui/imgui.h>

VolumetricLayer::VolumetricLayer()
{

}

VolumetricLayer::~VolumetricLayer()
{

}

void VolumetricLayer::OnAttach()
{
	Engine::ShaderLibrary::Load("assets/volumetric-shaders/LavaLamp.shader");

	Engine::Application::GetApplication().GetWindow().ToggleMaximize(true);
	Engine::FramebufferSpecification fboSpec =
	{
		Engine::Application::GetApplication().GetWindow().GetWidth(), Engine::Application::GetApplication().GetWindow().GetHeight(),
		{Engine::FramebufferTextureFormat::Depth, Engine::FramebufferTextureFormat::RGBA32F},
	};

	m_FBO = Engine::CreateRef<Engine::Framebuffer>(fboSpec);

	m_WhiteTexture = Engine::Texture2D::CreateWhiteTexture();
	m_FSQ = Engine::CreateRef<Engine::SimpleEntity>(Engine::PrimitiveType::FullScreenQuad, "LavaLamp");
	m_GroundPlane = Engine::CreateRef<Engine::SimpleEntity>(Engine::PrimitiveType::Plane, "FlatColor");
	m_GroundPlane->GetEntityTransform()->SetPosition({ 0.0f, -2.0f, 0.0f });
	m_GroundPlane->GetEntityTransform()->SetScale({5.0f, 5.0f, 5.0f});

	Engine::LightSpecification spec =
	{
		Engine::LightType::Point,
		{1.0f, 1.0f, 1.0f},
		1.0f
	};

	m_Light = Engine::CreateRef<Engine::Light>(spec);
	m_Light->GetLightTransform()->SetPosition({ -3.0f, 15.0f, -5.5f });
}

void VolumetricLayer::OnDetach()
{

}

void VolumetricLayer::OnUpdate(float deltaTime)
{
	m_Camera.Update(deltaTime);

	m_FBO->Bind();
	Engine::RenderCommand::ClearColor({ 0.0f, 0.0f, 0.0f, 0.0f });
	Engine::RenderCommand::Clear(true, true);
	m_WhiteTexture->BindToSamplerSlot(0);
	m_GroundPlane->GetEntityRenderer()->GetShader()->Bind();
	m_GroundPlane->GetEntityRenderer()->GetShader()->UploadUniformFloat3("u_Color", {1.0f, 1.0f, 1.0f});
	m_GroundPlane->GetEntityRenderer()->GetShader()->UploadUniformInt("u_Texture", 0);
	m_GroundPlane->DrawEntity(m_Camera.GetViewProjection());
	m_FBO->Unbind();


	Engine::RenderCommand::ClearColor({ 0.0f, 0.0f, 0.0f, 0.0f });
	Engine::RenderCommand::Clear(true, true);

	m_FBO->BindColorAttachment(0);
	m_FBO->BindDepthTexture(1);
	m_FSQ->GetEntityRenderer()->GetShader()->Bind();
	m_FSQ->GetEntityRenderer()->GetShader()->UploadUniformInt("u_SceneTexture", 0);
	m_FSQ->GetEntityRenderer()->GetShader()->UploadUniformInt("u_DepthTexture", 1);

	m_FSQ->GetEntityRenderer()->GetShader()->UploadUniformFloat3("u_Light.Position", m_Light->GetLightTransform()->GetPosition());
	m_FSQ->GetEntityRenderer()->GetShader()->UploadUniformFloat3("u_Light.Color", m_Light->GetLightColor());
	m_FSQ->GetEntityRenderer()->GetShader()->UploadUniformFloat("u_Light.Intensity", m_Light->GetLightIntensity());


	m_FSQ->GetEntityRenderer()->GetShader()->UploadUniformFloat("u_AtmosphereDensityFalloff", m_AtmosphereDensityFalloff);
	m_FSQ->GetEntityRenderer()->GetShader()->UploadUniformFloat3("u_AbsorptionCoefficient", m_AbsorptionCoefficient);
	m_FSQ->GetEntityRenderer()->GetShader()->UploadUniformFloat3("u_ScatteringCoefficient", m_ScatteringCoefficient);

	m_FSQ->GetEntityRenderer()->GetShader()->UploadUniformInt("u_DensitySteps", m_DensitySteps);
	m_FSQ->GetEntityRenderer()->GetShader()->UploadUniformInt("u_LightSteps", m_LightSteps);
	m_FSQ->GetEntityRenderer()->GetShader()->UploadUniformFloat3("u_SphereAPosition", m_SphereAPosition);
	m_FSQ->GetEntityRenderer()->GetShader()->UploadUniformFloat("u_SphereARadius", m_SphereARadius);

	m_FSQ->GetEntityRenderer()->GetShader()->UploadUniformMat4("u_InverseProjection", glm::inverse(m_Camera.GetProjection()));
	m_FSQ->GetEntityRenderer()->GetShader()->UploadUniformMat4("u_InverseView", glm::inverse(m_Camera.GetView()));
	m_FSQ->GetEntityRenderer()->GetShader()->UploadUniformFloat3("u_CameraPosition", m_Camera.GetPosition());
	m_FSQ->GetEntityRenderer()->GetShader()->UploadUniformFloat("u_NearClip", m_Camera.GetNearClip());
	m_FSQ->GetEntityRenderer()->GetShader()->UploadUniformFloat("u_FarClip", m_Camera.GetFarClip());
	m_FSQ->DrawEntity(m_Camera.GetViewProjection());
}

void VolumetricLayer::OnEvent(Engine::Event& e)
{
	m_Camera.OnEvent(e);
}

void VolumetricLayer::OnImGuiRender()
{
	ImGui::Begin("Lava Lamp");
	ImGui::DragFloat3("Absorption Coefficient", &m_AbsorptionCoefficient.x, 0.01);
	ImGui::DragFloat3("Scattering Coefficient", &m_ScatteringCoefficient.x, 0.01);
	ImGui::DragFloat("Atmosphere Density Falloff", &m_AtmosphereDensityFalloff, 0.01);

	ImGui::DragFloat3("Sphere A Position", &m_SphereAPosition.x, 0.01);
	ImGui::DragFloat("Sphere A Radius", &m_SphereARadius, 0.01);
	ImGui::DragInt("Density Steps", &m_DensitySteps, 0.01);
	ImGui::DragInt("Light Steps", &m_LightSteps, 0.01);
	ImGui::End();
}
