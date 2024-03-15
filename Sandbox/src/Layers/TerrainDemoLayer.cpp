#include "TerrainDemoLayer.h"

#include <imgui.h>
#include <iostream>
#include <glm/gtx/string_cast.hpp>

TerrainDemoLayer::TerrainDemoLayer()
{

}

TerrainDemoLayer::~TerrainDemoLayer()
{

}

void TerrainDemoLayer::OnAttach()
{
	Engine::Application::GetApplication().GetWindow().ToggleMaximize(true);
	Engine::ShaderLibrary::Load("assets/shaders/Terrain/TerrainGenerator.glsl");
	Engine::ShaderLibrary::Load("assets/shaders/Terrain/TerrainLighting.glsl");
	
	m_WhiteTexture = Engine::Texture2D::CreateWhiteTexture();

	Engine::LightSpecification lightSpec =
	{
		Engine::LightType::Point,
		{1.0f, 1.0f, 1.0f},
		1.0f
	};
	m_Light = Engine::CreateRef<Engine::Light>(lightSpec);
	m_Light->GetLightTransform()->SetPosition({ 0.0f, 10.0, 0.0f });

	m_Terrain = Engine::CreateRef<Engine::Terrain>();
}

void TerrainDemoLayer::OnDetach()
{

}

void TerrainDemoLayer::OnUpdate(float deltaTime)
{
	m_Camera.Update(deltaTime);

	Engine::RenderCommand::SetDrawMode(m_Wireframe ? Engine::DrawMode::WireFrame : Engine::DrawMode::Fill);
	Engine::RenderCommand::SetFaceCullMode(Engine::FaceCullMode::Back);
	Engine::RenderCommand::ClearColor({ 0.0f, 0.0f, 0.0f, 0.0f });
	Engine::RenderCommand::Clear(true, true);

	m_WhiteTexture->BindToSamplerSlot(0);
	Engine::ShaderLibrary::Get("TerrainLighting")->Bind();
	Engine::ShaderLibrary::Get("TerrainLighting")->UploadUniformMat4("u_ModelMatrix", m_Terrain->GetTransform()->Transform());
	Engine::ShaderLibrary::Get("TerrainLighting")->UploadUniformMat4("u_MVP", m_Camera.GetViewProjection() * m_Terrain->GetTransform()->Transform());
	Engine::ShaderLibrary::Get("TerrainLighting")->UploadUniformFloat3("u_CameraPosition", m_Camera.GetPosition());
	Engine::ShaderLibrary::Get("TerrainLighting")->UploadUniformFloat3("u_Light.Color", m_Light->GetLightColor());
	Engine::ShaderLibrary::Get("TerrainLighting")->UploadUniformFloat3("u_Light.WorldSpacePosition", m_Light->GetLightTransform()->GetPosition());
	Engine::ShaderLibrary::Get("TerrainLighting")->UploadUniformFloat("u_Light.Intensity", m_Light->GetLightIntensity());
	Engine::ShaderLibrary::Get("TerrainLighting")->UploadUniformInt("u_Texture", 0);
	Engine::ShaderLibrary::Get("TerrainLighting")->UploadUniformFloat3("u_Color", m_Terrain->GetProperties()->Color);
	m_Terrain->Draw();
}

void TerrainDemoLayer::OnEvent(Engine::Event& e)
{
	m_Camera.OnEvent(e);
}

void TerrainDemoLayer::OnImGuiRender()
{
	ImGui::Begin("Noise Settings");
	bool updated = false;

	if (ImGui::TreeNodeEx("Scene Settings"))
	{
		ImGui::Checkbox("Wireframe Mode", &m_Wireframe);
		ImGui::DragFloat3("Light Position", &m_Light->GetLightTransform()->GetPosition().x, 0.1);
		ImGui::DragFloat3("Light Color", &m_Light->GetLightColor().x, 0.01);
		ImGui::DragFloat("Light Intensity", &m_Light->GetLightIntensity(), 0.01);
		ImGui::TreePop();
	}


	if (ImGui::TreeNode("Terrain Settings"))
	{
		ImGui::Text(("Resolution: " + std::to_string(m_Terrain->GetResolution())).c_str());
		ImGui::Text(("LOD: " + std::to_string(m_Terrain->GetLOD())).c_str());

		if (ImGui::DragInt("LOD", &m_LOD, 0.01, m_Terrain->GetMaxLOD(), m_Terrain->GetMinLOD()))
		{
			m_Terrain->SetLOD(m_LOD);
			updated = true;
		}

		ImGui::DragFloat3("Terrain Color", &m_Terrain->GetProperties()->Color.x, 0.01);


		if (ImGui::DragFloat3("Terrain Size", &m_Terrain->GetProperties()->Scale.x, 0.1))
		{
			m_Terrain->GetTransform()->SetScale(m_Terrain->GetProperties()->Scale);
			updated = true;
		}

		if (ImGui::DragFloat3("Terrain Position", &m_Terrain->GetProperties()->Position.x, 0.1))
		{
			m_Terrain->GetTransform()->SetPosition(m_Terrain->GetProperties()->Position);
			updated = true;
		}

		ImGui::TreePop();
	}


	if (ImGui::TreeNodeEx("Noise Settings"))
	{
		if (ImGui::DragInt("Octaves", &m_Terrain->GetProperties()->NoiseSettings->Octaves, 0.01))
			updated = true;
		if (ImGui::DragFloat("Height Scale Factor", &m_Terrain->GetProperties()->HeightScaleFactor, 0.1))
			updated = true;
		if (ImGui::DragFloat("Height Threshold", &m_Terrain->GetProperties()->HeightThreshold, 0.01))
			updated = true;
		if (ImGui::DragFloat("Noise Scale", &m_Terrain->GetProperties()->NoiseSettings->NoiseScale, 0.01))
			updated = true;
		if (ImGui::DragFloat("Lacunarity", &m_Terrain->GetProperties()->NoiseSettings->Lacunarity, 0.01, 1.0f))
			updated = true;
		if (ImGui::DragFloat("Persistence", &m_Terrain->GetProperties()->NoiseSettings->Persistence, 0.01, 0.0f, 1.0f))
			updated = true;
		if (ImGui::DragFloat2("Offsets", &m_Terrain->GetProperties()->NoiseSettings->TextureOffset.x, 0.1f))
			updated = true;

		ImGui::TreePop();
	}

	if (updated)
		m_Terrain->UpdateTerrain();

	ImGui::End();
}

