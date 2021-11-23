#pragma once

#include "Engine.h"

class FractalLayer : public Engine::Layer
{
public:
	FractalLayer();
	~FractalLayer();

	void OnAttach() override;
	void OnDetach() override;
	void OnUpdate(float deltaTime) override;
	void OnEvent(Engine::Event& event) override;
	void OnImGuiRender() override;

private:
	bool OnKeyPressed(Engine::KeyPressedEvent& keyPressedEvent);

private:
	bool m_Up = true;
	float m_MengerMaxScale = 5.0;
	float m_MengerCounter = 0.0f;
	float m_MengerScale = 1.0f;
	float m_ScaleSpeed = 2.0f;
	float m_Counter = 0.0f;
	glm::vec3 m_LightPosition = glm::vec3(5.0f, 15.0f, 10.0f);
	Engine::Ref<Engine::Texture2D> m_WhiteTexture;
	Engine::Ref<Engine::SimpleEntity> m_Entity;
	Engine::Camera m_Camera;
	glm::vec4 m_ClearColor{ 0.1f, 0.1f, 0.1f, 0.1f };
};