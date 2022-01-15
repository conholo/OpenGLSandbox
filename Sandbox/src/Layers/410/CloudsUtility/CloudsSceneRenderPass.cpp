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

	m_Terrain = Engine::CreateRef<Engine::Terrain>();

	m_Terrain->GetProperties()->Scale = glm::vec3(18.0, 22.0f, 18.0f);
	m_Terrain->GetProperties()->Position = glm::vec3(0.0f, -70.0f, 0.0f);
	m_Terrain->GetProperties()->Color = glm::vec3(0.9f, 0.78f, 0.75f);
	m_Terrain->GetProperties()->HeightScaleFactor = 50.0f;
	m_Terrain->GetProperties()->HeightThreshold = 0.11f;
	m_Terrain->GetProperties()->NoiseSettings->Octaves = 8;
	m_Terrain->GetProperties()->NoiseSettings->Persistence = 0.26f;
	m_Terrain->GetProperties()->NoiseSettings->Lacunarity = 2.8f;
	m_Terrain->GetProperties()->NoiseSettings->NoiseScale = 6.0f;
	m_Terrain->SetLOD(0);
	m_Terrain->UpdateTerrain();

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

	Engine::RenderCommand::SetFaceCullMode(Engine::FaceCullMode::Front);
	Engine::RenderCommand::SetDrawMode(m_TerrainIsWireframe ? Engine::DrawMode::WireFrame : Engine::DrawMode::Fill);
	Engine::ShaderLibrary::Get("TerrainLighting")->Bind();
	m_WhiteTexture->BindToSamplerSlot(0);
	Engine::ShaderLibrary::Get("TerrainLighting")->UploadUniformMat4("u_ModelMatrix", m_Terrain->GetTransform()->Transform());
	Engine::ShaderLibrary::Get("TerrainLighting")->UploadUniformMat4("u_MVP", camera.GetViewProjection() * m_Terrain->GetTransform()->Transform());
	Engine::ShaderLibrary::Get("TerrainLighting")->UploadUniformFloat3("u_CameraPosition", camera.GetPosition());
	Engine::ShaderLibrary::Get("TerrainLighting")->UploadUniformFloat3("u_Light.Color", m_Sun->GetLightColor());
	Engine::ShaderLibrary::Get("TerrainLighting")->UploadUniformFloat3("u_Light.WorldSpacePosition", m_Sun->GetLightTransform()->GetPosition());
	Engine::ShaderLibrary::Get("TerrainLighting")->UploadUniformFloat("u_Light.Intensity", m_Sun->GetLightIntensity());
	Engine::ShaderLibrary::Get("TerrainLighting")->UploadUniformInt("u_Texture", 0);
	Engine::ShaderLibrary::Get("TerrainLighting")->UploadUniformFloat3("u_Color", m_Terrain->GetProperties()->Color);
	m_Terrain->Draw();
	Engine::RenderCommand::SetFaceCullMode(Engine::FaceCullMode::None);
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
