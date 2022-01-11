#include "Layers/410/CloudsUtility/CloudsUI.h"
#include "Layers/410/CloudsUtility/HelperFunctions.h"

#include <imgui/imgui.h>

CloudsUI::CloudsUI()
{
	m_TextureDisplaySettings = Engine::CreateRef<TextureDebugDisplaySettings>();
}

CloudsUI::~CloudsUI()
{

}

const Engine::Ref<WorleyChannelData>& CloudsUI::ChannelFromMask(const Engine::Ref<BaseShapeWorleySettings>& baseShapeSettings)
{
	switch (m_ActiveChannelMask)
	{
	case WorleyChannelMask::R: return baseShapeSettings->ChannelR;
	case WorleyChannelMask::G: return baseShapeSettings->ChannelG;
	case WorleyChannelMask::B: return baseShapeSettings->ChannelB;
	case WorleyChannelMask::A: return baseShapeSettings->ChannelA;
	}

	return nullptr;
}

void CloudsUI::Draw(const Engine::Ref<BaseShapeWorleySettings>& baseShapeSettings, const Engine::Ref<WorleyPerlinSettings>& perlinSettings)
{
	ImGui::Begin("Settings");
	if (ImGui::Checkbox("Display Texture Viewer", &m_TextureDisplaySettings->EnableTextureViewer))
		m_TextureDisplaySettings->PercentScreenTextureDisplay = m_TextureDisplaySettings->EnableTextureViewer ? 0.1f : 0.0f;

	if (!m_TextureDisplaySettings->EnableTextureViewer)
	{
		ImGui::End();
		return;
	}

	ImGui::DragFloat("Percent Screen Display", &m_TextureDisplaySettings->PercentScreenTextureDisplay, 0.001, 0.0f, 1.0f);


	std::string currentTextureEditor = NameFromUIType(m_ActiveUIType);
	if (ImGui::BeginCombo("Texture Editor Selection", currentTextureEditor.c_str()))
	{
		for (auto editorType : m_TextureDisplaySettings->EditorTypes)
		{
			bool isSelect = editorType == m_ActiveUIType;

			if (ImGui::Selectable(NameFromUIType(editorType).c_str(), isSelect))
				m_ActiveUIType = editorType;
		}
		ImGui::EndCombo();
	}

	if (m_ActiveUIType == CloudUIType::BaseShape)
	{
		std::string currentChannel = NameFromWorleyChannel(m_ActiveChannelMask);
		if (ImGui::BeginCombo("Channel Selection", currentChannel.c_str()))
		{
			for (auto channelType : m_TextureDisplaySettings->ChannelTypes)
			{
				bool isSelect = channelType == m_ActiveChannelMask;

				if (ImGui::Selectable(NameFromWorleyChannel(channelType).c_str(), isSelect))
				{
					m_ActiveChannelMask = channelType;

					if (m_TextureDisplaySettings->DisplaySelectedChannelOnly)
					{
						m_TextureDisplaySettings->EnableGreyScale = false;
						m_TextureDisplaySettings->ShowAlpha = false;
						m_TextureDisplaySettings->DrawAllChannels = false;

						glm::vec4 selectedChannel = ColorFromMask(m_ActiveChannelMask);

						if (selectedChannel.a == 1.0)
						{
							m_TextureDisplaySettings->DisplaySelectedChannelOnly = false;
							m_TextureDisplaySettings->ChannelWeights = selectedChannel;
							m_TextureDisplaySettings->ShowAlpha = true;
						}
						else
						{
							m_TextureDisplaySettings->ChannelWeights = selectedChannel;
						}
					}
				}
			}
			ImGui::EndCombo();
		}

		if (ImGui::Checkbox("Show All Channels", &m_TextureDisplaySettings->DrawAllChannels))
		{
			m_TextureDisplaySettings->EnableGreyScale = false;
			m_TextureDisplaySettings->ShowAlpha = false;
			m_TextureDisplaySettings->DisplaySelectedChannelOnly = false;
		}

		if (ImGui::Checkbox("Show Selected Channel Only", &m_TextureDisplaySettings->DisplaySelectedChannelOnly))
		{
			m_TextureDisplaySettings->EnableGreyScale = false;
			m_TextureDisplaySettings->ShowAlpha = false;
			m_TextureDisplaySettings->DrawAllChannels = false;

			glm::vec4 selectedChannel = ColorFromMask(m_ActiveChannelMask);

			if (selectedChannel.a == 1.0)
			{
				m_TextureDisplaySettings->ShowAlpha = true;
				m_TextureDisplaySettings->DisplaySelectedChannelOnly = false;
			}
			else
			{
				m_TextureDisplaySettings->ChannelWeights = selectedChannel;
			}
		}


		if (!m_TextureDisplaySettings->DrawAllChannels && !m_TextureDisplaySettings->DisplaySelectedChannelOnly)
		{

			if (ImGui::Checkbox("Show Alpha", &m_TextureDisplaySettings->ShowAlpha))
			{
				m_TextureDisplaySettings->ChannelWeights *= glm::vec4(0.0);

				if (m_TextureDisplaySettings->ShowAlpha)
					m_TextureDisplaySettings->EnableGreyScale = false;

				m_TextureDisplaySettings->ChannelWeights.a = m_TextureDisplaySettings->ShowAlpha ? 1.0 : 0.0;

			}
			if (ImGui::Checkbox("Enable Grey Scale", &m_TextureDisplaySettings->EnableGreyScale))
			{
				m_TextureDisplaySettings->ChannelWeights *= glm::vec4(0.0);

				if (m_TextureDisplaySettings->EnableGreyScale)
				{
					m_TextureDisplaySettings->ShowAlpha = false;
					m_TextureDisplaySettings->ChannelWeights += glm::vec4(1.0f);
					m_TextureDisplaySettings->ChannelWeights.a = 0.0f;
				}
			}
			ImGui::DragFloat4("Channel Weights", &m_TextureDisplaySettings->ChannelWeights.x, 0.001, 0.0, 1.0);
		}
		ImGui::DragFloat("Depth Slice", &m_TextureDisplaySettings->DepthSlice, 0.001, 0.0, 1.0);
	}

	ImGui::End();

	if (m_ActiveUIType == CloudUIType::BaseShape)
		DrawBaseShapeUI(baseShapeSettings, perlinSettings);
	else if (m_ActiveUIType == CloudUIType::Perlin)
		DrawPerlinUI(perlinSettings);
}

