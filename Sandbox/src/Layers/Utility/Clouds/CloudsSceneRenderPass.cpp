#include "CloudsSceneRenderPass.h"

#include "Engine/Core/Application.h"
#include "Engine/Rendering/RenderCommand.h"

static Engine::Ref<Engine::TerrainHeightLayer> CreateHeightLayer(const std::string& filePath, float heightThreshold, float tiling, float blendStrength)
{
	Engine::Ref<Engine::TerrainHeightLayer> layer = Engine::CreateRef<Engine::TerrainHeightLayer>();
	Engine::Texture2DSpecification textureArraySpec =
	{
		Engine::ImageUtils::WrapMode::Repeat,
		Engine::ImageUtils::WrapMode::Repeat,
		Engine::ImageUtils::FilterMode::Linear,
		Engine::ImageUtils::FilterMode::Linear,
		Engine::ImageUtils::ImageInternalFormat::FromImage,
		Engine::ImageUtils::ImageDataLayout::FromImage,
		Engine::ImageUtils::ImageDataType::UByte,
	};

	layer->HeightTexture = Engine::CreateRef<Engine::Texture2D>(filePath, textureArraySpec);
	layer->BlendStrength = blendStrength;
	layer->TextureTiling = tiling;
	layer->HeightThreshold = heightThreshold;
	return layer;
}

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
	m_Sun->GetLightTransform()->SetPosition({ 30.0f, 600.0f, 0.0f });

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

	m_Terrain->SetLOD(0);
	m_Terrain->UpdateTerrain();

	m_FBO = Engine::CreateRef<Engine::Framebuffer>(fboSpec);

	m_HeightLayers.resize(m_HeightLayerCount);
	m_HeightLayers[0] = CreateHeightLayer("assets/textures/Height Textures/Water.png", -0.15f, 15.0f, 0.4);
	m_HeightLayers[1] = CreateHeightLayer("assets/textures/Height Textures/Sandy grass.png", 0.13f, 20.0f, 0.32f);
	m_HeightLayers[2] = CreateHeightLayer("assets/textures/Height Textures/Grass.png", 0.28f, 6.30f, 0.29f);
	m_HeightLayers[3] = CreateHeightLayer("assets/textures/Height Textures/Stony ground.png", 0.59f, 6.0f, 0.5f);
	m_HeightLayers[4] = CreateHeightLayer("assets/textures/Height Textures/Rocks 1.png", 0.69f, 9.2f, 0.5f);
	m_HeightLayers[5] = CreateHeightLayer("assets/textures/Height Textures/Rocks 2.png", 0.78f, 5.7f, 0.5f);
	m_HeightLayers[6] = CreateHeightLayer("assets/textures/Height Textures/Snow.png", 0.85f, 12.0f, 0.5f);
}

CloudsSceneRenderPass::~CloudsSceneRenderPass()
{

}

void CloudsSceneRenderPass::DrawSceneEntities(const Engine::Camera& camera)
{
	//m_WhiteTexture->BindToSamplerSlot(0);
	//m_GroundPlane->GetEntityRenderer()->GetMaterial().GetShader()->Bind();
	//m_GroundPlane->GetEntityRenderer()->GetMaterial().GetShader()->UploadUniformFloat3("u_Color", { 1.0f, 0.8f, 0.8f });
	//m_GroundPlane->DrawEntity(camera.GetViewProjection());


	if (!m_DrawTerrain) return;

	Engine::RenderCommand::SetDrawMode(m_TerrainIsWireframe ? Engine::DrawMode::WireFrame : Engine::DrawMode::Fill);
	Engine::ShaderLibrary::Get("TerrainLighting")->Bind();
	Engine::ShaderLibrary::Get("TerrainLighting")->UploadUniformMat4("u_ModelMatrix", m_Terrain->GetTransform()->Transform());
	Engine::ShaderLibrary::Get("TerrainLighting")->UploadUniformMat4("u_MVP", camera.GetViewProjection() * m_Terrain->GetTransform()->Transform());
	Engine::ShaderLibrary::Get("TerrainLighting")->UploadUniformFloat3("u_CameraPosition", camera.GetPosition());
	Engine::ShaderLibrary::Get("TerrainLighting")->UploadUniformFloat3("u_Light.Color", m_Sun->GetLightColor());
	Engine::ShaderLibrary::Get("TerrainLighting")->UploadUniformFloat3("u_Light.WorldSpacePosition", m_Sun->GetLightTransform()->GetPosition());
	Engine::ShaderLibrary::Get("TerrainLighting")->UploadUniformFloat("u_Light.Intensity", m_Sun->GetLightIntensity());
	for (uint32_t i = 0; i < m_HeightLayers.size(); i++)
	{
		std::string heightTilingName = "u_TextureTiling[" + std::to_string(i) + "]";
		std::string heightThresholdName = "u_HeightThresholds[" + std::to_string(i) + "]";
		std::string blendStrengthName = "u_Blends[" + std::to_string(i) + "]";
		std::string heightTextureName = "u_HeightTextures[" + std::to_string(i) + "]";
		std::string heightTintColors = "u_TintColors[" + std::to_string(i) + "]";
		m_HeightLayers[i]->HeightTexture->BindToSamplerSlot(i);

		Engine::ShaderLibrary::Get("TerrainLighting")->UploadUniformInt(heightTextureName, i);
		Engine::ShaderLibrary::Get("TerrainLighting")->UploadUniformFloat(heightTilingName, m_HeightLayers[i]->TextureTiling);
		Engine::ShaderLibrary::Get("TerrainLighting")->UploadUniformFloat(heightThresholdName, m_HeightLayers[i]->HeightThreshold);
		Engine::ShaderLibrary::Get("TerrainLighting")->UploadUniformFloat(blendStrengthName, m_HeightLayers[i]->BlendStrength);
		Engine::ShaderLibrary::Get("TerrainLighting")->UploadUniformFloat3(heightTintColors, m_HeightLayers[i]->TintColor);
	}
	Engine::ShaderLibrary::Get("TerrainLighting")->UploadUniformInt("u_LayerCount", m_HeightLayerCount);
	Engine::ShaderLibrary::Get("TerrainLighting")->UploadUniformFloat("u_MinHeight", m_Terrain->GetMinHeightLocalSpace());
	Engine::ShaderLibrary::Get("TerrainLighting")->UploadUniformFloat("u_MaxHeight", m_Terrain->GetMaxHeightLocalSpace());
	Engine::ShaderLibrary::Get("TerrainLighting")->UploadUniformFloat3("u_Color", m_Terrain->GetProperties()->Color);
	m_Terrain->Draw();
	Engine::RenderCommand::SetDrawMode(Engine::DrawMode::Fill);
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
