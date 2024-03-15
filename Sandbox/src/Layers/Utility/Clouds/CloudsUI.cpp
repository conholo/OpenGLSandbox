#include "CloudsUI.h"
#include "HelperFunctions.h"

#include <imgui.h>

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
	std::string currentChannel = NameFromWorleyChannel(m_ActiveDetailMask);
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
	ImGui::Begin("Editor");

	if (ImGui::BeginTabBar("Scene Settings"))
	{
		for (uint32_t i = 0; i < m_AvailableTabTypes.size(); i++)
		{
			if (ImGui::BeginTabItem(NameFromUITabType(m_AvailableTabTypes[i]).c_str()))
			{
				m_ActiveTabType = m_AvailableTabTypes[i];

				if (m_ActiveTabType == UITabTypes::MainSettings)
					DrawMainSettings(uiData);
				else if (m_ActiveTabType == UITabTypes::CloudSettings)
					DrawCloudSettingsUI(uiData->MainCloudSettings, uiData->AnimationSettings, uiData->SceneRenderPass);
				else if (m_ActiveTabType == UITabTypes::NoiseTextureSettings)
				{
					DrawNoiseEditorSettings();
					if (m_ActiveUIType == CloudUIType::BaseShape)
						DrawBaseShapeUI(uiData->BaseShapeSettings, uiData->PerlinSettings);
					else if (m_ActiveUIType == CloudUIType::DetailShape)
						DrawDetailShapeUI(uiData->DetailShapeSettings);
					else if (m_ActiveUIType == CloudUIType::Perlin)
						DrawPerlinUI(uiData->PerlinSettings);
					else if (m_ActiveUIType == CloudUIType::Curl)
						DrawCurlUI(uiData->CloudCurlSettings);
				}
				else if (m_ActiveTabType == UITabTypes::TerrainSettings)
					DrawTerrainSettingsUI(uiData->SceneRenderPass, uiData->SceneRenderPass->GetTerrain(), uiData->SceneRenderPass->GetTerrainLOD(), uiData->SceneRenderPass->GetTerrainIsWireframe());
				else if (m_ActiveTabType == UITabTypes::WaterSettings)
					DrawWaterUI(uiData->WaterSettings);

				ImGui::EndTabItem();
			}
		}

		ImGui::EndTabBar();
	}

	ImGui::End();


}

void CloudsUI::DrawMainSettings(const Engine::Ref<CloudsUIData>& uiData)
{
	ImGui::Checkbox("Draw Clouds", &uiData->MainCloudSettings->DrawClouds);
	ImGui::Checkbox("Draw Terrain", uiData->SceneRenderPass->GetDrawTerrain());
	ImGui::Checkbox("Draw Water", &uiData->WaterSettings->DrawWater);
}

void CloudsUI::DrawNoiseEditorSettings()
{
	if (ImGui::Checkbox("Display Texture Viewer", &m_MainTextureDebugSettings->EnableTextureViewer))
		m_MainTextureDebugSettings->PercentScreenTextureDisplay = m_MainTextureDebugSettings->EnableTextureViewer ? 0.1f : 0.0f;

	if (!m_MainTextureDebugSettings->EnableTextureViewer)
		return;

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
				m_ActiveDebugShapeType = m_ActiveUIType == CloudUIType::Perlin || m_ActiveUIType == CloudUIType::Curl ? ActiveDebugShapeType::None : (m_ActiveUIType == CloudUIType::BaseShape ? ActiveDebugShapeType::BaseShape : ActiveDebugShapeType::DetailNoise);
			}
		}
		ImGui::EndCombo();
	}

	if (m_ActiveUIType == CloudUIType::BaseShape)
		DrawBaseShapeSelectionUI();
	if (m_ActiveUIType == CloudUIType::DetailShape)
		DrawDetailShapeSelectionUI();
}

