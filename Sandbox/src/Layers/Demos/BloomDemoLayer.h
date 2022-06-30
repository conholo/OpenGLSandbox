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
	void OnEvent(Engine::Event& e) override;
	void OnImGuiRender() override;

private:
	Engine::Camera m_Camera;
};