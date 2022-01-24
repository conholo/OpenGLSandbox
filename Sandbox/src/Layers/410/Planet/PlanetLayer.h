#pragma once

#include "Engine.h"

#include "Layers/410/Planet/Planet.h"
#include "Layers/410/CloudsUtility/CloudDataStructures.h"

class PlanetLayer : public Engine::Layer
{
public:
	PlanetLayer();
	~PlanetLayer();

	void OnAttach() override;
	void OnDetach() override;
	void OnUpdate(float deltaTime) override;
	void OnEvent(Engine::Event& e) override;
	void OnImGuiRender() override;

private:
	
	int m_DensitySteps = 35;
	int m_LightSteps = 1;

	bool m_AnimateClouds = false;
	float m_AnimationSpeed = 5.0f;
	float m_TimeScale = 0.001f;
	float m_CloudScale = 150.0f;
	float m_LowerCloudOffsetPct = 0.15f;
	float m_UpperCloudOffsetPct = 0.0f;

	float m_ViewerAttenuationFactor = 4.00f;
	float m_DensityMultiplier = 4.75f;
	float m_LuminanceMultiplier = 15.0f;
	float m_AtmosphereStrength = 1.0f;
	float m_DensityThreshold = 0.075f;
	glm::vec4 m_ShapeNoiseWeights{ 0.1f, 0.1f, 0.1f, 0.1f };

	Engine::Ref<WorleyPerlinSettings> m_PerlinSettings;
	Engine::Ref<BaseShapeWorleySettings> m_ShapeSettings;

	Engine::Ref<Engine::SimpleEntity> m_FSQ;
	Engine::Ref<Engine::Framebuffer> m_FBO;
	bool m_Wireframe = false;
	int m_MeshResolution = 6;
	Engine::Ref<Planet> m_Planet;
	Engine::Ref<Engine::Light> m_Light;
	Engine::Camera m_Camera;
};