#pragma once

#include "Engine.h"
#include <glm/glm.hpp>

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
	void Resize(uint32_t width, uint32_t height);
	bool OnKeyPressed(Engine::KeyPressedEvent& keyPressedEvent);

private:
	float m_Counter;

	Engine::Ref<Engine::Framebuffer> m_FBO;
	uint32_t m_ViewportWidth, m_ViewportHeight;

	Engine::Ref<Engine::SimpleEntity> m_FullScreenQuad;
	Engine::Ref<Engine::SimpleEntity> m_Plane;

	Engine::Ref<Engine::Texture2D> m_WhiteTexture;
	Engine::Camera m_Camera;

private:
	uint32_t m_CurrentMip = 1;
	uint32_t m_WorkGroupSize = 4;
	Engine::Ref<Engine::Texture2D> m_TextureA;
	Engine::Ref<Engine::Texture2D> m_FromImageTexture;

	uint32_t m_ImageViewID;
};