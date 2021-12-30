#pragma once

#include "Engine.h"
#include <glm/glm.hpp>


struct Particle
{
	glm::vec3 Position;
	glm::vec3 Velocity;
	glm::vec3 Color;
};

class ComputeTestLayer : public Engine::Layer
{
public:
	ComputeTestLayer();
	~ComputeTestLayer();

	void OnAttach() override;
	void OnDetach() override;
	void OnUpdate(float deltaTime) override;
	void OnEvent(Engine::Event& event) override;
	void OnImGuiRender() override;

private:
	void Simple2DTextureTest();

	void Resize(uint32_t width, uint32_t height);
	bool OnKeyPressed(Engine::KeyPressedEvent& keyPressedEvent);

private:
	float m_Counter = 0.0;
	float m_SwitchTime = 1.0;
	int m_ValueX = 0;
	int m_ValueY = 0;
	uint32_t m_ViewportWidth, m_ViewportHeight;
	std::vector<int> m_Values;

	Engine::Ref<Engine::SimpleEntity> m_DisplayQuad;
	Engine::Ref<Engine::SimpleEntity> m_Plane;

	Engine::Ref<Engine::Texture2D> m_WhiteTexture;
	Engine::Camera m_Camera;

private:
	Engine::Ref<Engine::Texture2D> m_ComputeTexture;
	Engine::Ref<Engine::Texture2D> m_FromImageTexture;
};