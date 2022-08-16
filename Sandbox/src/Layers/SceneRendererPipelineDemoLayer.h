#pragma once

#include "Engine.h"

class SceneRendererPipelineDemoLayer : public Engine::Layer
{
public:
	SceneRendererPipelineDemoLayer();
	~SceneRendererPipelineDemoLayer();

	void OnAttach() override;
	void OnDetach() override;
	void OnUpdate(float deltaTime) override;
	void OnEvent(Engine::Event& event) override;
	void OnImGuiRender();

private:
	bool OnKeyPressed(Engine::KeyPressedEvent& keyPressedEvent);
	bool OnResize(Engine::WindowResizedEvent& windowResizeEvent);

private:
	float m_Counter = 0.0f;

	Engine::Ref<Engine::Scene> m_Scene = nullptr;

	Engine::Entity m_Sphere;
	Engine::Entity m_Plane;
	Engine::Entity m_Cube;
	Engine::Entity m_Light;

	Engine::RendererMaterialProperties m_PlaneProperties =
	{
		{0.5f ,0.5f, 0.5f},
		{1.0f, 1.0f, 1.0f},
		0.6f,
		0.5f,
		1.0f,
		32.0f
	};

	Engine::RendererMaterialProperties m_CubeProperties =
	{
		{0.5f ,0.5f, 0.5f},
		{0.0f, 0.8f, 0.5f},
		0.3f,
		0.5f,
		1.0f,
		16.0f
	};

	Engine::RendererMaterialProperties m_SphereProperties =
	{
		{0.5f ,0.5f, 0.5f},
		{0.0f, 0.8f, 0.5f},
		0.3f,
		0.5f,
		1.0f,
		16.0f
	};

};