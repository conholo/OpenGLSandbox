#include "Layers/410/CloudsLayer.h"
#include "Layers/410/CloudsUtility/HelperFunctions.h"
#include <imgui/imgui.h>

CloudsLayer::CloudsLayer()
{
	Engine::Application::GetApplication().GetWindow().ToggleMaximize(true);
	Engine::ShaderLibrary::Load("assets/410 shaders/WorleyGenerator.shader");
	Engine::ShaderLibrary::Load("assets/410 shaders/3DTextureViewer.shader");
	Engine::ShaderLibrary::Load("assets/410 shaders/Perlin2D.shader");
	Engine::ShaderLibrary::Load("assets/410 shaders/Clouds.shader");
	Engine::ShaderLibrary::Load("assets/410 shaders/CloudsComposite.shader");
	Engine::ShaderLibrary::Load("assets/410 shaders/TextureDisplay.shader");
	Engine::ShaderLibrary::Load("assets/410 shaders/Curl.shader");

	Engine::ShaderLibrary::Load("assets/410 shaders/TerrainGenerator.shader");
	Engine::ShaderLibrary::Load("assets/410 shaders/TerrainLighting.shader");

}

CloudsLayer::~CloudsLayer()
{

}

void CloudsLayer::OnAttach()
{
	m_Camera.SetPosition({ 65.0f, 65.0f, 452.0f });

	m_CloudsUI = Engine::CreateRef<CloudsUI>();
	m_CloudSettings = Engine::CreateRef<CloudSettings>();
	m_CloudAnimationSettings = Engine::CreateRef<CloudAnimationSettings>();

	m_PerlinSettings = Engine::CreateRef<WorleyPerlinSettings>();
	m_CurlSettings = Engine::CreateRef<CurlSettings>();
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
	m_CloudUIData->CurlSettings = m_CurlSettings;
	m_CloudUIData->SceneRenderPass = m_SceneRenderPass;

	m_MainCloudPassData = Engine::CreateRef<MainCloudPassData>();
	m_MainCloudPassData->AnimationSettings = m_CloudAnimationSettings;
	m_MainCloudPassData->MainSettings = m_CloudSettings;
	m_MainCloudPassData->CurlSettings = m_CurlSettings;
	m_MainCloudPassData->BaseShapeSettings = m_BaseShapeSettings;
	m_MainCloudPassData->DetailShapeSettings = m_DetailShapeSettings;
	m_MainCloudPassData->PerlinSettings = m_PerlinSettings;
	m_MainCloudPassData->SceneRenderPass = m_SceneRenderPass;
	m_MainCloudPassData->UI = m_CloudsUI;

	Resize(Engine::Application::GetApplication().GetWindow().GetWidth(), Engine::Application::GetApplication().GetWindow().GetHeight());
}

void CloudsLayer::OnDetach()
{

}

void CloudsLayer::OnUpdate(float deltaTime)
{
	m_Camera.Update(deltaTime);
	m_SceneRenderPass->ExecutePass(m_Camera);
	m_MainCloudRenderPass->ExecutePass(m_Camera, m_MainCloudPassData);
	m_CloudsCompositePass->ExecutePass(m_Camera, m_MainCloudRenderPass);
}

void CloudsLayer::OnEvent(Engine::Event& e)
{
	m_Camera.OnEvent(e);
}

void CloudsLayer::OnImGuiRender()
{
	m_CloudsUI->Draw(m_CloudUIData);
}

bool CloudsLayer::Resize(int width, int height)
{
	m_SceneRenderPass->Resize(width, height);
	m_MainCloudRenderPass->Resize(width, height);

	return false;
}
