#pragma once

#include "Layers/410/CloudsUtility/MainCloudRenderPass.h"

class CloudsCompositePass
{
public:
	CloudsCompositePass();
	~CloudsCompositePass();

	void ExecutePass(const Engine::Camera& camera, const Engine::Ref<MainCloudRenderPass>& mainRenderPass);
private:
	glm::vec4 m_ClearColor{ 0.1f, 0.1f, 0.1f, 0.1f };
	Engine::Ref<Engine::SimpleEntity> m_CompositeQuad;
};