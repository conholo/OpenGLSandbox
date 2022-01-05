#pragma once

#include "Engine.h"


enum class ChannelMask { R, G, B, A };

class WorleyGeneratorLayer : public Engine::Layer
{
public:
	WorleyGeneratorLayer();
	~WorleyGeneratorLayer();

	void OnAttach() override;
	void OnDetach() override;
	void OnUpdate(float deltaTime) override;
	void OnEvent(Engine::Event& event) override;
	void OnImGuiRender() override;

private:
	void UpdateChannel(ChannelMask mask, float persistence, const glm::ivec3& cells);

	void DrawChannelSelector(const std::string& label);
	void DrawDisplayChannelMaskSelector(const std::string& label);

private:
	float m_DepthViewer = 0.0f;
	float m_TextureTiling = 1.0f;
	int m_Seed = 1;
	const uint32_t m_ThreadGroupSize = 8;
	uint32_t m_Resolution = 128;

	ChannelMask m_ActiveMask = ChannelMask::R;
	float m_Persistence = 0.1f;
	glm::ivec3 m_LayerCells{ 2, 6, 10 };

	Engine::Ref<Engine::ShaderStorageBuffer> m_WorleyPointsBufferA;
	Engine::Ref<Engine::ShaderStorageBuffer> m_WorleyPointsBufferB;
	Engine::Ref<Engine::ShaderStorageBuffer> m_WorleyPointsBufferC;
	Engine::Ref<Engine::Texture3D> m_Worley3DTexture;

	Engine::Ref<Engine::SimpleEntity> m_Entity;
	Engine::Ref<Engine::Texture2D> m_WhiteTexture;
	Engine::Camera m_Camera;
};