#include "VolumetricCloudsDemoLayer.h"

VolumetricCloudsDemoLayer::VolumetricCloudsDemoLayer()
{
	Engine::Application::GetApplication().GetWindow().ToggleMaximize(true);
	Engine::ShaderLibrary::Load("assets/shaders/Clouds/WorleyGenerator.glsl");
	Engine::ShaderLibrary::Load("assets/shaders/Clouds/NormalizeWorley.glsl");
	Engine::ShaderLibrary::Load("assets/shaders/Clouds/3DTextureViewer.glsl");
	Engine::ShaderLibrary::Load("assets/shaders/Clouds/Perlin2D.glsl");
	Engine::ShaderLibrary::Load("assets/shaders/Clouds/Clouds.glsl");
	Engine::ShaderLibrary::Load("assets/shaders/Clouds/CloudsComposite.glsl");
	Engine::ShaderLibrary::Load("assets/shaders/Clouds/TextureDisplay.glsl");
	Engine::ShaderLibrary::Load("assets/shaders/Clouds/Curl.glsl");
	Engine::ShaderLibrary::Load("assets/shaders/Terrain/TerrainGenerator.glsl");
	Engine::ShaderLibrary::Load("assets/shaders/Terrain/TerrainLighting.glsl");
}

VolumetricCloudsDemoLayer::~VolumetricCloudsDemoLayer()
{

}

void VolumetricCloudsDemoLayer::OnAttach()
{
	m_Camera.SetPosition({ 65.0f, 65.0f, 452.0f });

	m_CloudsUI = Engine::CreateRef<CloudsUI>();
	m_CloudSettings = Engine::CreateRef<CloudSettings>();
	m_CloudAnimationSettings = Engine::CreateRef<CloudAnimationSettings>();

	m_PerlinSettings = Engine::CreateRef<WorleyPerlinSettings>();
	m_CurlSettings = Engine::CreateRef<CurlSettings>();
	m_WaterSettings = Engine::CreateRef<WaterData>();
	m_BaseShapeSettings = Engine::CreateRef<BaseShapeWorleySettings>();
	m_BaseShapeSettings->UpdateAllChannels(m_PerlinSettings);
	m_DetailShapeSettings = Engine::CreateRef<DetailShapeWorleySettings>();
	m_DetailShapeSettings->UpdateAllChannels();

	m_SceneRenderPass = Engine::CreateRef<CloudsSceneRenderPass>();
	m_MainCloudRenderPass = Engine::CreateRef<MainCloudRenderPass>();
	m_CloudsCompositePass = Engine::CreateRef<CloudsCompositePass>();

	m_CloudUIData = Engine::CreateRef<CloudsUIData>();
	m_CloudUIData->AnimationSettings = m_CloudAnimationSettings;
	m_CloudUIData->MainCloudSettings = m_CloudSettings;
	m_CloudUIData->BaseShapeSettings = m_BaseShapeSettings;
	m_CloudUIData->DetailShapeSettings = m_DetailShapeSettings;
	m_CloudUIData->PerlinSettings = m_PerlinSettings;
	m_CloudUIData->CloudCurlSettings = m_CurlSettings;
	m_CloudUIData->SceneRenderPass = m_SceneRenderPass;
	m_CloudUIData->WaterSettings = m_WaterSettings;

	m_MainCloudPassData = Engine::CreateRef<MainCloudPassData>();
	m_MainCloudPassData->AnimationSettings = m_CloudAnimationSettings;
	m_MainCloudPassData->MainSettings = m_CloudSettings;
	m_MainCloudPassData->MainCloudCurlSettings = m_CurlSettings;
	m_MainCloudPassData->BaseShapeSettings = m_BaseShapeSettings;
	m_MainCloudPassData->DetailShapeSettings = m_DetailShapeSettings;
	m_MainCloudPassData->PerlinSettings = m_PerlinSettings;
	m_MainCloudPassData->SceneRenderPass = m_SceneRenderPass;
	m_MainCloudPassData->UI = m_CloudsUI;
	m_MainCloudPassData->MainWaterData = m_WaterSettings;

	//Resize(Engine::Application::GetApplication().GetWindow().GetWidth(), Engine::Application::GetApplication().GetWindow().GetHeight());
}

void VolumetricCloudsDemoLayer::OnDetach()
{

}

void VolumetricCloudsDemoLayer::OnUpdate(float deltaTime)
{
	m_Camera.Update(deltaTime);
	m_SceneRenderPass->ExecutePass(m_Camera);
	m_MainCloudRenderPass->ExecutePass(m_Camera, m_MainCloudPassData);
	m_CloudsCompositePass->ExecutePass(m_Camera, m_MainCloudRenderPass);
}

void VolumetricCloudsDemoLayer::OnEvent(Engine::Event& e)
{
	m_Camera.OnEvent(e);
}

void VolumetricCloudsDemoLayer::OnImGuiRender()
{
	m_CloudsUI->Draw(m_CloudUIData);
}

bool VolumetricCloudsDemoLayer::Resize(int width, int height)
{
	m_SceneRenderPass->Resize(width, height);
	m_MainCloudRenderPass->Resize(width, height);

	return false;
}
