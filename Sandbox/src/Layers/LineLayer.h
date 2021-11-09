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
	bool OnKeyPressed(Engine::KeyPressedEvent& keyPressedEvent);
	bool OnMouseButtonReleased(Engine::MouseButtonReleasedEvent& releasedEvent);

private:
	Engine::Camera m_Camera;
	glm::vec4 m_ClearColor{ 0.1f, 0.1f, 0.1f, 0.1f };

	Engine::Ref<Engine::Texture2D> m_WhiteTexture;
	Engine::Ref<Engine::Line> m_Line;
	Engine::Ref<Engine::SimpleEntity> m_Cube;
};