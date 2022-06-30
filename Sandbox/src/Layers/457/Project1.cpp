#include "Layers/457/Project1.h"

#include <imgui/imgui.h>

Project1::Project1()
{

}

Project1::~Project1()
{

}

void Project1::OnAttach()
{
	Engine::ShaderLibrary::Load("assets/457 shaders/Project1.shader");
	Engine::ShaderLibrary::Load("assets/410 shaders/Perlin2D.shader");

	Engine::Application::GetApplication().GetWindow().ToggleMaximize(true);
	m_WhiteTexture = Engine::Texture2D::CreateWhiteTexture();

	m_PerlinSettings = Engine::CreateRef<WorleyPerlinSettings>();

	m_Entity = Engine::CreateRef<Engine::SimpleEntity>(m_Type, "Project1");

	m_SelectablePrimitives.push_back(Engine::PrimitiveType::Sphere);
	m_SelectablePrimitives.push_back(Engine::PrimitiveType::Cube);
	m_SelectablePrimitives.push_back(Engine::PrimitiveType::Quad);
	m_SelectablePrimitives.push_back(Engine::PrimitiveType::Plane);
}

void Project1::OnDetach()
{

}

void Project1::OnUpdate(float deltaTime)
{
	m_Camera.Update(deltaTime);
	Engine::RenderCommand::ClearColor({ 0.0f, 0.0f, 0.0f, 0.0f });
	Engine::RenderCommand::Clear(true, true);
	m_WhiteTexture->BindToSamplerSlot(0);
	m_PerlinSettings->PerlinTexture->BindToSamplerSlot(1);
	Engine::ShaderLibrary::Get("Project1")->Bind();
	Engine::ShaderLibrary::Get("Project1")->UploadUniformInt("u_Texture", 0);
	Engine::ShaderLibrary::Get("Project1")->UploadUniformInt("u_NoiseTexture", 1);
	Engine::ShaderLibrary::Get("Project1")->UploadUniformBool("u_UseNoise", m_UseNoise);
	Engine::ShaderLibrary::Get("Project1")->UploadUniformFloat("u_NoiseFrequency", m_NoiseFrequency);
	Engine::ShaderLibrary::Get("Project1")->UploadUniformFloat("u_NoiseAmplitude", m_NoiseAmplitude);
	Engine::ShaderLibrary::Get("Project1")->UploadUniformFloat("u_SpecularStrength", m_SpecularStrength);
	Engine::ShaderLibrary::Get("Project1")->UploadUniformFloat("u_DiffuseStrength", m_DiffuseStrength);


	Engine::ShaderLibrary::Get("Project1")->UploadUniformFloat("u_ElapsedTime", Engine::Time::Elapsed());
	Engine::ShaderLibrary::Get("Project1")->UploadUniformInt("u_Factor", m_Factor);
	Engine::ShaderLibrary::Get("Project1")->UploadUniformFloat("u_AlphaPercent", m_AlphaPercent);
	Engine::ShaderLibrary::Get("Project1")->UploadUniformFloat("u_Ad", m_AD);
	Engine::ShaderLibrary::Get("Project1")->UploadUniformFloat("u_Bd", m_BD);
	Engine::ShaderLibrary::Get("Project1")->UploadUniformFloat("u_Tol", m_Tolerance);
	Engine::ShaderLibrary::Get("Project1")->UploadUniformFloat("u_Shininess", m_Shininess);
	Engine::ShaderLibrary::Get("Project1")->UploadUniformFloat("u_AmbientStrength", m_AmbientStrength);
	Engine::ShaderLibrary::Get("Project1")->UploadUniformInt("u_Animate", m_Animate ? 1 : 0);

	Engine::ShaderLibrary::Get("Project1")->UploadUniformMat4("u_ModelMatrix", m_Entity->GetEntityTransform()->Transform());
	Engine::ShaderLibrary::Get("Project1")->UploadUniformFloat3("u_BGColor", m_BGColor);
	Engine::ShaderLibrary::Get("Project1")->UploadUniformFloat3("u_DotColor", m_DotColor);
	Engine::ShaderLibrary::Get("Project1")->UploadUniformFloat3("u_LightPosition", m_LightPosition);
	Engine::ShaderLibrary::Get("Project1")->UploadUniformFloat3("u_CameraPosition", m_Camera.GetPosition());
	m_Entity->DrawEntity(m_Camera.GetViewProjection());
}

void Project1::OnEvent(Engine::Event& event)
{
	m_Camera.OnEvent(event);
}

void Project1::OnImGuiRender()
{
	ImGui::Begin("Project 1 Properties");
	ImGui::Checkbox("Use Noise", &m_UseNoise);

	if (m_UseNoise)
	{
		ImGui::DragFloat("Noise Amplitude", &m_NoiseAmplitude, 0.01);
		ImGui::DragFloat("Noise Frequency", &m_NoiseFrequency, 0.01);
	}

	ImGui::DragInt("Factor", &m_Factor, 0.1, 1);
	ImGui::DragFloat("Diameter A", &m_AD, 0.001);
	ImGui::DragFloat("Diameter B", &m_BD, 0.001);
	ImGui::DragFloat("Tolerance", &m_Tolerance, 0.001);
	ImGui::DragFloat("Alpha %", &m_AlphaPercent, 0.001, 0.0, 1.0);
	ImGui::DragFloat3("Background Color", &m_BGColor.x, 0.001);
	ImGui::DragFloat3("Dot Color", &m_DotColor.x, 0.001);
	ImGui::Separator();
	ImGui::DragFloat3("Light Position", &m_LightPosition.x, 0.001);
	ImGui::DragFloat("Shininess", &m_Shininess, 0.001);
	ImGui::DragFloat("Ambient Strength", &m_AmbientStrength, 0.001);
	ImGui::DragFloat("Diffuse Strength", &m_DiffuseStrength, 0.001);
	ImGui::DragFloat("Specular Strength", &m_SpecularStrength, 0.001);
	ImGui::Checkbox("Animate", &m_Animate);

	if (ImGui::BeginCombo("Primitive Type", Engine::MeshFactory::PrimitiveTypeToString(m_Type).c_str()))
	{
		for (auto primitiveType : m_SelectablePrimitives)
		{
			if (ImGui::Selectable(Engine::MeshFactory::PrimitiveTypeToString(primitiveType).c_str()))
			{
				if (m_Type == primitiveType) continue;
				m_Type = primitiveType;
				m_Entity->GetEntityRenderer()->SetPrimtiveType(m_Type);
			}
		}

		ImGui::EndCombo();
	}

	ImGui::End();
}
