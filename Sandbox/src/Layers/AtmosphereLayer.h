#pragma once

#include "Engine.h"
#include <glm/glm.hpp>
#include <stdint.h>
#include <imgui/imgui.h>
#include <imgui/imfilebrowser.h>

#include "Prague/PragueUI.h"
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

	void LoadDataset(const std::string& filepath, double visibility);

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
	bool m_DisplayGrid = true;
	Engine::Ref<Engine::EditorGrid> m_EditorGrid;

private:

	bool m_ApplyTonemap = true;
	int m_Direction = 1;
	float m_Timer = 0.0f;
	bool m_IsLoaded = false;
	bool m_AnimateElevation = false;
	float m_AnimationSpeed = 0.00001f;

	Engine::Ref<Engine::ShaderStorageBuffer> m_RadianceDataSSBO;
	Engine::Ref<Engine::ShaderStorageBuffer> m_SunMetaDataSSBO;
	Engine::Ref<Engine::ShaderStorageBuffer> m_ZenithMetaDataSSBO;
	Engine::Ref<Engine::ShaderStorageBuffer> m_EmphMetaDataSSBO;
	Engine::Ref<Engine::ShaderStorageBuffer> m_VisibilitiesRadianceSSBO;
	Engine::Ref<Engine::ShaderStorageBuffer> m_AlbedosRadianceSSBO;
	Engine::Ref<Engine::ShaderStorageBuffer> m_AltitudesRadianceSSBO;
	Engine::Ref<Engine::ShaderStorageBuffer> m_ElevationsRadianceSSBO;

	bool m_Update = false;
	PragueInput* m_Input;
	PragueUI* m_UI;
	PragueSkyModel* m_SkyModel;
	PragueSkyModel::AvailableData m_AvailableData;
};