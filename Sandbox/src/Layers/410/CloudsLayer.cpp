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
}

CloudsLayer::~CloudsLayer()
{

}

void CloudsLayer::OnAttach()
{
	m_CloudsUI = Engine::CreateRef<CloudsUI>();
	m_CloudSettings = Engine::CreateRef<CloudSettings>();
	m_CloudAnimationSettings = Engine::CreateRef<CloudAnimationSettings>();

	m_PerlinSettings = Engine::CreateRef<WorleyPerlinSettings>();
	m_BaseShapeSettings = Engine::CreateRef<BaseShapeWorleySettings>();
	m_BaseShapeSettings->UpdateAllChannels(m_PerlinSettings);

	m_SceneRenderPass = Engine::CreateRef<CloudsSceneRenderPass>();
	m_MainCloudRenderPass = Engine::CreateRef<MainCloudRenderPass>();
	m_CloudsCompositePass = Engine::CreateRef<CloudsCompositePass>();

	m_MainCloudPassData = Engine::CreateRef<MainCloudPassData>();
	m_MainCloudPassData->AnimationSettings = m_CloudAnimationSettings;
	m_MainCloudPassData->MainSettings = m_CloudSettings;
	m_MainCloudPassData->BaseShapeSettings = m_BaseShapeSettings;
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
	m_CloudsUI->Draw(m_BaseShapeSettings, m_PerlinSettings);
}

bool CloudsLayer::Resize(int width, int height)
{
	m_SceneRenderPass->Resize(width, height);
	m_MainCloudRenderPass->Resize(width, height);

	return false;
}