void CloudsUI::DrawCloudSettingsUI(const Engine::Ref<CloudSettings>& cloudSettings, const Engine::Ref<CloudAnimationSettings>& animationSettings, const Engine::Ref<CloudsSceneRenderPass>& sceneRenderPass)
{
	if (ImGui::TreeNodeEx("Sun/Sky Settings"))
	{
		ImGui::DragFloat("Sun Size", &cloudSettings->SunSize, 0.01, 0.0);
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

		ImGui::TreePop();
	}

	if (ImGui::TreeNodeEx("Cloud Scale/Container Settings"))
	{
		ImGui::DragFloat3("Cloud Container Position", &cloudSettings->CloudContainerPosition.x, 0.1);
		ImGui::DragFloat3("Cloud Container Scale", &cloudSettings->CloudContainerScale.x, 0.1);
		ImGui::DragFloat("Container Edge Fade Distance", &cloudSettings->ContainerEdgeFadeDistance, 0.1);
		ImGui::DragFloat("Cloud Scale", &cloudSettings->CloudScale, 0.01, 0.0);
		ImGui::DragFloat("Cloud Scale Factor", &cloudSettings->CloudScaleFactor, 0.1);
		ImGui::TreePop();
	}

	if (ImGui::TreeNodeEx("Animation Settings"))
	{
		ImGui::Checkbox("Animate Clouds", &animationSettings->AnimateClouds);
		ImGui::DragFloat("Animation Speed", &animationSettings->AnimationSpeed, 0.1);
		ImGui::DragFloat("Time Scale", &animationSettings->TimeScale, 0.001);
		ImGui::DragFloat("Cloud Scroll Offset Speed", &animationSettings->CloudScrollOffsetSpeed, 0.1f);
		ImGui::DragFloat3("Shape Texture Offset", &animationSettings->ShapeTextureOffset.x, 1.0f);
		ImGui::TreePop();
	}

	if (ImGui::TreeNodeEx("Density Settings"))
	{
		if (ImGui::DragInt("Density Steps", &cloudSettings->DensitySteps, 0.1, 1))
			if (cloudSettings->DensitySteps < 1) cloudSettings->DensitySteps = 1;
		ImGui::DragFloat("Random Ray Offset", &cloudSettings->RandomOffsetStrength, 0.01, 0.0);
		ImGui::DragFloat("Curl Intensity", &cloudSettings->CurlIntensity, 0.01);
		ImGui::DragFloat("Density Multiplier", &cloudSettings->DensityMultiplier, 0.001, 0.0);
		ImGui::DragFloat("Density Threshold", &cloudSettings->DensityThreshold, 0.001, 0.0);
		ImGui::DragFloat4("Shape Noise Weights", &cloudSettings->ShapeNoiseWeights.x, 0.01, 0.0f, 1.0f);
		ImGui::DragFloat3("Detail Noise Weights", &cloudSettings->DetailNoiseWeights.x, 0.01, 0.0f, 1.0f);
		ImGui::DragFloat("Detail Noise Weight", &cloudSettings->DetailNoiseWeight, 0.01, 0.0);
		ImGui::DragFloat3("Cloud Type Weights", &cloudSettings->CloudTypeWeights.x, 0.01, 0.0f, 1.0f);
		ImGui::DragFloat("Cloud Type Weight Strength", &cloudSettings->CloudTypeWeightStrength, 0.01, 0.0);

		ImGui::TreePop();
	}

	if (ImGui::TreeNodeEx("Lighting Settings"))
	{
		if (ImGui::DragInt("Light Steps", &cloudSettings->LightSteps, 0.1, 1))
			if (cloudSettings->LightSteps < 1) cloudSettings->LightSteps = 1;
		ImGui::DragFloat("Powder Constant", &cloudSettings->PowderConstant, 0.001, 0.0);
		ImGui::DragFloat("Silver Lining Constant", &cloudSettings->SilverLiningConstant, 0.001, 0.0);
		ImGui::DragFloat("Extinction Factor", &cloudSettings->ExtinctionFactor, 0.01);
		ImGui::DragFloat("Phase Blend", &cloudSettings->PhaseBlend, 0.001, 0.0);
		ImGui::DragFloat("Forward Scattering", &cloudSettings->ForwardScattering, 0.001, 0.0, 1.0);
		ImGui::DragFloat("Back Scattering", &cloudSettings->BackScattering, 0.001, 0.0, 1.0);
		ImGui::DragFloat("Base Brightness", &cloudSettings->BaseBrightness, 0.001, 0.0);
		ImGui::DragFloat("Phase Factor", &cloudSettings->PhaseFactor, 0.001, 0.0, 1.0);
		ImGui::TreePop();
	}
}

