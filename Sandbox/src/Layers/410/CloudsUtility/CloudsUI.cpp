#include "Layers/410/CloudsUtility/CloudsUI.h"
#include "Layers/410/CloudsUtility/HelperFunctions.h"

#include <imgui/imgui.h>

CloudsUI::CloudsUI()
{
	m_MainTextureDebugSettings = Engine::CreateRef<TextureDebugDisplaySettings>();
	m_DetailTextureDisplaySettings = Engine::CreateRef<DetailTextureDebugDisplaySettings>();
	m_ShapeTextureDisplaySettings = Engine::CreateRef<ShapeTextureDebugDisplaySettings>();
}

CloudsUI::~CloudsUI()
{

}

const Engine::Ref<WorleyChannelData>& CloudsUI::ShapeChannelFromMask(const Engine::Ref<BaseShapeWorleySettings>& baseShapeSettings)
{
	switch (m_ActiveShapeMask)
	{
	case WorleyChannelMask::R: return baseShapeSettings->ChannelR;
	case WorleyChannelMask::G: return baseShapeSettings->ChannelG;
	case WorleyChannelMask::B: return baseShapeSettings->ChannelB;
	case WorleyChannelMask::A: return baseShapeSettings->ChannelA;
	}

	return nullptr;
}

const Engine::Ref<WorleyChannelData>& CloudsUI::DetailChannelFromMask(const Engine::Ref<DetailShapeWorleySettings>& detailShapeSettings)
{
	switch (m_ActiveDetailMask)
	{
	case WorleyChannelMask::R: return detailShapeSettings->ChannelR;
	case WorleyChannelMask::G: return detailShapeSettings->ChannelG;
	case WorleyChannelMask::B: return detailShapeSettings->ChannelB;
	}

	return nullptr;
}

void CloudsUI::DrawBaseShapeSelectionUI()
{
	std::string currentChannel = NameFromWorleyChannel(m_ActiveShapeMask);
	if (ImGui::BeginCombo("Channel Selection", currentChannel.c_str()))
	{
		for (auto channelType : m_ShapeTextureDisplaySettings->ChannelTypes)
		{
			bool isSelect = channelType == m_ActiveShapeMask;

			if (ImGui::Selectable(NameFromWorleyChannel(channelType).c_str(), isSelect))
			{
				m_ActiveShapeMask = channelType;

				if (m_ShapeTextureDisplaySettings->DisplaySelectedChannelOnly)
				{
					m_ShapeTextureDisplaySettings->EnableGreyScale = false;
					m_ShapeTextureDisplaySettings->ShowAlpha = false;
					m_ShapeTextureDisplaySettings->DrawAllChannels = false;

					glm::vec4 selectedChannel = ColorFromMask(m_ActiveShapeMask);

					if (selectedChannel.a == 1.0)
					{
						m_ShapeTextureDisplaySettings->DisplaySelectedChannelOnly = false;
						m_ShapeTextureDisplaySettings->ChannelWeights = selectedChannel;
						m_ShapeTextureDisplaySettings->ShowAlpha = true;
					}
					else
					{
						m_ShapeTextureDisplaySettings->ChannelWeights = selectedChannel;
					}
				}
			}
		}
		ImGui::EndCombo();
	}

	if (ImGui::Checkbox("Show All Channels", &m_ShapeTextureDisplaySettings->DrawAllChannels))
	{
		m_ShapeTextureDisplaySettings->EnableGreyScale = false;
		m_ShapeTextureDisplaySettings->ShowAlpha = false;
		m_ShapeTextureDisplaySettings->DisplaySelectedChannelOnly = false;
	}

	if (ImGui::Checkbox("Show Selected Channel Only", &m_ShapeTextureDisplaySettings->DisplaySelectedChannelOnly))
	{
		m_ShapeTextureDisplaySettings->EnableGreyScale = false;
		m_ShapeTextureDisplaySettings->ShowAlpha = false;
		m_ShapeTextureDisplaySettings->DrawAllChannels = false;

		glm::vec4 selectedChannel = ColorFromMask(m_ActiveShapeMask);

		if (selectedChannel.a == 1.0)
		{
			m_ShapeTextureDisplaySettings->ShowAlpha = true;
			m_ShapeTextureDisplaySettings->DisplaySelectedChannelOnly = false;
		}
		else
		{
			m_ShapeTextureDisplaySettings->ChannelWeights = selectedChannel;
		}
	}

	if (!m_ShapeTextureDisplaySettings->DrawAllChannels && !m_ShapeTextureDisplaySettings->DisplaySelectedChannelOnly)
	{
		if (ImGui::Checkbox("Show Alpha", &m_ShapeTextureDisplaySettings->ShowAlpha))
		{
			m_ShapeTextureDisplaySettings->ChannelWeights *= glm::vec4(0.0);

			if (m_ShapeTextureDisplaySettings->ShowAlpha)
				m_ShapeTextureDisplaySettings->EnableGreyScale = false;

			m_ShapeTextureDisplaySettings->ChannelWeights.a = m_ShapeTextureDisplaySettings->ShowAlpha ? 1.0 : 0.0;

		}
		if (ImGui::Checkbox("Enable Grey Scale", &m_ShapeTextureDisplaySettings->EnableGreyScale))
		{
			m_ShapeTextureDisplaySettings->ChannelWeights *= glm::vec4(0.0);

			if (m_ShapeTextureDisplaySettings->EnableGreyScale)
			{
				m_ShapeTextureDisplaySettings->ShowAlpha = false;
				m_ShapeTextureDisplaySettings->ChannelWeights += glm::vec4(1.0f);
				m_ShapeTextureDisplaySettings->ChannelWeights.a = 0.0f;
			}
		}
		ImGui::DragFloat4("Channel Weights", &m_ShapeTextureDisplaySettings->ChannelWeights.x, 0.001, 0.0, 1.0);
	}
	ImGui::DragFloat("Depth Slice", &m_ShapeTextureDisplaySettings->DepthSlice, 0.001, 0.0, 1.0);
}


