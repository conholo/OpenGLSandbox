#pragma once

#include "Engine.h"

class CurlTestLayer : public Engine::Layer
{
public:
	CurlTestLayer();
	~CurlTestLayer();

	void OnAttach() override;
	void OnDetach() override;
	void OnUpdate(float deltaTime) override;
	void OnEvent(Engine::Event& e) override;
	void OnImGuiRender() override;

private:
	float m_Strength = 0.1f;
	float m_Tiling = 1.0f;
	float m_Persistence = 0.5f;
	float m_TilingSpeed = 0.001f;
	glm::vec2 m_TilingOffset{ 50.0f, 50.0f };
	glm::vec2 m_Weights{ 0.5f, 0.5f };

private:
	Engine::Ref<Engine::Texture2D> m_CurlTexture;
	Engine::Ref<Engine::SimpleEntity> m_Quad;
	Engine::Camera m_Camera;
};