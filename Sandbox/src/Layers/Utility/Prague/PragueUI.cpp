#include "PragueUI.h"

#include <iostream>

void helpMarker(const char* desc)
{
	ImGui::TextDisabled("(?)");
	if (ImGui::IsItemHovered())
	{
		ImGui::BeginTooltip();
		ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
		ImGui::TextUnformatted(desc);
		ImGui::PopTextWrapPos();
		ImGui::EndTooltip();
	}
}

void errorMarker(const char* desc)
{
	ImGui::Text("ERROR!");
	if (ImGui::IsItemHovered())
	{
		ImGui::BeginTooltip();
		ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
		ImGui::TextUnformatted(desc);
		ImGui::PopTextWrapPos();
		ImGui::EndTooltip();
	}
}

PragueUI::PragueUI()
{
	m_FileBrowser.SetTitle("Select dataset file");
	m_FileBrowser.SetTypeFilters({ ".dat" });
	std::filesystem::path assetPath = "../assets/data/";
	m_FileBrowser.SetPwd(assetPath);
}


void PragueUI::Begin()
{
	ImGui::Begin("Prague Sky Model");
}

void PragueUI::End()
{
	ImGui::End();
}

void PragueUI::DrawDatasetConfiguration(const LoadDataSetFn& loadFn)
{
	static std::string loadError = "";
	static bool        loaded = false;
	static bool        loading = false;
	static std::string datasetName = "SkyModelDataset.dat";
	static std::string datasetPath = "SkyModelDataset.dat";
	static float       visibility = 59.4f;
	static int         visibilityToLoad = 0;


	ImGui::SetNextWindowPos(ImVec2(0, 0));

	ImGui::Text("Dataset:");

	// Dataset file selection
	if (ImGui::Button(datasetName.c_str(), ImVec2(ImGui::CalcItemWidth(), 20)))
		m_FileBrowser.Open();

	ImGui::SameLine();
	ImGui::Text("file");
	ImGui::SameLine();
	helpMarker("Sky model dataset file");
	m_FileBrowser.Display();
	if (m_FileBrowser.HasSelected())
	{
		datasetPath = m_FileBrowser.GetSelected().string();
		datasetName = datasetPath.substr(datasetPath.find_last_of("\\") + 1);
		m_FileBrowser.ClearSelected();
	}
	const char* visibilitiesToLoad[] = { "Everything",
									 "only visibilities 20.0 - 27.6 km",
									 "only visibilities 27.6 - 40.0 km",
									 "only visibilities 40.0 - 59.4 km",
									 "only visibilities 59.4 - 90.0 km",
									 "only visibilities 90.0 - 131.8 km" };

	// Selection of visibilities to load
	ImGui::Combo("part to load", &visibilityToLoad, visibilitiesToLoad, IM_ARRAYSIZE(visibilitiesToLoad));
	ImGui::SameLine();
	helpMarker("What portion of the dataset should be loaded");
	// Load button
	if (loading)
	{
		try
		{
			double singleVisibility = 0.0;
			switch (visibilityToLoad)
			{
			case 1: // 20.0 - 27.6
				singleVisibility = 23.8;
				break;
			case 2: // 27.6 - 40.0
				singleVisibility = 33.8;
				break;
			case 3: // 40.0 - 59.4
				singleVisibility = 49.7;
				break;
			case 4: // 59.4 - 90.0
				singleVisibility = 74.7;
				break;
			case 5: // 90.0 - 131.8
				singleVisibility = 110.9;
				break;
			default:
				singleVisibility = 0.0;
				break;
			}
			loadFn(datasetPath, singleVisibility);
		}
		catch (std::exception& e)
		{
			loadError = e.what();
			loaded = false;
		}
		loading = false;
	}
	if (ImGui::Button("Load"))
	{
		loading = true;
		ImGui::SameLine();
		ImGui::Text("Loading ...");
	}
	if (loaded && !loading)
	{
		ImGui::SameLine();
		ImGui::Text("OK");
	}
	else if (!loadError.empty() && !loading)
	{
		ImGui::SameLine();
		errorMarker(loadError.c_str());
	}

	// Dataset section end
	ImGui::Dummy(ImVec2(0.0f, 1.0f));
	ImGui::Separator();

}

void PragueUI::DrawInputUI(PragueInput* input, const PragueSkyModel::AvailableData& data, bool* valueChanged)
{
	char label[150];
	const PragueSkyModel::AvailableData available = data;

	*valueChanged |= ImGui::SliderFloat("Albedo", &input->Albedo, available.albedoMin, available.albedoMax, "%.2f", ImGuiSliderFlags_AlwaysClamp);
	ImGui::SameLine();
	sprintf(label, "Ground albedo, value in range [%.1f, %.1f]", available.albedoMin, available.albedoMax);
	helpMarker(label);

	*valueChanged |= ImGui::SliderFloat("altitude", &input->Altitude, available.altitudeMin, available.altitudeMax, "%.0f m", ImGuiSliderFlags_AlwaysClamp);
	ImGui::SameLine();
	sprintf(label, "Altitude of view point in meters, value in range [%.1f, %.1f]", available.altitudeMin, available.altitudeMax);
	helpMarker(label);

	*valueChanged |= ImGui::SliderAngle("azimuth", &input->Azimuth, 0.0f, 360.0f, "%.1f deg", ImGuiSliderFlags_AlwaysClamp);
	ImGui::SameLine();
	helpMarker("Sun azimuth at view point in degrees, value in range [0, 360]");

	*valueChanged |= ImGui::SliderAngle("elevation", &input->Elevation, available.elevationMin, available.elevationMax, "%.1f deg", ImGuiSliderFlags_AlwaysClamp);
	ImGui::SameLine();
	sprintf(label, "Sun elevation at view point in degrees, value in range [%.1f, %.1f]", available.elevationMin, available.elevationMax);
	helpMarker(label);

	*valueChanged |= ImGui::SliderFloat("visibility", &input->Visibility, available.visibilityMin, available.visibilityMax, "%.1f km", ImGuiSliderFlags_AlwaysClamp);
	ImGui::SameLine();
	sprintf(label, "Horizontal visibility (meteorological range) at ground level in kilometers, value in range [%.1f, %.1f]", available.visibilityMin, available.visibilityMax);
	helpMarker(label);

	*valueChanged |= ImGui::Button("Update Transmittance");
}

void PragueUI::DrawCheckbox(const std::string& name, bool* param)
{
	ImGui::Checkbox(name.c_str(), param);
}