void CloudsUI::DrawDetailShapeSelectionUI()
{
	std::string currentChannel = NameFromWorleyChannel(m_ActiveShapeMask);
	if (ImGui::BeginCombo("Channel Selection", currentChannel.c_str()))
	{
		for (auto channelType : m_DetailTextureDisplaySettings->ChannelTypes)
		{
			bool isSelect = channelType == m_ActiveDetailMask;

			if (ImGui::Selectable(NameFromWorleyChannel(channelType).c_str(), isSelect))
			{
				m_ActiveDetailMask = channelType;

				if (m_DetailTextureDisplaySettings->DisplaySelectedChannelOnly)
				{
					m_DetailTextureDisplaySettings->EnableGreyScale = false;
					m_DetailTextureDisplaySettings->DrawAllChannels = false;

					glm::vec4 selectedChannel = ColorFromMask(m_ActiveDetailMask);
					m_DetailTextureDisplaySettings->ChannelWeights = selectedChannel;
				}
			}
		}
		ImGui::EndCombo();
	}

	if (ImGui::Checkbox("Show All Channels", &m_DetailTextureDisplaySettings->DrawAllChannels))
	{
		m_DetailTextureDisplaySettings->EnableGreyScale = false;
		m_DetailTextureDisplaySettings->DisplaySelectedChannelOnly = false;
	}

	if (ImGui::Checkbox("Show Selected Channel Only", &m_DetailTextureDisplaySettings->DisplaySelectedChannelOnly))
	{
		m_DetailTextureDisplaySettings->EnableGreyScale = false;
		m_DetailTextureDisplaySettings->DrawAllChannels = false;

		glm::vec4 selectedChannel = ColorFromMask(m_ActiveDetailMask);

		m_DetailTextureDisplaySettings->ChannelWeights = selectedChannel;
	}

	if (!m_DetailTextureDisplaySettings->DrawAllChannels && !m_DetailTextureDisplaySettings->DisplaySelectedChannelOnly)
	{
		if (ImGui::Checkbox("Enable Grey Scale", &m_DetailTextureDisplaySettings->EnableGreyScale))
		{
			if (m_DetailTextureDisplaySettings->EnableGreyScale)
				m_DetailTextureDisplaySettings->ChannelWeights = glm::vec4(1.0f);
		}
		ImGui::DragFloat3("Channel Weights", &m_DetailTextureDisplaySettings->ChannelWeights.x, 0.001, 0.0, 1.0);
	}
	ImGui::DragFloat("Depth Slice", &m_DetailTextureDisplaySettings->DepthSlice, 0.001, 0.0, 1.0);
}

