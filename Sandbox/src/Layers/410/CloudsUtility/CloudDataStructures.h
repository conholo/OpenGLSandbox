#pragma once

#include "Engine/Core/Memory.h"
#include "Engine/Rendering/ShaderStorageBuffer.h"
#include "Engine/Rendering/Texture.h"
#include <vector>
#include <glm/glm.hpp>

#include "Layers/410/CloudsUtility/CloudEnums.h"

struct CloudAnimationSettings
{
	bool AnimateClouds = false;
	float AnimationSpeed = 0.0f;
	float TimeScale = 0.0001f;
	float CloudScrollOffsetSpeed = 0.00001f;
};

struct CloudSettings
{
	glm::vec3 SkyColorA = { 0.2f, 0.4f, 0.5f };
	glm::vec3 SkyColorB = { 0.3f, 0.4f, 0.9f };

	int DensitySteps = 50;
	int LightSteps = 10;

	glm::vec3 BaseShapeTextureOffset = glm::vec3(0.0);
	float CloudScale = 15.0f;
	float DensityThreshold = .18f;
	float DensityMultiplier = 1.0f;
	float PhaseBlend = 0.5f;
	glm::vec4 PhaseParams{ 1.0f, 1.0f, 1.0f, 1.0f };
	float PowderConstant = 0.5f;
	float SilverLiningConstant = 0.5f;

	glm::vec3 CloudContainerPosition{ 0.0f, 30.0f, 0.0f };
	glm::vec3 CloudContainerScale{ 100.0f, 50.0f, 100.0f };
	float ContainerEdgeFadeDistance = 50.0f;
};

struct WorleyPerlinSettings
{
	WorleyPerlinSettings();

	const uint32_t PerlinThreadGroupSize = 8;
	uint32_t PerlinResolution = 128;

	int Octaves = 4;
	float NoiseScale = 5.0f;
	float Lacunarity = 32.0f;
	float Persistence = .65f;
	glm::vec2 TextureOffset{ 0.0f, 0.0f };

	std::vector<glm::vec4> RandomPerlinOffsets;
	Engine::Ref<Engine::ShaderStorageBuffer> RandomPerlinOffsetsBuffer;
	Engine::Ref<Engine::Texture2D> PerlinTexture;

	void UpdatePoints();
	void UpdateTexture();
};

struct WorleyChannelData
{
	WorleyChannelMask Mask;
	bool InvertWorley = false;
	float WorleyTiling = 1.0f;
	float WorleyLayerPersistence = 0.1f;
	glm::ivec3 LayerCells;
	glm::ivec3 LayerSeeds;

	std::vector<glm::vec4> PointsA;
	std::vector<glm::vec4> PointsB;
	std::vector<glm::vec4> PointsC;
	Engine::Ref<Engine::ShaderStorageBuffer> ShapePointsBufferA;
	Engine::Ref<Engine::ShaderStorageBuffer> ShapePointsBufferB;
	Engine::Ref<Engine::ShaderStorageBuffer> ShapePointsBufferC;

	void UpdatePoints();
	void UpdateChannel(const Engine::Ref<Engine::Texture2D>& perlinTexture, float perlinWorleyMix, uint32_t threadGroups);
};

struct TextureDebugDisplaySettings
{
	float PercentScreenTextureDisplay = 0.1f;
	float DepthSlice;
	bool DisplaySelectedChannelOnly = false;
	bool ShowAlpha = false;
	bool DrawAllChannels = true;
	bool EnableGreyScale = false;
	bool EnableTextureViewer = true;
	glm::vec4 ChannelWeights{ 0.0f, 0.0f,0.0f, 0.0f };
	std::vector<CloudUIType> EditorTypes = { CloudUIType::BaseShape, CloudUIType::DetailNoise, CloudUIType::Perlin };
	std::vector<WorleyChannelMask> ChannelTypes = { WorleyChannelMask::R, WorleyChannelMask::G, WorleyChannelMask::B, WorleyChannelMask::A };
};

struct BaseShapeWorleySettings
{
	BaseShapeWorleySettings();

	const uint32_t ShapeThreadGroupSize = 8;
	uint32_t ShapeResolution = 128;

	glm::ivec3 DefaultLayerCellsR{ 2, 6, 10 };
	glm::ivec3 DefaultLayerCellsG{ 5, 8, 12 };
	glm::ivec3 DefaultLayerCellsB{ 8, 16, 25 };
	glm::ivec3 DefaultLayerCellsA{ 14, 22, 33 };
	glm::vec4 DefaultPersistence{ 0.1f, 0.3f, 0.5f, 0.8f };

	glm::vec4 ShapeNoiseWeights{ 0.4f, 0.3f, 0.2f, 0.1f };
	float PerlinWorleyMix = 0.21f;
	Engine::Ref<Engine::Texture3D> BaseShapeTexture;
	Engine::Ref<WorleyChannelData> ChannelR;
	Engine::Ref<WorleyChannelData> ChannelG;
	Engine::Ref<WorleyChannelData> ChannelB;
	Engine::Ref<WorleyChannelData> ChannelA;

	void UpdateAllChannels(const Engine::Ref<WorleyPerlinSettings>& perlinSettings);

private:
	Engine::Ref<WorleyChannelData> CreateChannelData(WorleyChannelMask mask, const glm::ivec3& defaultCells, float persistence);
};