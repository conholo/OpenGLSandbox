#pragma once

#include "Engine/Core/Memory.h"
#include "Engine/Rendering/ShaderStorageBuffer.h"
#include "Engine/Rendering/Texture.h"
#include <vector>
#include <glm/glm.hpp>

#include "CloudEnums.h"

struct CloudAnimationSettings
{
	bool AnimateClouds = true;
	float AnimationSpeed = 5.5f;
	float TimeScale = 0.002f;
	float CloudScrollOffsetSpeed = 0.00001f;
	glm::vec3 ShapeTextureOffset{ 0.0f, 0.0f, 0.0f };
};

struct WaterData
{
	bool DrawWater = true;
	float SeaFrequency = .870f;
	float SeaAmplitude = .05f;
	float SeaHeight = -2.8f;
	float SeaChoppy = 5.5f;
	uint32_t OceanOctaves = 20;
	uint32_t OceanSteps = 8;
};

struct CloudSettings
{
	bool DrawClouds = true;
	// Sky / Sun Parameters
	glm::vec3 SkyColorA = { 45.0f / 255.0f, 74.0 / 255.0f, 103.0 / 255.0f};
	glm::vec3 SkyColorB = { 19.0f / 255.0f, 144.0f / 255.0f, 244.0f / 255.0f };
	float SunSize = 1.0f;				// Relative size of the sun.

	// March Parameters
	int DensitySteps = 76;
	int LightSteps = 14;
	float RandomOffsetStrength = 12.0f;

	// Phase Parameters
	float PhaseBlend = 1.0f;			// The amount of blend between forward/back scattering. (0.5 is a good mix)
	float ForwardScattering = 0.996f;	// Amount of light that is scattered towards the view direction when looking towards the sun through the clouds (0.2 is good).
	float BackScattering = 0.618f;		// Amount of light that is scattered away from the view direction when looking towards the sun through the clouds (0.5 is good)
	float BaseBrightness = 1.279f;		// Additive factor associated with the HG Phase Function (1.0 is a good value)
	float PhaseFactor = 0.997f;			// Multiplicative factor associated with the HG Phase Function (1.0 is a good value)

	// Lighting Parameters
	float PowderConstant = 4.24f;		// In Scattering probability factor.  Creases where bulges occur are more likely to produce in-scattered light.
	float SilverLiningConstant = 0.648;	// In Scattering probability factor.  When looking towards the sun, light is more likely to scatter towards the viewer around the cloud edges.
	float ExtinctionFactor = .14f;

	// Density Parameters
	float DensityThreshold = .214;
	float DensityMultiplier = .144f;
	glm::vec4 ShapeNoiseWeights{ 0.0f, 1.0f, .28f, .36f };
	glm::vec3 BaseShapeTextureOffset = glm::vec3(0.0);

	glm::vec3 DetailNoiseWeights{ 1.0, 1.0, 0.0 };
	float DetailNoiseWeight = .02f;

	glm::vec3 CloudTypeWeights{ 0.25f, 0.33f, 0.5f };
	float CloudTypeWeightStrength = 6.0f;

	float CurlIntensity = 30.0f;

	// Container / Scale Parameters
	glm::vec3 CloudContainerPosition{ 0.0f, 190.0f, 0.0f };
	glm::vec3 CloudContainerScale{ 700.0f, 300.0f, 300.0f };
	float CloudScaleFactor = 1153.5f;
	float CloudScale = 2.260f;
	float ContainerEdgeFadeDistance = 41.0f;
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
	uint32_t PerlinResolution = 512;

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
	bool InvertWorley = true;
	float WorleyTiling = 1.0f;
	float WorleyLayerPersistence = 0.1f;
	float InversionWeight = 1.0f;
	glm::ivec3 LayerCells;
	glm::ivec3 LayerSeeds;

	std::vector<glm::vec4> PointsA;
	std::vector<glm::vec4> PointsB;
	std::vector<glm::vec4> PointsC;
	std::vector<int> MinMax;
	Engine::Ref<Engine::ShaderStorageBuffer> ShapePointsBufferA;
	Engine::Ref<Engine::ShaderStorageBuffer> ShapePointsBufferB;
	Engine::Ref<Engine::ShaderStorageBuffer> ShapePointsBufferC;
	Engine::Ref<Engine::ShaderStorageBuffer> MinMaxBuffer;

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