#pragma once

#include "Engine.h"
#include "Layers/Utility/Prague/PragueDriver.h"

class PragueSkyDemoLayer : public Engine::Layer
{
public:
	PragueSkyDemoLayer();
	~PragueSkyDemoLayer() override;

	void OnAttach() override;
	void OnDetach() override;
	void OnUpdate(float deltaTime) override;
	void OnEvent(Engine::Event& e) override;
	void OnImGuiRender() override;

private:
	void InitializeViewParameters();
	void LoadEntities();
	void GenerateGlobalRenderResources();
	void InitializeEnvironmentAndAtmosphereResources();

private:
	void TickAtmosphereParamsUpdate(float deltaTime);

private:
	Engine::Camera m_Camera;
	Engine::Ref<Engine::Light> m_Light;

	Engine::Ref<Engine::Framebuffer> m_ScenePBRFBO;
	Engine::Ref<Engine::Framebuffer> m_SkyRadianceFBO;
	Engine::Ref<Engine::Framebuffer> m_DepthFBO;

	Engine::Ref<Engine::SimpleEntity> m_CompositeFSQ;
	Engine::Ref<Engine::SimpleEntity> m_RadianceFSQ;
	Engine::Ref<Engine::SimpleEntity> m_DepthVisualizerFSQ;

	Engine::Ref<Engine::CubeMap> m_Skybox;

	std::vector<Engine::Ref<Engine::SimpleEntity>> m_Towers;

	// Realtime Params
	bool m_AnimateAltitude = false;
	bool m_GridEnabled = true;
	bool m_LockCameraToAltitudeAndTarget = false;
	bool m_RequestCameraReset = false;
	float m_PreviousAltitude;
	float m_AnimationSpeed = 100.0f;
	glm::vec3 m_EnvironmentMapSampleLODs {0.0f };
	glm::vec3 m_EnvironmentMapSampleIntensities {1.0f };
	// Realtime Params

	// Shader/Uniform Params
	bool m_ApplyTransmittance = true;
	bool m_UpdateEnvironmentMaps = true;
	float m_Exposure = 0.0f;
	// Shader/Uniform Params

	//Engine::Ref<Engine::EnvironmentMapPipeline> m_EnvironmentPipeline;
	Engine::Ref<PragueDriver> m_PragueDriver;
	uint32_t m_ViewportWidth, m_ViewportHeight;
};