void CloudsUI::DrawBaseShapeUI(const Engine::Ref<BaseShapeWorleySettings>& baseShapeSettings, const Engine::Ref<WorleyPerlinSettings>& perlinSettings)
{
	Engine::Ref<WorleyChannelData> activeWorleyChannelData = ShapeChannelFromMask(baseShapeSettings);
	if (activeWorleyChannelData == nullptr) return;

	bool updated = false;
	bool updatedPoints = false;

	if (ImGui::Checkbox("Invert", &activeWorleyChannelData->InvertWorley))
		updated = true;

	if (activeWorleyChannelData->InvertWorley)
		if (ImGui::DragFloat("Inversion Weight", &activeWorleyChannelData->InversionWeight, 0.01, 0.0, 1.0))
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

	if (ImGui::Button("Generate New Points") || updatedPoints)
	{
		activeWorleyChannelData->UpdatePoints();
		updated = true;
	}
	if (ImGui::Button("Update Channel") || updated)
		baseShapeSettings->UpdateChannel(m_ActiveShapeMask, perlinSettings->PerlinTexture);
}

void CloudsUI::DrawDetailShapeUI(const Engine::Ref<DetailShapeWorleySettings>& detailShapeSettings)
{
	Engine::Ref<WorleyChannelData> activeDetailChannelData = DetailChannelFromMask(detailShapeSettings);
	if (activeDetailChannelData == nullptr) return;

	bool updated = false;
	bool updatedPoints = false;

	if (ImGui::Checkbox("Invert", &activeDetailChannelData->InvertWorley))
		updated = true;
	if (activeDetailChannelData->InvertWorley)
		if (ImGui::DragFloat("Inversion Weight", &activeDetailChannelData->InversionWeight, 0.01, 0.0, 1.0))
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

	if (ImGui::Button("Generate New Points") || updatedPoints)
	{
		activeDetailChannelData->UpdatePoints();
		updated = true;
	}
	if (ImGui::Button("Update Channel") || updated)
		detailShapeSettings->UpdateChannel(m_ActiveDetailMask);
}

void CloudsUI::DrawPerlinUI(const Engine::Ref<WorleyPerlinSettings>& perlinSettings)
{
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
}

void CloudsUI::DrawCurlUI(const Engine::Ref<CurlSettings>& curlSettings)
{
	bool updated = false;
	if (ImGui::DragFloat("Strength", &curlSettings->Strength, 0.001f))
		updated = true;
	if (ImGui::DragFloat("Tiling", &curlSettings->Tiling, 0.001, 0.01))
		updated = true;
	if (ImGui::DragFloat2("Texture Offset", &curlSettings->TilingOffset.x, 1.0f))
		updated = true;
	if (ImGui::Button("Update Noise") || updated)
		curlSettings->UpdateTexture();
}

void CloudsUI::DrawWaterUI(const Engine::Ref<WaterData>& waterSettings)
{
	ImGui::DragFloat("Amplitude", &waterSettings->SeaAmplitude, 0.01f);
	ImGui::DragFloat("Frequency", &waterSettings->SeaFrequency, 0.01f);
	ImGui::DragFloat("Choppy", &waterSettings->SeaChoppy, 0.01f);
	ImGui::DragFloat("Height", &waterSettings->SeaHeight, 0.01f);
	ImGui::DragInt("Octaves", (int*)(&waterSettings->OceanOctaves), 0.01f);
	ImGui::DragInt("Steps", (int*)(&waterSettings->OceanSteps), 0.01f);
}

