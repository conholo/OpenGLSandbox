#pragma once

#include "Engine.h"

class BlankLayer : public Engine::Layer
{
public:
	BlankLayer();
	~BlankLayer();

	void OnAttach() override;
	void OnDetach() override;
	void OnUpdate(float deltaTime) override;
	void OnEvent(Engine::Event& e) override;
	void OnImGuiRender() override;

private:
	Engine::Camera m_Camera;
};