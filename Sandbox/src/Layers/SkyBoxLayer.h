#pragma once

#include "Engine.h"

class SkyboxLayer : public Engine::Layer
{
public:
	SkyboxLayer();
	~SkyboxLayer();

	void OnAttach() override;
	void OnDetach() override;
	void OnUpdate(float deltaTime) override;
	void OnEvent(Engine::Event& event) override;

private:
	bool OnKeyPressed(Engine::KeyPressedEvent& keyPressedEvent);
	void DrawSkybox(float deltaTime);
	void DrawReflectionSpheres();

private:
	Engine::Camera m_Camera;
	glm::vec4 m_ClearColor{ 0.1f, 0.1f, 0.1f, 0.1f };

private:
	float m_Counter;
	Engine::Ref<Engine::SimpleEntity> m_Cube;
	Engine::Ref<Engine::SimpleEntity> m_NonReflectedCube;

	// Skybox
	Engine::Ref<Engine::TextureCube> m_CubeTexture;
	Engine::Ref<Engine::CubeMap> m_SkyBox;
	bool m_AnimateInclination = true;
	float m_TAIStep = 1.0f;
	glm::vec3 m_TAI{ 3.0f, 3.0f, 0.1f };

	// Helper Texture
	Engine::Ref<Engine::Texture2D> m_WhiteTexture;

	// Grid
	Engine::Ref<Engine::EditorGrid> m_EditorGrid;
};