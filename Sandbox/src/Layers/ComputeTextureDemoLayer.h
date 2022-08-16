#pragma once

#include <glm/glm.hpp>

#include "Engine.h"

class ComputeTextureDemoLayer : public Engine::Layer
{
public:
	ComputeTextureDemoLayer();
	~ComputeTextureDemoLayer();

	void OnAttach() override;
	void OnDetach() override;
	void OnUpdate(float deltaTime) override;
	void OnEvent(Engine::Event& event) override;

private:
	bool OnKeyPressed(Engine::KeyPressedEvent& event);

private:
	Engine::Camera m_Camera;

	Engine::Ref<Engine::Texture2D> m_TestComputeTexture;

	bool m_CameraOrbitModeActive = true;
	glm::vec3 m_CameraPosition = glm::vec3(0.0f, 0.0f, 2.0f);
	glm::vec4 m_ClearColor = glm::vec4(0.1f);
	float m_CameraYawRotate = 0.0f;

	Engine::Ref<Engine::SimpleEntity> m_Sphere;
	Engine::Ref<Engine::SimpleEntity> m_Cube;
	Engine::Ref<Engine::SimpleEntity> m_Quad;

private:

	float m_CubeRotation = 0.0f;

	float m_MinScale = 1.0;
	float m_MaxScale = 200.0f;
	float m_MinLacunarity = 2.0f;
	float m_MaxLacunarity = 64.0f;

	float m_MinPersistence = 0.1f;
	float m_MaxPersistence = .8f;

	float m_CurrentScale;
	float m_CurrentLacunarity;
	float m_CurrentPersistence;

	float m_Speed = 2.0f;
	float m_Duration = 2.0f;
	glm::vec2 m_Offset = { 0.0f, 0.0f };
	glm::vec4 m_NoiseColor = glm::vec4(0.5, 0.3, 0.8, 1.0);
};