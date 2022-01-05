#pragma once

#include "Engine.h"

class CubeMapLayer : public Engine::Layer
{
public:
	CubeMapLayer();
	~CubeMapLayer();

	void OnAttach() override;
	void OnDetach() override;
	void OnUpdate(float deltaTime) override;
	void OnEvent(Engine::Event& event) override;

private:
	bool OnKeyPressed(Engine::KeyPressedEvent& event);

	void DrawReflectionScene();

private:
	float m_CameraYawRotate = 0.0f;
	bool m_CameraOrbitModeActive = false;

	Engine::Camera m_Camera;

private:
	Engine::Ref<Engine::CubeMap> m_CubeMap;
	Engine::Ref<Engine::CubeMap> m_SkyMap;
	Engine::Ref<Engine::TextureCube> m_SkyTexture3D;
};