#pragma once

#include "Engine.h"
#include "Layers/410/CloudsUtility/CloudDataStructures.h"


class CloudsUI
{
public:
	CloudsUI();
	~CloudsUI();

	void Draw(const Engine::Ref<BaseShapeWorleySettings>& baseShapeSettings, const Engine::Ref<WorleyPerlinSettings>& perlinSettings);
	const Engine::Ref<TextureDebugDisplaySettings>& GetTextureDisplaySettings() const { return m_TextureDisplaySettings; }

	WorleyChannelMask GetActiveChannelMask() const { return m_ActiveChannelMask; }
	CloudUIType GetActiveUIType() const { return m_ActiveUIType; }

private:
	void DrawBaseShapeUI(const Engine::Ref<BaseShapeWorleySettings>& baseShapeSettings, const Engine::Ref<WorleyPerlinSettings>& perlinSettings);
	void DrawPerlinUI(const Engine::Ref<WorleyPerlinSettings>& perlinSettings);

private:
	const Engine::Ref<WorleyChannelData>& ChannelFromMask(const Engine::Ref<BaseShapeWorleySettings>& baseShapeSettings);

private:
	WorleyChannelMask m_ActiveChannelMask = WorleyChannelMask::R;
	CloudUIType m_ActiveUIType = CloudUIType::BaseShape;
	Engine::Ref<TextureDebugDisplaySettings> m_TextureDisplaySettings;
};