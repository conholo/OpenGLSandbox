#pragma once

#include "Engine/Core/Memory.h"
#include "Engine/Rendering/ShaderStorageBuffer.h"
#include "Engine/Rendering/Texture.h"
#include <vector>
#include <glm/glm.hpp>

#include "Layers/410/CloudsUtility/CloudEnums.h"

struct CloudAnimationSettings
{
	bool AnimateClouds = true;
	float AnimationSpeed = 5.5f;
	float TimeScale = 0.011f;
	float CloudScrollOffsetSpeed = 0.00001f;
	glm::vec3 ShapeTextureOffset{ 0.0f, 0.0f, 0.0f };
};

struct CloudSettings
{
	bool DrawClouds = true;
	glm::vec3 SkyColorA = { 0.18f, 0.184f, 0.188f };
	glm::vec3 SkyColorB = { 0.67f, 0.78f, 0.79f };

	int DensitySteps = 50;
	int LightSteps = 10;

	glm::vec3 BaseShapeTextureOffset = glm::vec3(0.0);
	float DetailNoiseWeight = 1.0f;
	float CloudScale = 5.0f;
	float DensityThreshold = 0.12f;
	float DensityMultiplier = 0.43f;
	float PhaseBlend = 0.5f;
	float ForwardScattering = 0.91f;
	float BackScattering = 0.33f;
	float BaseBrightness = 1.0f;
	float PhaseFactor = 0.74f;
	float PowderConstant = 1.0f;
	float SilverLiningConstant = 0.644f;
	glm::vec4 ShapeNoiseWeights{ 0.95f, 0.48f, 0.32f, 0.42f };
	glm::vec3 DetailNoiseWeights{ 0.23f, 0.33f, 0.45f };

	glm::vec3 CloudContainerPosition{ 0.0f, 75.0f, 0.0f };
	glm::vec3 CloudContainerScale{ 400.0f, 300.0f, 300.0f };
	float CloudScaleFactor = 1000.0f;
	float ContainerEdgeFadeDistance = 50.0f;
};

struct CurlSettings
{
	CurlSettings();

	const uint32_t CurlThreadGroupSize = 8;
	uint32_t CurlResolution = 128;

	float Strength = 0.5f;
	float Tiling = 3.5f;
	glm::vec2 TilingOffset{ 0.0f, 0.0f };

	Engine::Ref<Engine::Texture2D> CurlTexture;

	void UpdateTexture();
};

struct WorleyPerlinSettings
{
	WorleyPerlinSettings();

	const uint32_t PerlinThreadGroupSize = 8;
	uint32_t PerlinResolution = 128;

	int Octaves = 5;
	float NoiseScale = 2.1f;
	float Lacunarity = 2.0f;
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
	float InversionWeight = 0.0f;
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
	void UpdateChannel(uint32_t threadGroups);
};

struct TextureDebugDisplaySettings
{
	bool EnableTextureViewer = true;
	float PercentScreenTextureDisplay = 0.1f;
	std::vector<CloudUIType> EditorTypes = { CloudUIType::BaseShape, CloudUIType::DetailShape, CloudUIType::Perlin, CloudUIType::Curl };
};

struct ShapeTextureDebugDisplaySettings
{
	float DepthSlice;
	bool DisplaySelectedChannelOnly = false;
	bool ShowAlpha = false;
	bool DrawAllChannels = true;
	bool EnableGreyScale = false;
	glm::vec4 ChannelWeights{ 0.0f, 0.0f,0.0f, 0.0f };
	std::vector<WorleyChannelMask> ChannelTypes = { WorleyChannelMask::R, WorleyChannelMask::G, WorleyChannelMask::B, WorleyChannelMask::A };
};

struct DetailTextureDebugDisplaySettings
{
	float DepthSlice;
	bool DisplaySelectedChannelOnly = false;
	bool DrawAllChannels = true;
	bool EnableGreyScale = false;
	glm::vec3 ChannelWeights{ 0.0f, 0.0f,0.0f };
	std::vector<WorleyChannelMask> ChannelTypes = { WorleyChannelMask::R, WorleyChannelMask::G, WorleyChannelMask::B };
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

	float PerlinWorleyMix = 0.1f;
	Engine::Ref<Engine::Texture3D> BaseShapeTexture;
	Engine::Ref<WorleyChannelData> ChannelR;
	Engine::Ref<WorleyChannelData> ChannelG;
	Engine::Ref<WorleyChannelData> ChannelB;
	Engine::Ref<WorleyChannelData> ChannelA;

	void UpdateChannel(WorleyChannelMask mask, const Engine::Ref<Engine::Texture2D>& perlinTexture);
	void UpdateAllChannels(const Engine::Ref<WorleyPerlinSettings>& perlinSettings);
};

struct DetailShapeWorleySettings
{
	DetailShapeWorleySettings();

	const uint32_t DetailThreadGroupSize = 8;
	uint32_t DetailResolution = 32;

	glm::ivec3 DefaultLayerCellsR{ 2, 6, 10 };
	glm::ivec3 DefaultLayerCellsG{ 5, 8, 12 };
	glm::ivec3 DefaultLayerCellsB{ 8, 16, 25 };
	glm::vec3 DefaultPersistence{ 0.1f, 0.3f, 0.5f };

	Engine::Ref<Engine::Texture3D> DetailShapeTexture;
	Engine::Ref<WorleyChannelData> ChannelR;
	Engine::Ref<WorleyChannelData> ChannelG;
	Engine::Ref<WorleyChannelData> ChannelB;

	void UpdateChannel(WorleyChannelMask mask);
	void UpdateAllChannels();
};