void CloudsUI::DrawTerrainSettingsUI(const Engine::Ref<CloudsSceneRenderPass>& scenePass, const Engine::Ref<Engine::Terrain>& terrain, int* terrainLOD, bool* wireFrame)
{
	bool updated = false;

	std::string minText = "Min Height: " + std::to_string(terrain->GetMinHeightLocalSpace());
	std::string maxText = "Max Height: " + std::to_string(terrain->GetMaxHeightLocalSpace());

	ImGui::Text(minText.c_str());
	ImGui::Text(maxText.c_str());
	ImGui::Checkbox("Wireframe Mode", wireFrame);

	if (ImGui::TreeNode("Detail Parameters"))
	{
		ImGui::Text(("Resolution: " + std::to_string(terrain->GetResolution())).c_str());
		ImGui::Text(("LOD: " + std::to_string(terrain->GetLOD())).c_str());

		if (ImGui::DragInt("LOD", terrainLOD, 0.01, terrain->GetMaxLOD() - 3, terrain->GetMinLOD() - 3))
		{
			terrain->SetLOD(*terrainLOD);
			updated = true;
		}

		ImGui::DragFloat3("Terrain Color", &terrain->GetProperties()->Color.x, 0.01);

		if (ImGui::DragFloat3("Terrain Size", &terrain->GetProperties()->Scale.x, 0.1))
		{
			terrain->GetTransform()->SetScale(terrain->GetProperties()->Scale);
			updated = true;
		}

		if (ImGui::DragFloat3("Terrain Position", &terrain->GetProperties()->Position.x, 0.1))
		{
			terrain->GetTransform()->SetPosition(terrain->GetProperties()->Position);
			updated = true;
		}

		ImGui::TreePop();
	}

	if (ImGui::TreeNodeEx("Terrain Noise Settings"))
	{
		if (ImGui::DragInt("Octaves##", &terrain->GetProperties()->NoiseSettings->Octaves, 0.01))
			updated = true;
		if (ImGui::DragFloat("Height Scale Factor##", &terrain->GetProperties()->HeightScaleFactor, 0.1))
			updated = true;
		if (ImGui::DragFloat("Height Threshold##", &terrain->GetProperties()->HeightThreshold, 0.01))
			updated = true;
		if (ImGui::DragFloat("Noise Scale##", &terrain->GetProperties()->NoiseSettings->NoiseScale, 0.01))
			updated = true;
		if (ImGui::DragFloat("Lacunarity##", &terrain->GetProperties()->NoiseSettings->Lacunarity, 0.01, 1.0f))
			updated = true;
		if (ImGui::DragFloat("Persistence##", &terrain->GetProperties()->NoiseSettings->Persistence, 0.01, 0.0f, 1.0f))
			updated = true;
		if (ImGui::DragFloat2("Offsets##", &terrain->GetProperties()->NoiseSettings->TextureOffset.x, 0.1f))
			updated = true;

		ImGui::TreePop();
	}

	if (ImGui::TreeNodeEx("Terrain Layer Settings"))
	{
		for (uint32_t i = 0; i < scenePass->GetTerrainHeightLayers().size(); i++)
		{
			std::string layerName = "Layer " + std::to_string(i);
			if (ImGui::TreeNodeEx(layerName.c_str()))
			{
				ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
				ImGui::ColorEdit3("Tint Color", &scenePass->GetTerrainHeightLayers()[i]->TintColor.r, ImGuiColorEditFlags_NoInputs);
				ImGui::PopStyleVar();
				ImGui::Image((void*)scenePass->GetTerrainHeightLayers()[i]->HeightTexture->GetID(), {25, 25}, { 0, 1 }, { 1, 0 });

				ImGui::Separator();
				ImGui::DragFloat("Height Threshold##", &scenePass->GetTerrainHeightLayers()[i]->HeightThreshold, 0.01f, -1.0f, 1.0f);
				ImGui::DragFloat("Blend Weight##", &scenePass->GetTerrainHeightLayers()[i]->BlendStrength, 0.01f, 0.0f, 1.0f);
				ImGui::DragFloat("Texture Tiling##", &scenePass->GetTerrainHeightLayers()[i]->TextureTiling, 0.01f);
				ImGui::Separator();

				ImGui::TreePop();
			}
		}

		ImGui::TreePop();
	}

	if (updated)
		terrain->UpdateTerrain();
}

