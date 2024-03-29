#pragma once

#include "Engine.h"

class BloomDemoLayer : public Engine::Layer
{
public:
	BloomDemoLayer();
	~BloomDemoLayer();

	void OnAttach() override;
	void OnDetach() override;
	void OnUpdate(float deltaTime) override;
	void OnEvent(Engine::Event& event) override;
	void OnImGuiRender() override;

private:
	void CheckForResize();
	void BloomComputePass();
	void PollCurvePicking();
	void AnimateChopsticks();

private:
	bool OnKeyPressed(Engine::KeyPressedEvent& keyPressedEvent);
	bool OnMouseButtonReleased(Engine::MouseButtonReleasedEvent& releasedEvent);
	bool OnMouseButtonPressed(Engine::MouseButtonPressedEvent& pressedEvent);
	void OnResize();

private:
	Engine::Ref<Engine::Texture2DImageView> m_BloomView;
	int32_t m_ViewMipLevel = 0;

	std::vector<Engine::Ref<Engine::Texture2D>> m_BloomComputeTextures;
	Engine::Ref<Engine::Texture2D> m_BloomDirtTexture;
	const uint32_t m_BloomWorkGroupSize = 4;
	bool m_BloomEnabled = true;
	bool m_BloomDirtEnabled = true;
	float m_BloomThreshold = 1.240f;
	float m_BloomKnee = 0.220f;
	float UpsampleScale = 1.0f;
	float m_BloomIntensity = 0.790f;
	float m_BloomDirtIntensity = 1.0f;
	float m_Exposure = 1.0f;

private:
	bool m_Orthographic = false;
	int m_DragID = -1;
	bool m_IsDragging = false;

private:
	Engine::Ref<Engine::BezierSurface> m_Surface;

private:
	Engine::Camera m_Camera;
	glm::vec4 m_ClearColor{ 0.0f, 0.0f, 0.0f, 0.0f };

	Engine::Ref<Engine::Framebuffer> m_FBO;
	Engine::Ref<Engine::Texture2D> m_WhiteTexture;
	Engine::Ref<Engine::Texture2D> m_BrickWallTexture;
	Engine::Ref<Engine::Texture2D> m_GroundTexture;
	Engine::Ref<Engine::SimpleEntity> m_FullScreenQuad;

	bool m_ChopSticksAOn = true;
	bool m_AnimateChopsticks = false;
	float m_NextChopStickAnimationTime = 2.0f;
	float m_ChopStickAnimationPeriod = 1.0f;
	float m_ChopstickCounter = 0.0f;
	glm::vec3 m_ChopStickAColor;
	glm::vec3 m_ChopStickBColor;
	Engine::Ref<Engine::BezierCurve> m_ChopstickA;
	Engine::Ref<Engine::BezierCurve> m_ChopstickB;

	Engine::Ref<Engine::SimpleEntity> m_Table;
	Engine::Ref<Engine::SimpleEntity> m_BrickWall;
	Engine::RendererMaterialProperties m_TableProperties
	{
		{0.4f, 0.4f, 0.4f},
		{1.0f,  1.0f, 1.0f},
		0.6f,
		0.8f,
		1.00f,
		2.0f
	};

	Engine::RendererMaterialProperties m_FlagProperties
	{
		{0.4f, 0.4f, 0.4f},
		{0.6f, 0.6f, 0.6f},
		0.260f,
		0.7f,
		0.1f,
		2.0f
	};

	Engine::RendererMaterialProperties m_BrickProperties
	{
		{0.5f, 0.5f, 0.5f},
		{0.6f, 0.6f, 0.6f},
		0.5f,
		0.7f,
		0.1f,
		2.0f
	};

	Engine::Ref<Engine::Light> m_Light;

	int m_CurveEditIndex = 0;
	std::vector<Engine::Ref<Engine::BezierCurve>> m_Curves;
	glm::vec3 m_LineColor{ 10.0f, 1.0f, 6.0f };
	std::string m_SaveNameHolder = "Curve";

	uint32_t m_ViewportWidth;
	uint32_t m_ViewportHeight;

private:
	bool m_DrawWall = true;
	bool m_DrawGround = true;
	bool m_DrawFlag = true;
	bool m_DrawSign = true;
};