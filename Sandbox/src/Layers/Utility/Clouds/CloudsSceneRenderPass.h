#pragma once

#include "Engine/Core/Memory.h"
#include "Engine/Rendering/FrameBuffer.h"
#include "Engine/Rendering/Camera.h"
#include "Engine/Scene/SimpleECS/Light.h"
#include "Engine/Scene/SimpleECS/SimpleEntity.h"
#include "Engine/Experimental/Terrain.h"
#include "Engine/Rendering/EditorGrid.h"

class CloudsSceneRenderPass
{
public:
	CloudsSceneRenderPass();
	~CloudsSceneRenderPass();

	void ExecutePass(const Engine::Camera& camera);

	void BindMainColorAttachment(uint32_t slot) const { return m_FBO->BindColorAttachment(slot); }
	void BindDepthAttachment(uint32_t slot) const { return m_FBO->BindDepthTexture(slot); }
	const Engine::Ref<Engine::Light>& GetSunLight() const { return m_Sun; }
	void Resize(uint32_t width, uint32_t height);

	const std::vector<Engine::Ref<Engine::TerrainHeightLayer>>& GetTerrainHeightLayers() const { return m_HeightLayers; }

	float* GetSeaFrequency() { return &m_SeaFrequency; }
	float* GetSeaAmplitude() { return &m_SeaAmplitude; }
	float* GetSeaChoppy() { return &m_SeaChoppy; }
	float* GetSeaHeight() { return &m_SeaHeight; }
	uint32_t* GetOceanOctaves() { return &m_OceanOctaves; }
	uint32_t* GetOceanSteps() { return &m_OceanSteps; }

	const Engine::Ref<Engine::Terrain>& GetTerrain() const { return m_Terrain; }
	int* GetTerrainLOD() { return &m_TerrainLOD; }
	bool* GetTerrainIsWireframe() { return &m_TerrainIsWireframe; }
	bool* GetDrawTerrain() { return &m_DrawTerrain; }

private:
	void DrawSceneEntities(const Engine::Camera& camera);

private:
	bool m_DrawTerrain = true;
	int m_TerrainLOD = 0;
	bool m_TerrainIsWireframe = false;
	Engine::Ref<Engine::Framebuffer> m_FBO;
	glm::vec4 m_ClearColor{ 0.1f, 0.1f, 0.1f, 0.1f };


	float m_SeaFrequency = .1f;
	float m_SeaAmplitude = .6f;
	float m_SeaHeight = -4.0f;
	float m_SeaChoppy = 5.5f;
	uint32_t m_OceanOctaves = 8;
	uint32_t m_OceanSteps = 5;

	Engine::Ref<Engine::EditorGrid> m_EditorGrid;
	Engine::Ref<Engine::Terrain> m_Terrain;
	Engine::Ref<Engine::Texture2D> m_WhiteTexture;
	Engine::Ref<Engine::Texture2D> m_WaterTexture;
	Engine::Ref<Engine::Light> m_Sun;
	Engine::Ref<Engine::SimpleEntity> m_GroundPlane;
	int m_HeightLayerCount = 7;
	std::vector<Engine::Ref<Engine::TerrainHeightLayer>> m_HeightLayers;
};