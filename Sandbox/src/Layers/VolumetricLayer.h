#pragma once

#include "Engine.h"

class VolumetricLayer : public Engine::Layer
{
public:
	VolumetricLayer();
	~VolumetricLayer();

	void OnAttach() override;
	void OnDetach() override;
	void OnUpdate(float deltaTime) override;
	void OnEvent(Engine::Event& e) override;
	void OnImGuiRender() override;

private:
	
	glm::vec3 m_SphereAPosition{ 0.0f, 1.0f, 0.0f };
	float m_SphereARadius = 1.0f;
	int m_DensitySteps = 30;
	int m_LightSteps = 10;

	float m_AtmosphereDensityFalloff = 0.5f;

	glm::vec3 m_AbsorptionCoefficient{ 0.1f, 0.2f, 0.3f };
	glm::vec3 m_ScatteringCoefficient{ 0.5f, 0.3f, 0.1f };

	Engine::Camera m_Camera;
	Engine::Ref<Engine::Texture2D> m_WhiteTexture;
	Engine::Ref<Engine::SimpleEntity> m_FSQ;
	Engine::Ref<Engine::SimpleEntity> m_GroundPlane;
	Engine::Ref<Engine::Framebuffer> m_FBO;

	Engine::Ref<Engine::Light> m_Light;
};