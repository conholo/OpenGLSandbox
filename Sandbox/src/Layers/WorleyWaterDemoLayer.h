#pragma once

#include "Engine.h"
#include <vector>

enum class SurfaceType { None = 0, Ripple, Wave, MultiWave };

struct BlinnPhongProperties
{
	BlinnPhongProperties() = default;
	glm::vec3 AmbientColor = glm::vec3(1.0f, 1.0f, 1.0f);
	glm::vec3 DiffuseColor = glm::vec3(1.0f, 1.0f, 1.0f);
	float AmbientStrength = 1.0f;
	float DiffuseStrength = 1.0f;
	float SpecularStrength = 1.0f;
	float Shininess = 1.0f;
};

struct FloatingBox
{
	FloatingBox()
	{
		Randomize();
	}

	void Randomize()
	{
		Position = { Engine::Random::RandomRange(-70.0f, 70.0f), 0.0f, Engine::Random::RandomRange(-70.0f, 70.0f) };

		float scale = Engine::Random::RandomRange(1.0f, 5.0f);
		Scale = { scale, scale, scale };
	}

	glm::vec3 Position;
	glm::vec3 Scale;
};

class WorleyWaterDemoLayer : public Engine::Layer
{
public:
	WorleyWaterDemoLayer();
	~WorleyWaterDemoLayer();

	void OnAttach() override;
	void OnDetach() override;
	void OnUpdate(float deltaTime) override;
	void OnEvent(Engine::Event& event) override;
	void OnImGuiRender();

private:
	bool OnKeyPressed(Engine::KeyPressedEvent& keyPressedEvent);

private:
	Engine::Camera m_Camera;
	glm::vec4 m_ClearColor{ 0.1f, 0.1f, 0.1f, 0.1f };

private:
	glm::vec3 m_LightMovementDirection{ 0.0f, 0.0f, 0.0f };
	bool m_MoveLight = false;
	SurfaceType m_SurfaceType = SurfaceType::Ripple;
	bool m_AnimateVertex = false;
	bool m_AnimateFragment = false;
	bool m_PauseVertex = false;
	bool m_PauseFragment = false;
	bool m_ShowBoxes = false;
	bool m_WireFrame = false;
	float m_Counter = 0;
	float m_VertexCounter = 0;
	float m_FragmentCounter = 0;
	
	uint32_t m_FloatingBoxesCount = 10;
	std::vector<FloatingBox> m_FloatingBoxes;
	BlinnPhongProperties m_BoxProperties;

	Engine::Ref<Engine::SimpleEntity> m_Entity;
	Engine::Ref<Engine::SimpleEntity> m_FloatingEntity;
	Engine::Ref<Engine::Light> m_PointLight;
	Engine::Ref<Engine::Texture2D> m_WhiteTexture;
	Engine::Ref<Engine::Texture2D> m_BoxTexture;
};