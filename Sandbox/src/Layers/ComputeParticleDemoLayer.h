#pragma once

#include "Engine.h"


struct QuadVertex
{
	glm::vec3 Position;
	glm::vec4 Color;
};

struct Particle
{
	glm::vec4 Position;
	glm::vec4 Color;
};


class ComputeParticleDemoLayer : public Engine::Layer
{
public:
	ComputeParticleDemoLayer();
	~ComputeParticleDemoLayer();

	void OnAttach() override;
	void OnDetach() override;
	void OnUpdate(float deltaTime) override;
	void OnEvent(Engine::Event& event) override;

private:
	void Reset();
	bool OnKeyPressed(Engine::KeyPressedEvent& event);

private:
	float m_CameraYawRotate = 0.0f;
	bool m_CameraOrbitModeActive = true;

	Engine::Ref<Engine::ShaderStorageBuffer> m_ParticleBuffer;
	Engine::Ref<Engine::ShaderStorageBuffer> m_VelocityBuffer;

	uint32_t m_ParticleCount = 1024 * 1024;
	Particle* m_Particles = nullptr;
	glm::vec4* m_Velocities = nullptr;

	Engine::Ref<Engine::Texture2D> m_BrickTexture;
	Engine::Ref<Engine::Texture2D> m_GrassTexture;
	Engine::Ref<Engine::Texture2D> m_WhiteTexture;
	Engine::Ref<Engine::SimpleEntity> m_GroundPlane;
	Engine::Camera m_Camera;
};