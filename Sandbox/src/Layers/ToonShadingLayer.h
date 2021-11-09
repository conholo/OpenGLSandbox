#pragma once

#include "Engine.h"

class ToonShadingLayer : public Engine::Layer
{
public:
	ToonShadingLayer();
	~ToonShadingLayer();

	void OnAttach() override;
	void OnDetach() override;
	void OnUpdate(float deltaTime) override;
	void OnEvent(Engine::Event& event) override;

private:
	bool OnKeyPressed(Engine::KeyPressedEvent& keyPressedEvent);

private:
	Engine::Camera m_Camera;
	glm::vec4 m_ClearColor{ 0.1f, 0.1f, 0.1f, 0.1f };

private:
	float m_Counter;
	Engine::Ref<Engine::SimpleEntity> m_Entity;
	Engine::Ref<Engine::SimpleEntity> m_Plane;
	Engine::Ref<Engine::Light> m_PointLight;
	glm::vec3 m_LightOrigin{ 2.0f, 1.0f, 4.0f };

	Engine::Ref<Engine::Texture2D> m_WhiteTexture;
};