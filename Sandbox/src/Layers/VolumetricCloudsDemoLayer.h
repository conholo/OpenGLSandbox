#pragma once

#include "Engine.h"

#include "Utility/Clouds/CloudsSceneRenderPass.h"
#include "Utility/Clouds/MainCloudRenderPass.h"
#include "Utility/Clouds/CloudsCompositePass.h"
#include "Utility/Clouds/CloudsUI.h"

class VolumetricCloudsDemoLayer : public Engine::Layer
{
public:
	VolumetricCloudsDemoLayer();
	~VolumetricCloudsDemoLayer();

	void OnAttach() override;
	void OnDetach() override;
	void OnUpdate(float deltaTime) override;
	void OnEvent(Engine::Event& e) override;
	void OnImGuiRender() override;

private:
	bool Resize(int width, int height);

private:
	Engine::Camera m_Camera;
	Engine::Ref<CloudsSceneRenderPass> m_SceneRenderPass;
	Engine::Ref<MainCloudRenderPass> m_MainCloudRenderPass;
	Engine::Ref<CloudsCompositePass> m_CloudsCompositePass;

	Engine::Ref<CloudsUIData> m_CloudUIData;
	Engine::Ref<MainCloudPassData> m_MainCloudPassData;

	Engine::Ref<Engine::SimpleEntity> m_CompositeQuad;

private:

	bool m_Wireframe = false;
	int m_LOD = 3;

	Engine::Ref<Engine::Terrain> m_Terrain;
	Engine::Ref<CloudsUI> m_CloudsUI;
	Engine::Ref<CloudAnimationSettings> m_CloudAnimationSettings;
	Engine::Ref<CloudSettings> m_CloudSettings;
	Engine::Ref<BaseShapeWorleySettings> m_BaseShapeSettings;
	Engine::Ref<DetailShapeWorleySettings> m_DetailShapeSettings;
	Engine::Ref<WorleyPerlinSettings> m_PerlinSettings;
	Engine::Ref<CurlSettings> m_CurlSettings;
	Engine::Ref<WaterData> m_WaterSettings;
};