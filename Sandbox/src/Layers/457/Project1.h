#pragma once

#include "Engine.h"

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

	Engine::PrimitiveType m_Type = Engine::PrimitiveType::Sphere;
	bool m_Animate = false;
	float m_AD = 0.1f;
	float m_BD = 0.1f;
	float m_Tolerance = 0.01f;
	float m_Shininess = 32.0f;
	float m_AmbientStrength = 0.5f;

	glm::vec3 m_BGColor = glm::vec3(0.3f, 0.4f, 0.8f);
	glm::vec3 m_DotColor = glm::vec3(1.0f, 0.5f, 0.0f);
	glm::vec3 m_LightPosition = glm::vec3(0.0f, 10.0f, 0.0f);

	Engine::Ref<Engine::SimpleEntity> m_Entity;
	Engine::Ref<Engine::Texture2D> m_WhiteTexture;
	Engine::Camera m_Camera;
};