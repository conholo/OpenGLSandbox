#pragma once

#include "Engine.h"

enum class ChannelMask { All, R, G, B, A };

struct PerlinSettings
{
	int Octaves = 4;
	float NoiseScale = 5.0f;
	float Lacunarity = 32.0f;
	float Persistence = .65f;
	glm::vec2 TextureOffset{ 0.0f, 0.0f };
};

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
	void InitializeTextures();
	void UpdatePerlinTexture(bool updatePoints);
	void UpdateWorleyChannel(ChannelMask mask, float persistence, const glm::ivec3& cells);
	void DrawChannelSelector(const std::string& label, ChannelMask& mask, bool showAll = false);
	void DrawTextures(float deltaTime);

private:
	bool Resize(int width, int height);
	void InitializeSceneEntities();
	void DrawSceneEntities();

private:
	// Texture Display Settings
	bool m_ShowAllChannels = true;
	bool m_GreyScale = true;
	bool m_DisplayTextures = false;
	bool m_DisplayWorley = true;
	float m_DepthViewer = 0.0f;
	glm::vec4 m_ChannelWeights = glm::vec4(0.0);

private:
	// Texture Globals
	const uint32_t m_ThreadGroupSize = 8;
	uint32_t m_Resolution = 128;

private:
	// Perlin
	PerlinSettings m_PerlinSettings;
	bool m_AnimatePerlinOffsets = false;
	float m_OffsetAnimateSpeed = 2.0f;

	std::vector<glm::vec4> m_RandomPerlinOffsets;
	Engine::Ref<Engine::ShaderStorageBuffer> m_RandomPerlinOffsetsBuffer;
	Engine::Ref<Engine::Texture2D> m_PerlinTestTexture;
	Engine::Ref<Engine::SimpleEntity> m_PerlinDisplayEntity;

private:
	// Worley
	ChannelMask m_ActiveMask = ChannelMask::R;
	float m_TextureTiling = 1.0f;
	float m_WorleyLayerPersistence = 0.1f;
	glm::ivec3 m_LayerCells{ 2, 6, 10 };
	int m_RandomSeed = 1;

	Engine::Ref<Engine::ShaderStorageBuffer> m_WorleyPointsBufferA;
	Engine::Ref<Engine::ShaderStorageBuffer> m_WorleyPointsBufferB;
	Engine::Ref<Engine::ShaderStorageBuffer> m_WorleyPointsBufferC;
	Engine::Ref<Engine::Texture3D> m_Worley3DTexture;
	Engine::Ref<Engine::SimpleEntity> m_WorleyDisplayEntity;

private:
	// Scene
	Engine::Ref<Engine::SimpleEntity> m_CompositeQuad;
	Engine::Ref<Engine::Framebuffer> m_SceneFBO;
	Engine::Ref<Engine::Texture2D> m_WhiteTexture;
	Engine::Camera m_Camera;

	Engine::Ref<Engine::SimpleEntity> m_Cube;
	Engine::Ref<Engine::SimpleEntity> m_Sphere;
	Engine::Ref<Engine::SimpleEntity> m_GroudPlane;

	Engine::Ref<Engine::Light> m_Sun;
private:

	float m_AnimationSpeed = 0.0f;
	float m_TimeScale = 0.0001f;
	float m_CloudScrollOffsetSpeed = 0.00001f;
	bool m_AnimateClouds = false;

	glm::vec3 m_SkyColorA = {0.2f, 0.4f, 0.5f};
	glm::vec3 m_SkyColorB = { 0.3f, 0.4f, 0.9f };


	float m_ContainerEdgeFadeDistance = 10.0f;
	glm::vec4 m_PhaseParams{ 1.0f, 1.0f, 1.0f, 1.0f };
	float m_PhaseBlend = 0.5f;
	float m_PowderConstant = 0.5f;
	float m_SilverLiningConstant = 0.5f;
	bool m_InvertWorley = true;
	float m_PerlinWorleyMix = 0.21f;
	float m_DensityThreshold = .18f;
	float m_DensityMultiplier = 1.0f;
	float m_CloudScale = 15.0f;
	glm::vec3 m_CloudOffset = glm::vec3(0.0);
	int m_DensitySteps = 50;
	int m_LightSteps = 10;
	Engine::Ref<Engine::Framebuffer> m_CloudFBO;
	Engine::Ref<Engine::SimpleEntity> m_CloudQuad;
	glm::vec3 m_CloudContainerPosition{0.0f, 30.0f, 0.0f};
	glm::vec3 m_CloudContainerScale{100.0f, 50.0f, 100.0f};
	glm::vec4 m_ShapeNoiseWeights{ 0.4f, 0.3f, 0.2f, 0.1f };
};