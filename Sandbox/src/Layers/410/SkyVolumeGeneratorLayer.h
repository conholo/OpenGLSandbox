#pragma once

#include "Engine.h"

class SkyVolumeGeneratorLayer : public Engine::Layer
{
public:
	SkyVolumeGeneratorLayer();
	~SkyVolumeGeneratorLayer();

	void OnAttach() override;
	void OnDetach() override;
	void OnUpdate(float deltaTime) override;
	void OnEvent(Engine::Event& event) override;
	void OnImGuiRender() override;

private:
	glm::vec3 m_TAI{ 3.0f, 3.0f, 0.1f };
	Engine::Ref<Engine::TextureCube> m_TextureCube;
	Engine::Ref<Engine::CubeMap> m_CubeMap;

	Engine::Ref<Engine::SimpleEntity> m_Entity;
	Engine::Ref<Engine::Texture2D> m_WhiteTexture;
	Engine::Camera m_Camera;
};