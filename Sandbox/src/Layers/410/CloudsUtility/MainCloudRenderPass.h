#pragma once

#include "Layers/410/CloudsUtility/CloudsSceneRenderPass.h"
#include "Layers/410/CloudsUtility/CloudsUI.h"

struct MainCloudPassData
{
	Engine::Ref<CloudSettings> MainSettings;
	Engine::Ref<CloudAnimationSettings> AnimationSettings;
	Engine::Ref<CloudsSceneRenderPass> SceneRenderPass;
	Engine::Ref<BaseShapeWorleySettings> BaseShapeSettings;
	Engine::Ref<WorleyPerlinSettings> PerlinSettings;
	Engine::Ref<CloudsUI> UI;
};

class MainCloudRenderPass
{
public:
	MainCloudRenderPass();
	~MainCloudRenderPass();

	void ExecutePass(const Engine::Camera& camera, const Engine::Ref<MainCloudPassData>& passData);
	void BindMainColorAttachment(uint32_t slot) const { return m_CloudFBO->BindColorAttachment(slot); }
	void Resize(uint32_t width, uint32_t height);

private:
	glm::vec4 m_ClearColor{ 0.1f, 0.1f, 0.1f, 0.1f };

	Engine::Ref<Engine::Framebuffer> m_CloudFBO;
	Engine::Ref<Engine::SimpleEntity> m_CloudQuad;
};