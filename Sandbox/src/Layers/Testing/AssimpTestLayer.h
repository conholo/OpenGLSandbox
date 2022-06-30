#pragma once

#include "Engine.h"

class AssimpTestLayer : public Engine::Layer
{
public:
	AssimpTestLayer();
	~AssimpTestLayer();

	void OnAttach() override;
	void OnDetach() override;
	void OnUpdate(float deltaTime) override;
	void OnEvent(Engine::Event& e) override;
	void OnImGuiRender() override;

private:
	Engine::Ref<Engine::Texture2D> m_WhiteTexture;
	Engine::Ref<Engine::SimpleEntity> m_Sphere;
	Engine::Camera m_Camera;
};