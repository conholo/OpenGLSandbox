#pragma once

#include "Engine.h"
#include <glm/glm.hpp>
#include <stdint.h>
#include <imgui/imgui.h>
#include <imgui/imfilebrowser.h>

#include "Prague/PragueSkyModel.h"

class AtmosphereLayer : public Engine::Layer
{
public:
	AtmosphereLayer();
	~AtmosphereLayer();

	void OnAttach() override;
	void OnDetach() override;
	void OnUpdate(float deltaTime) override;
	void OnEvent(Engine::Event& e) override;
	void OnImGuiRender() override;

private:

	void UpdateSkyBox();

private:
	uint32_t m_ViewportWidth;
	uint32_t m_ViewportHeight;
	glm::vec4 m_ClearColor{ 0.0f, 0.0f, 0.0f, 0.0f };

	Engine::Ref<Engine::SimpleEntity> m_FSQ;

private:
	Engine::Camera m_Camera;

	Engine::Ref<Engine::Framebuffer> m_HDRFBO;
	Engine::Ref<Engine::Texture2D> m_WhiteTexture;
	Engine::Ref<Engine::SimpleEntity> m_Planet;
	Engine::Ref<Engine::SimpleEntity> m_Ground;

private:
	const uint32_t m_ThreadsPerGroup = 32;
	Engine::Ref<Engine::CubeMap> m_SkyBox;
	Engine::Ref<Engine::TextureCube> m_CubeTexture;
	Engine::Ref<Engine::EditorGrid> m_EditorGrid;

private:

	float m_Albedo = 0.5;
	float m_Altitude = 2500.0f;
	float m_Azimuth = 0.0;
	float m_Elevation = 0.0;
	float m_Visibility = 25.0f;
	float m_Exposure = 0.0;

	Engine::Ref<Engine::ShaderStorageBuffer> m_RadianceDataSSBO;
	Engine::Ref<Engine::ShaderStorageBuffer> m_SunMetaDataSSBO;
	Engine::Ref<Engine::ShaderStorageBuffer> m_ZenithMetaDataSSBO;
	Engine::Ref<Engine::ShaderStorageBuffer> m_EmphMetaDataSSBO;
	Engine::Ref<Engine::ShaderStorageBuffer> m_VisibilitiesRadianceSSBO;
	Engine::Ref<Engine::ShaderStorageBuffer> m_AlbedosRadianceSSBO;
	Engine::Ref<Engine::ShaderStorageBuffer> m_AltitudesRadianceSSBO;
	Engine::Ref<Engine::ShaderStorageBuffer> m_ElevationsRadianceSSBO;

	PragueSkyModel* m_SkyModel;
	PragueSkyModel::AvailableData m_AvailableData;

	ImGui::FileBrowser fileDialogOpen;
};