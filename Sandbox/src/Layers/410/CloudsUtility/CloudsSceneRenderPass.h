#pragma once

#include "Engine/Core/Memory.h"
#include "Engine/Rendering/FrameBuffer.h"
#include "Engine/Rendering/Camera.h"
#include "Engine/Scene/SimpleECS/Light.h"
#include "Engine/Scene/SimpleECS/SimpleEntity.h"
#include "Engine/Experimental/Terrain.h"

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

	const Engine::Ref<Engine::Terrain>& GetTerrain() const { return m_Terrain; }
	int* GetTerrainLOD() { return &m_TerrainLOD; }
	bool* GetTerrainIsWireframe() { return &m_TerrainIsWireframe; }

private:
	void DrawSceneEntities(const Engine::Camera& camera);

private:
	int m_TerrainLOD = 3;
	bool m_TerrainIsWireframe = false;
	Engine::Ref<Engine::Framebuffer> m_FBO;
	glm::vec4 m_ClearColor{ 0.1f, 0.1f, 0.1f, 0.1f };

	Engine::Ref<Engine::Terrain> m_Terrain;
	Engine::Ref<Engine::Texture2D> m_WhiteTexture;
	Engine::Ref<Engine::Light> m_Sun;
	Engine::Ref<Engine::SimpleEntity> m_GroundPlane;
};