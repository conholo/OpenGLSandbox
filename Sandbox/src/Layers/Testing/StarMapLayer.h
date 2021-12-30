#pragma once

#include "Engine.h"

class StarMapLayer : public Engine::Layer
{
public:
	StarMapLayer();
	~StarMapLayer();

	void OnAttach() override;
	void OnDetach() override;
	void OnUpdate(float deltaTime) override;
	void OnEvent(Engine::Event& event) override;
	void OnImGuiRender() override;

private:
	bool OnKeyPressed(Engine::KeyPressedEvent& keyPressedEvent);

private:
	Engine::Ref<Engine::SimpleEntity> m_Plane;
	Engine::Ref<Engine::SimpleEntity> m_Sphere;

	Engine::Ref<Engine::Texture2D> m_WhiteTexture;
	Engine::Camera m_Camera;
};