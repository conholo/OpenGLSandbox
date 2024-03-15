#pragma once

#include "Engine.h"
#include "CloudDataStructures.h"
#include "CloudsSceneRenderPass.h"

struct CloudsUIData
{
	Engine::Ref<BaseShapeWorleySettings> BaseShapeSettings;
	Engine::Ref<DetailShapeWorleySettings> DetailShapeSettings;
	Engine::Ref<WorleyPerlinSettings> PerlinSettings;
	Engine::Ref<CloudSettings> MainCloudSettings;
	Engine::Ref<CloudAnimationSettings> AnimationSettings;
	Engine::Ref<CloudsSceneRenderPass> SceneRenderPass;
	Engine::Ref<CurlSettings> CloudCurlSettings;
	Engine::Ref<WaterData> WaterSettings;
};

struct UITab
{
	int Index = -1;
	std::string TabName;
};

class CloudsUI
{
public:
	CloudsUI();
	~CloudsUI();

	void Draw(const Engine::Ref<CloudsUIData>& uiData);
	const Engine::Ref<TextureDebugDisplaySettings>& GetTextureDisplaySettings() const { return m_MainTextureDebugSettings; }

	bool GetDisplayAlpha() const { return m_ActiveDebugShapeType == ActiveDebugShapeType::None || m_ActiveDebugShapeType == ActiveDebugShapeType::DetailNoise ? false : m_ShapeTextureDisplaySettings->ShowAlpha; }
	bool GetShowAllChannels() const { return m_ActiveDebugShapeType == ActiveDebugShapeType::BaseShape ? m_ShapeTextureDisplaySettings->DrawAllChannels : m_DetailTextureDisplaySettings->DrawAllChannels; }
	bool GetEnableGreyScale() const { return m_ActiveDebugShapeType == ActiveDebugShapeType::BaseShape ? m_ShapeTextureDisplaySettings->EnableGreyScale : m_DetailTextureDisplaySettings->EnableGreyScale; }
	float GetDepthSlice() const { return m_ActiveDebugShapeType == ActiveDebugShapeType::BaseShape ? m_ShapeTextureDisplaySettings->DepthSlice : m_DetailTextureDisplaySettings->DepthSlice; }
	glm::vec4 GetChannelWeights() const { return m_ActiveDebugShapeType == ActiveDebugShapeType::BaseShape ? m_ShapeTextureDisplaySettings->ChannelWeights : glm::vec4(m_DetailTextureDisplaySettings->ChannelWeights, 1.0f); }
	
	ActiveDebugShapeType GetActiveShapeType() const { return m_ActiveDebugShapeType; }
	CloudUIType GetActiveUIType() const { return m_ActiveUIType; }

private:
	void DrawMainSettings(const Engine::Ref<CloudsUIData>& uiData);
	void DrawNoiseEditorSettings();
	void DrawCloudSettingsUI(const Engine::Ref<CloudSettings>& cloudSettings, const Engine::Ref<CloudAnimationSettings>& animationSettings, const Engine::Ref<CloudsSceneRenderPass>& sceneRenderPass);
	void DrawBaseShapeUI(const Engine::Ref<BaseShapeWorleySettings>& baseShapeSettings, const Engine::Ref<WorleyPerlinSettings>& perlinSettings);
	void DrawDetailShapeUI(const Engine::Ref<DetailShapeWorleySettings>& detailShapeSettings);
	void DrawPerlinUI(const Engine::Ref<WorleyPerlinSettings>& perlinSettings);
	void DrawCurlUI(const Engine::Ref<CurlSettings>& curlSettings);
	void DrawWaterUI(const Engine::Ref<WaterData>& waterSettings);

	void DrawTerrainSettingsUI(const Engine::Ref<CloudsSceneRenderPass>& cloudPass, const Engine::Ref<Engine::Terrain>& terrain, int* terrainLOD, bool* wireFrame);

	void DrawBaseShapeSelectionUI();
	void DrawDetailShapeSelectionUI();

private:
	const Engine::Ref<WorleyChannelData>& ShapeChannelFromMask(const Engine::Ref<BaseShapeWorleySettings>& baseShapeSettings);
	const Engine::Ref<WorleyChannelData>& DetailChannelFromMask(const Engine::Ref<DetailShapeWorleySettings>& detailShapeSettings);

private:
	std::vector<UITabTypes> m_AvailableTabTypes{ UITabTypes::MainSettings, UITabTypes::CloudSettings, UITabTypes::NoiseTextureSettings, UITabTypes::TerrainSettings, UITabTypes::WaterSettings };
	UITabTypes m_ActiveTabType = UITabTypes::MainSettings;
	ActiveDebugShapeType m_ActiveDebugShapeType = ActiveDebugShapeType::BaseShape;
	WorleyChannelMask m_ActiveShapeMask = WorleyChannelMask::R;
	WorleyChannelMask m_ActiveDetailMask = WorleyChannelMask::R;
	CloudUIType m_ActiveUIType = CloudUIType::BaseShape;
	Engine::Ref<TextureDebugDisplaySettings> m_MainTextureDebugSettings;
	Engine::Ref<DetailTextureDebugDisplaySettings> m_DetailTextureDisplaySettings;
	Engine::Ref<ShapeTextureDebugDisplaySettings> m_ShapeTextureDisplaySettings;
};