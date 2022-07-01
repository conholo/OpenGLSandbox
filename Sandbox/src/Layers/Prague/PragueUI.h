#pragma once


#include "PragueSkyModel.h"
#include <imgui/imgui.h>
#include <imgui/imfilebrowser.h>
#include <functional>

class PragueUI
{
public:
	using LoadDataSetFn = std::function<void(const std::string&, double)>;

	PragueUI();
	~PragueUI() = default;

	void Begin();
	void End();
	void DrawDatasetConfiguration(const LoadDataSetFn& loadFn);
	void DrawInputUI(PragueInput* input, const PragueSkyModel::AvailableData& data, bool* valueChanged);
	void DrawCheckbox(const std::string& name, bool* param);


private:
	ImGui::FileBrowser m_FileBrowser;
};