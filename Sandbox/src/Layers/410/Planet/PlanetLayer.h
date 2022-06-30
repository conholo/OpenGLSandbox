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
	
	int m_DensitySteps = 50;
	int m_LightSteps = 4;
	int m_AtmosphericOpticalDepthPoints = 3;

	float m_SunSizeAttenuation = 1.0f;
	bool m_AnimateClouds = true;
	float m_AnimationSpeed = 1.6f;
	float m_TimeScale = 0.008f;
	float m_CloudScale = 90.0f;
	float m_LowerCloudOffsetPct = .82f;
	float m_UpperCloudOffsetPct = .62f;
	float m_BaseShapeErosionFactor = 0.060f;

	float m_ExtinctionFactor = 0.2f;
	float m_ForwardScatteringConstant = .95f;

	float m_ViewerAttenuationFactor = 4.00f;
	float m_DensityMultiplier = 3.0f;
	float m_LuminanceMultiplier = 1.0f;
	float m_AtmosphereStrength = 2.5f;
	//glm::vec3 m_WaveLengths{ 700.0f, 530.0f, 440.0f };
	glm::vec3 m_WaveLengths{ 1050.0f, 728.0f, 506.0f };
	float m_ScatteringStrength = 1.560f;
	float m_AtmosphereDensityFalloff = 5.5f;
	float m_DensityThreshold = .25f;
	float m_SilverLiningConstant = 1.0f;
	float m_PowderConstant = 1.0f;
	float m_TypeWeightMultiplier = 1.0f;
	glm::vec4 m_ShapeNoiseWeights{ 1.7f, 7.5f, 6.5f, 10.0f};
	glm::vec3 m_DetailNoiseWeights{ .825f, .21f, .055f};
	glm::vec3 m_CloudTypeWeights{ 1.05f, .74f, 3.9f};

	Engine::Ref<CloudAnimationSettings> m_CloudAnimationSettings;
	Engine::Ref<CloudSettings> m_CloudSettings;
	Engine::Ref<BaseShapeWorleySettings> m_BaseShapeSettings;
	Engine::Ref<DetailShapeWorleySettings> m_DetailShapeSettings;
	Engine::Ref<WorleyPerlinSettings> m_PerlinSettings;
	Engine::Ref<CurlSettings> m_CurlSettings;

	glm::vec3 m_AtmosphereColor{ 0.0f, 0.0f, 1.0f };
	glm::vec3 m_CloudTintColor{ 1.0f, 1.0f, 1.0f };

	Engine::Ref<Engine::SimpleEntity> m_FSQ;
	Engine::Ref<Engine::Framebuffer> m_FBO;
	bool m_Wireframe = false;
	int m_MeshResolution = 6;
	Engine::Ref<Planet> m_Planet;
	Engine::Ref<Engine::Light> m_Light;
	Engine::Camera m_Camera;
};