void CloudsUI::Draw(const Engine::Ref<CloudsUIData>& uiData)
{
	ImGui::Begin("Settings");
	if (ImGui::Checkbox("Display Texture Viewer", &m_MainTextureDebugSettings->EnableTextureViewer))
		m_MainTextureDebugSettings->PercentScreenTextureDisplay = m_MainTextureDebugSettings->EnableTextureViewer ? 0.1f : 0.0f;

	if (!m_MainTextureDebugSettings->EnableTextureViewer)
	{
		ImGui::End();
		return;
	}

	ImGui::DragFloat("Percent Screen Display", &m_MainTextureDebugSettings->PercentScreenTextureDisplay, 0.001, 0.0f, 1.0f);

	std::string currentTextureEditor = NameFromUIType(m_ActiveUIType);
	if (ImGui::BeginCombo("Texture Editor Selection", currentTextureEditor.c_str()))
	{
		for (auto editorType : m_MainTextureDebugSettings->EditorTypes)
		{
			bool isSelect = editorType == m_ActiveUIType;

			if (ImGui::Selectable(NameFromUIType(editorType).c_str(), isSelect))
			{
				m_ActiveUIType = editorType;
				m_ActiveDebugShapeType = m_ActiveUIType == CloudUIType::Perlin ? ActiveDebugShapeType::None : (m_ActiveUIType == CloudUIType::BaseShape ? ActiveDebugShapeType::BaseShape : ActiveDebugShapeType::DetailNoise);
			}
		}
		ImGui::EndCombo();
	}

	if (m_ActiveUIType == CloudUIType::BaseShape)
		DrawBaseShapeSelectionUI();
	if (m_ActiveUIType == CloudUIType::DetailShape)
		DrawDetailShapeSelectionUI();

	ImGui::End();

	if (m_ActiveUIType == CloudUIType::BaseShape)
		DrawBaseShapeUI(uiData->BaseShapeSettings, uiData->PerlinSettings);
	else if (m_ActiveUIType == CloudUIType::DetailShape)
		DrawDetailShapeUI(uiData->DetailShapeSettings);
	else if (m_ActiveUIType == CloudUIType::Perlin)
		DrawPerlinUI(uiData->PerlinSettings);

	DrawCloudSettingsUI(uiData->MainCloudSettings, uiData->AnimationSettings, uiData->SceneRenderPass);
}

void CloudsUI::DrawCloudSettingsUI(const Engine::Ref<CloudSettings>& cloudSettings, const Engine::Ref<CloudAnimationSettings>& animationSettings, const Engine::Ref<CloudsSceneRenderPass>& sceneRenderPass)
{
	ImGui::Begin("Cloud Settings");

	ImGui::DragFloat3("Sun Position", &sceneRenderPass->GetSunLight()->GetLightTransform()->GetPosition().x, 0.1);
	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
	ImGui::ColorEdit3("Sun Color", &sceneRenderPass->GetSunLight()->GetLightColor().r, ImGuiColorEditFlags_NoInputs);
	ImGui::PopStyleVar();
	ImGui::DragFloat("Sun Intensity", &sceneRenderPass->GetSunLight()->GetLightIntensity(), 0.1);

	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
	ImGui::ColorEdit3("Sky Color A", &cloudSettings->SkyColorA.r, ImGuiColorEditFlags_NoInputs);
	ImGui::PopStyleVar();
	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
	ImGui::ColorEdit3("Sky Color B", &cloudSettings->SkyColorB.r, ImGuiColorEditFlags_NoInputs);
	ImGui::PopStyleVar();
	ImGui::Separator();

	ImGui::DragFloat3("Cloud Container Position", &cloudSettings->CloudContainerPosition.x, 0.1);
	ImGui::DragFloat3("Cloud Container Scale", &cloudSettings->CloudContainerScale.x, 0.1);
	ImGui::DragFloat("Container Edge Fade Distance", &cloudSettings->ContainerEdgeFadeDistance, 0.1);

	ImGui::Separator();
	if (ImGui::DragInt("Density Steps", &cloudSettings->DensitySteps, 0.1, 1))
		if (cloudSettings->DensitySteps < 1) cloudSettings->DensitySteps = 1;
	if (ImGui::DragInt("Light Steps", &cloudSettings->LightSteps, 0.1, 1))
		if (cloudSettings->LightSteps < 1) cloudSettings->LightSteps = 1;

	ImGui::Checkbox("Animate Clouds", &animationSettings->AnimateClouds);
	ImGui::DragFloat("Animation Speed", &animationSettings->AnimationSpeed, 0.1);
	ImGui::DragFloat("Time Scale", &animationSettings->TimeScale, 0.001);
	ImGui::DragFloat("Cloud Scroll Offset Speed", &animationSettings->CloudScrollOffsetSpeed, 0.1f);
	ImGui::DragFloat3("Shape Texture Offset", &animationSettings->ShapeTextureOffset.x, 1.0f);

	ImGui::DragFloat("Density Multiplier", &cloudSettings->DensityMultiplier, 0.001, 0.0);
	ImGui::DragFloat("Density Threshold", &cloudSettings->DensityThreshold, 0.001, 0.0);
	ImGui::DragFloat("Powder Constant", &cloudSettings->PowderConstant, 0.001, 0.0);
	ImGui::DragFloat("Phase Blend", &cloudSettings->PhaseBlend, 0.001, 0.0);
	ImGui::DragFloat4("Phase Params", &cloudSettings->PhaseParams.x, 0.001, 0.0);
	ImGui::DragFloat("Silver Lining Constant", &cloudSettings->SilverLiningConstant, 0.001, 0.0);
	ImGui::DragFloat("Cloud Scale", &cloudSettings->CloudScale, 0.01, 0.0);
	ImGui::DragFloat4("Shape Noise Weights", &cloudSettings->ShapeNoiseWeights.x, 0.01, 0.0f, 1.0f);
	ImGui::DragFloat3("Detail Noise Weights", &cloudSettings->DetailNoiseWeights.x, 0.01, 0.0f, 1.0f);
	ImGui::DragFloat("Detail Noise Weight", &cloudSettings->DetailNoiseWeight, 0.01, 0.0);

	ImGui::End();
}

