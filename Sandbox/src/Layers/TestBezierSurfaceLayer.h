#pragma once

#include "Engine.h"

struct LightingProperties
{
	glm::vec3 AmbientColor = glm::vec3(1.0f, 1.0f, 1.0f);
	glm::vec3 DiffuseColor = glm::vec3(1.0f, 1.0f, 1.0f);
	float AmbientStrength = 1.0f;
	float DiffuseStrength = 1.0f;
	float SpecularStrength = 1.0f;
	float Shininess = 1.0f;
};

class TestBezierSurfaceLayer : public Engine::Layer
{
public:
	TestBezierSurfaceLayer();
	~TestBezierSurfaceLayer();

	void OnAttach() override;
	void OnDetach() override;
	void OnUpdate(float deltaTime) override;
	void OnEvent(Engine::Event& event) override;
	void OnImGuiRender() override;

private:
	bool OnMouseButtonPressed(Engine::MouseButtonPressedEvent& pressedEvent);
	bool OnMouseButtonReleased(Engine::MouseButtonReleasedEvent& releasedEvent);
	bool OnKeyPressed(Engine::KeyPressedEvent& pressedEvent);
private:
	LightingProperties m_Properties =
	{
		{0.6f, 0.6f, 0.6f},
		{1.0f, 1.0f, 1.0f},
		0.5f,
		0.7f,
		0.1f,
		2.0f
	};
	Engine::Ref<Engine::Texture2D> m_Texture;
	Engine::Ref<Engine::SimpleEntity> m_Cube;
	Engine::Ref<Engine::BezierSurface> m_Surface;
	Engine::Ref<Engine::Light> m_Light;
	Engine::Camera m_Camera;
	glm::vec4 m_ClearColor{ 0.1f, 0.1f, 0.1f, 0.1f };
};