void CloudsUI::DrawBaseShapeUI(const Engine::Ref<BaseShapeWorleySettings>& baseShapeSettings, const Engine::Ref<WorleyPerlinSettings>& perlinSettings)
{
	Engine::Ref<WorleyChannelData> activeWorleyChannelData = ChannelFromMask(baseShapeSettings);
	if (activeWorleyChannelData == nullptr) return;

	std::string editorName = "Shape Editor: " + NameFromWorleyChannel(activeWorleyChannelData->Mask) + "###";
	ImGui::Begin(editorName.c_str());

	bool updated = false;
	bool updatedPoints = false;

	if (ImGui::Checkbox("Invert", &activeWorleyChannelData->InvertWorley))
		updated = true;
	if (ImGui::DragInt3("Layer Cells", &activeWorleyChannelData->LayerCells.x, 0.1, 1))
	{
		updated = true;
		updatedPoints = true;
	}
	if (ImGui::DragInt3("Layer Seeds", &activeWorleyChannelData->LayerSeeds.x, 0.1))
		updated = true;
	if (ImGui::DragFloat("Persistence", &activeWorleyChannelData->WorleyLayerPersistence, 0.001, 0.01))
		updated = true;
	if (ImGui::DragFloat("Tiling", &activeWorleyChannelData->WorleyTiling, 0.01))
		updated = true;

	uint32_t threadGroups = glm::ceil(baseShapeSettings->ShapeResolution / (float)baseShapeSettings->ShapeThreadGroupSize);

	if (ImGui::Button("Generate New Points") || updatedPoints)
	{
		activeWorleyChannelData->UpdatePoints();
		updated = true;
	}
	if (ImGui::Button("Update Channel") || updated)
		activeWorleyChannelData->UpdateChannel(perlinSettings->PerlinTexture, baseShapeSettings->PerlinWorleyMix, threadGroups);

	ImGui::End();
}

void CloudsUI::DrawPerlinUI(const Engine::Ref<WorleyPerlinSettings>& perlinSettings)
{
	ImGui::Begin("Perlin Editor");
	bool updated = false;
	if (ImGui::DragInt("Octaves", &perlinSettings->Octaves, 0.1, 0, 8))
		updated = true;
	if (ImGui::DragFloat("Perlin Texture Scale", &perlinSettings->NoiseScale, 0.001, 0.01))
		updated = true;
	if (ImGui::DragFloat("Perlin Persistence", &perlinSettings->Persistence, 0.001, 0.01, 1.0f))
		updated = true;
	if (ImGui::DragFloat("Perlin Lacunarity", &perlinSettings->Lacunarity, 0.001, 1.0f))
		updated = true;
	if (ImGui::DragFloat2("Perlin Texture Offset", &perlinSettings->TextureOffset.x, 0.01))
		updated = true;


	if (ImGui::Button("Generate New Offsets"))
	{
		perlinSettings->UpdatePoints();
		updated = true;
	}
	if (ImGui::Button("Update Noise"))
		perlinSettings->UpdateTexture();

	if (updated)
		perlinSettings->UpdateTexture();

	ImGui::End();
}
