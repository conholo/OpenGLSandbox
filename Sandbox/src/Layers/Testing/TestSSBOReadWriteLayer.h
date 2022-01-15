#pragma once

#include "Engine.h"
#include <glm/glm.hpp>

class TestSSBOReadWriteLayer : public Engine::Layer
{
public:
	TestSSBOReadWriteLayer();
	~TestSSBOReadWriteLayer();

	void OnAttach() override;
	void OnDetach() override;
	void OnUpdate(float deltaTime) override;
	void OnEvent(Engine::Event& e) override;
	void OnImGuiRender() override;

private:

	uint32_t m_LocalGroupSize = 8;
	glm::ivec2 m_Dimensions{ 32, 32 };
	std::vector<glm::vec4> m_Values;

	Engine::Ref<Engine::Texture2D> m_RWTexture;
	Engine::Ref<Engine::ShaderStorageBuffer> m_TestSSBO;
	Engine::Camera m_Camera;

	Engine::Ref<Engine::SimpleEntity> m_QuadDisplay;
};