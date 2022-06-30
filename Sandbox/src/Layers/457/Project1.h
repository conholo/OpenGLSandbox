#pragma once

#include "Engine.h"
#include "Layers/410/CloudsUtility/CloudDataStructures.h"

class Project1 : public Engine::Layer
{
public:
	Project1();
	~Project1();

	void OnAttach() override;
	void OnDetach() override;
	void OnUpdate(float deltaTime) override;
	void OnEvent(Engine::Event& event) override;
	void OnImGuiRender() override;

private:

	std::vector<Engine::PrimitiveType> m_SelectablePrimitives;
	
	Engine::Ref<WorleyPerlinSettings> m_PerlinSettings;

	Engine::PrimitiveType m_Type = Engine::PrimitiveType::Sphere;
	bool m_Animate = false;
	bool m_UseNoise = false;
	float m_NoiseFrequency = 0.5f;
	float m_NoiseAmplitude = 2.0f;

	float m_AlphaPercent = 0.0f;
	int m_Factor = 2;
	float m_AD = 0.1f;
	float m_BD = 0.1f;
	float m_Tolerance = 0.01f;
	float m_Shininess = 32.0f;
	float m_AmbientStrength = 0.5f;
	float m_SpecularStrength = 1.0f;
	float m_DiffuseStrength = 1.0f;

	glm::vec3 m_BGColor = glm::vec3(0.3f, 0.4f, 0.8f);
	glm::vec3 m_DotColor = glm::vec3(1.0f, 0.5f, 0.0f);
	glm::vec3 m_LightPosition = glm::vec3(0.0f, 10.0f, 0.0f);

	Engine::Ref<Engine::SimpleEntity> m_Entity;
	Engine::Ref<Engine::Texture2D> m_WhiteTexture;
	Engine::Camera m_Camera;
};