void CloudsUI::DrawBaseShapeUI(const Engine::Ref<BaseShapeWorleySettings>& baseShapeSettings, const Engine::Ref<WorleyPerlinSettings>& perlinSettings)
{
	Engine::Ref<WorleyChannelData> activeWorleyChannelData = ShapeChannelFromMask(baseShapeSettings);
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

	if (activeWorleyChannelData->Mask == WorleyChannelMask::R)
	{
		if (ImGui::DragFloat("Perlin Worley Mix", &baseShapeSettings->PerlinWorleyMix, 0.01))
			updated = true;
	}

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

void CloudsUI::DrawDetailShapeUI(const Engine::Ref<DetailShapeWorleySettings>& detailShapeSettings)
{
	Engine::Ref<WorleyChannelData> activeDetailChannelData = DetailChannelFromMask(detailShapeSettings);
	if (activeDetailChannelData == nullptr) return;

	std::string editorName = "Detail Editor: " + NameFromWorleyChannel(activeDetailChannelData->Mask) + "###";
	ImGui::Begin(editorName.c_str());

	bool updated = false;
	bool updatedPoints = false;

	if (ImGui::Checkbox("Invert", &activeDetailChannelData->InvertWorley))
		updated = true;
	if (ImGui::DragInt3("Layer Cells", &activeDetailChannelData->LayerCells.x, 0.1, 1))
	{
		updated = true;
		updatedPoints = true;
	}
	if (ImGui::DragInt3("Layer Seeds", &activeDetailChannelData->LayerSeeds.x, 0.1))
		updated = true;
	if (ImGui::DragFloat("Persistence", &activeDetailChannelData->WorleyLayerPersistence, 0.001, 0.01))
		updated = true;
	if (ImGui::DragFloat("Tiling", &activeDetailChannelData->WorleyTiling, 0.01))
		updated = true;


	uint32_t threadGroups = glm::ceil(detailShapeSettings->DetailResolution / (float)detailShapeSettings->DetailThreadGroupSize);

	if (ImGui::Button("Generate New Points") || updatedPoints)
	{
		activeDetailChannelData->UpdatePoints();
		updated = true;
	}
	if (ImGui::Button("Update Channel") || updated)
		activeDetailChannelData->UpdateChannel(threadGroups);

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
	if (ImGui::DragFloat("Perlin Lacunarity", &perlinSettings->Lacunarity, 0.25f, 1.0f))
		updated = true;
	if (ImGui::DragFloat2("Perlin Texture Offset", &perlinSettings->TextureOffset.x, 10.0f))
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

