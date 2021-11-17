#pragma once

#include "Engine.h"

class LineLayer : public Engine::Layer
{
public:
	LineLayer();
	~LineLayer();

	void OnAttach() override;
	void OnDetach() override;
	void OnUpdate(float deltaTime) override;
	void OnEvent(Engine::Event& event) override;
	void OnImGuiRender() override;

private:

	void BloomComputePass();

private:
	bool OnKeyPressed(Engine::KeyPressedEvent& keyPressedEvent);
	bool OnMouseButtonReleased(Engine::MouseButtonReleasedEvent& releasedEvent);
	bool OnMouseButtonPressed(Engine::MouseButtonPressedEvent& pressedEvent);
	void OnResize();

private:
	std::vector<Engine::Ref<Engine::Texture2D>> m_BloomComputeTextures;
	Engine::Ref<Engine::Texture2D> m_BloomDirtTexture;
	const uint32_t m_BloomWorkGroupSize = 4;
	bool m_BloomEnabled = true;
	float m_BloomThreshold = 1.0f;
	float m_BloomKnee = 0.1f;
	float UpsampleScale = 1.0f;
	float m_BloomIntensity = 1.0f;
	float m_BloomDirtIntensity = 1.0f;
	float m_Exposure = 1.0f;

private:
	int m_DragID = -1;
	bool m_IsDragging = false;
	bool m_Animate = false;
	bool m_Looped = false;
	bool m_Debug = true;
	bool m_Orthographic = true;

	bool m_MouseAnimation = false;

	std::vector<Engine::LineVertex> m_BaseCurveVertices;

	Engine::Camera m_Camera;
	glm::vec4 m_ClearColor{ 0.0f, 0.0f, 0.0f, 0.0f };

	Engine::Ref<Engine::Framebuffer> m_FBO;
	Engine::Ref<Engine::Texture2D> m_WhiteTexture;
	Engine::Ref<Engine::BezierCurve> m_Curve;
	Engine::Ref<Engine::SimpleEntity> m_FullScreenQuad;

	Engine::Ref<Engine::SimpleEntity> m_Cube;
	Engine::RendererMaterialProperties m_SphereProperties;

	Engine::Ref<Engine::Light> m_Light;

	glm::vec3 m_LineColor{ 0.0f, 3.0f, 0.0f };

	uint32_t m_ViewportWidth;
	uint32_t m_ViewportHeight;
};