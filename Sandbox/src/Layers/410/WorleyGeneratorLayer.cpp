#include "Layers/410/WorleyGeneratorLayer.h"
#include <iostream>
#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>
#include <glm/gtx/string_cast.hpp>



WorleyGeneratorLayer::WorleyGeneratorLayer()
{
}

WorleyGeneratorLayer::~WorleyGeneratorLayer()
{

}

static std::vector<glm::vec4> CreatePoints(uint32_t cells, int seed)
{
	std::vector<glm::vec4> points;
	points.resize(cells * cells * cells);

	Engine::Random::Seed(seed);
	float cellSize = 1.0f / cells;

	for (uint32_t x = 0; x < cells; x++)
	{
		for (uint32_t y = 0; y < cells; y++)
		{
			for (uint32_t z = 0; z < cells; z++)
			{
				float randomX = Engine::Random::RandomRange(0, 1);
				float randomY = Engine::Random::RandomRange(0, 1);
				float randomZ = Engine::Random::RandomRange(0, 1);

				glm::vec3 cellCorner = glm::vec3(x, y, z) * cellSize;
				glm::vec3 offset = glm::vec3(randomX, randomY, randomZ) * cellSize;

				uint32_t index = x + cells * (y + z * cells);
				points[index] = glm::vec4(cellCorner + offset, 0.0f);
			}
		}
	}

	return points;
}

static glm::vec4 ColorFromMask(ChannelMask mask)
{
	switch (mask)
	{
	case ChannelMask::R: return { 1.0f, 0.0f, 0.0f, 0.0f };
	case ChannelMask::G: return { 0.0f, 1.0f, 0.0f, 0.0f };
	case ChannelMask::B: return { 0.0f, 0.0f, 1.0f, 0.0f };
	case ChannelMask::A: return { 0.0f, 0.0f, 0.0f, 1.0f };
	}

	return { 0.0f, 0.0f, 0.0f, 0.0f };
}


void WorleyGeneratorLayer::UpdateChannel(ChannelMask mask, float persistence, const glm::ivec3& cells)
{
	uint32_t threadGroups = glm::ceil(m_Resolution / (float)m_ThreadGroupSize);

	std::vector<glm::vec4> pointsA = CreatePoints(cells.x, m_Seed);
	std::vector<glm::vec4> pointsB = CreatePoints(cells.y, m_Seed);
	std::vector<glm::vec4> pointsC = CreatePoints(cells.z, m_Seed);


	m_WorleyPointsBufferA = Engine::CreateRef<Engine::ShaderStorageBuffer>(pointsA.data(), sizeof(glm::vec4) * pointsA.size());
	m_WorleyPointsBufferA->BindToComputeShader(0, Engine::ShaderLibrary::Get("WorleyGenerator")->GetID());
	m_WorleyPointsBufferB = Engine::CreateRef<Engine::ShaderStorageBuffer>(pointsB.data(), sizeof(glm::vec4) * pointsB.size());
	m_WorleyPointsBufferB->BindToComputeShader(1, Engine::ShaderLibrary::Get("WorleyGenerator")->GetID());
	m_WorleyPointsBufferC = Engine::CreateRef<Engine::ShaderStorageBuffer>(pointsC.data(), sizeof(glm::vec4) * pointsC.size());
	m_WorleyPointsBufferC->BindToComputeShader(2, Engine::ShaderLibrary::Get("WorleyGenerator")->GetID());

	Engine::ShaderLibrary::Get("WorleyGenerator")->Bind();
	Engine::ShaderLibrary::Get("WorleyGenerator")->UploadUniformInt("u_CellsA", cells.x);
	Engine::ShaderLibrary::Get("WorleyGenerator")->UploadUniformInt("u_CellsB", cells.y);
	Engine::ShaderLibrary::Get("WorleyGenerator")->UploadUniformInt("u_CellsC", cells.z);
	Engine::ShaderLibrary::Get("WorleyGenerator")->UploadUniformFloat("u_Tiling", m_TextureTiling);
	Engine::ShaderLibrary::Get("WorleyGenerator")->UploadUniformFloat("u_Persistence", persistence);
	Engine::ShaderLibrary::Get("WorleyGenerator")->UploadUniformFloat4("u_ChannelMask", ColorFromMask(mask));
	Engine::ShaderLibrary::Get("WorleyGenerator")->DispatchCompute(threadGroups, threadGroups, threadGroups);
	Engine::ShaderLibrary::Get("WorleyGenerator")->EnableShaderImageAccessBarrierBit();
}


void WorleyGeneratorLayer::OnAttach()
{
	m_Camera.SetOrthographic();
	Engine::Application::GetApplication().GetWindow().ToggleMaximize(true);
	m_WhiteTexture = Engine::Texture2D::CreateWhiteTexture();
	Engine::ShaderLibrary::Load("assets/410 shaders/WorleyGenerator.shader");
	Engine::ShaderLibrary::Load("assets/410 shaders/Unlit.shader");

	m_Entity = Engine::CreateRef<Engine::SimpleEntity>(Engine::PrimitiveType::Quad, "Unlit");
	m_Entity->GetEntityTransform()->SetScale({ 1.5f, 1.5f, 1.5f });

	Engine::TextureSpecification spec =
	{
		Engine::ImageUtils::Usage::Storage,
		Engine::ImageUtils::WrapMode::Repeat,
		Engine::ImageUtils::FilterMode::Linear,
		Engine::ImageUtils::ImageInternalFormat::RGBA32F,
		Engine::ImageUtils::ImageDataLayout::RGBA,
		Engine::ImageUtils::ImageDataType::Float,
		m_Resolution, m_Resolution
	};

	m_Worley3DTexture = Engine::CreateRef<Engine::Texture3D>(spec);
	m_Worley3DTexture->BindToImageSlot(0, 0, Engine::ImageUtils::TextureAccessLevel::ReadWrite, Engine::ImageUtils::TextureShaderDataFormat::RGBA32F);

	UpdateChannel(ChannelMask::R, 0.1f, {2, 3, 5});
	UpdateChannel(ChannelMask::G, 0.3f, {3, 5, 8});
	UpdateChannel(ChannelMask::B, 0.7f, {7, 8, 9});
	UpdateChannel(ChannelMask::A, 0.9f, {8, 10, 11});
}

void WorleyGeneratorLayer::OnDetach()
{
}

void WorleyGeneratorLayer::OnUpdate(float deltaTime)
{
	m_Camera.Update(deltaTime);

	Engine::RenderCommand::ClearColor({ 0.0f, 0.0f, 0.0f, 0.0f });
	Engine::RenderCommand::Clear(true, true);

	m_Entity->GetEntityRenderer()->GetShader()->Bind();
	m_Worley3DTexture->BindToSamplerSlot(0);
	m_Entity->GetEntityRenderer()->GetShader()->UploadUniformInt("u_Texture", 0);
	m_Entity->GetEntityRenderer()->GetShader()->UploadUniformFloat("u_DepthSlice", m_DepthViewer);
	m_Entity->GetEntityRenderer()->GetShader()->UploadUniformFloat3("u_Color", { 1.0f, 1.0f, 1.0f });
	m_Entity->DrawEntity(m_Camera.GetViewProjection());
}

void WorleyGeneratorLayer::OnEvent(Engine::Event& event)
{
	m_Camera.OnEvent(event);
}

static std::string ChannelMaskToString(ChannelMask mask)
{
	switch (mask)
	{
	case ChannelMask::R: return "R";
	case ChannelMask::G: return "G";
	case ChannelMask::B: return "B";
	case ChannelMask::A: return "A";
	}

	return "";
}

void WorleyGeneratorLayer::DrawChannelSelector(const std::string& label)
{
	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(2, 2));
	if (ImGui::BeginTable("##Channel Mask Table", 2, ImGuiTableFlags_BordersOuter | ImGuiTableFlags_Resizable))
	{
		ImGui::TableNextRow();
		ImGui::TableSetColumnIndex(0);
		ImGui::AlignTextToFramePadding();
		ImGui::Text(label.c_str());
		ImGui::TableSetColumnIndex(1);
		ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x);

		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(2, 2));
		if (ImGui::BeginTable("##Button Table", 5, ImGuiTableFlags_BordersOuter | ImGuiTableFlags_Resizable | ImGuiTableFlags_BordersInner))
		{
			float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;
			ImVec2 buttonSize = { lineHeight + 10.0f, lineHeight };

			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);

			glm::vec4 activeColor = ColorFromMask(m_ActiveMask);

			if (ImGui::Button("R", buttonSize))
				m_ActiveMask = ChannelMask::R;
			ImGui::TableSetColumnIndex(1);
			if (ImGui::Button("G", buttonSize))
				m_ActiveMask = ChannelMask::G;
			ImGui::TableSetColumnIndex(2);
			if (ImGui::Button("B", buttonSize))
				m_ActiveMask = ChannelMask::B;
			ImGui::TableSetColumnIndex(3);
			if (ImGui::Button("A", buttonSize))
				m_ActiveMask = ChannelMask::A;
			ImGui::TableSetColumnIndex(4);
			ImGui::Image((void*)m_WhiteTexture->GetID(), buttonSize, { 0.0f, 0.0f }, { 1.0f, 1.0f }, { activeColor.r, activeColor.g, activeColor.b, 1.0f });

			ImGui::EndTable();
		}
		ImGui::PopStyleVar();

		ImGui::PopItemWidth();
		ImGui::EndTable();
	}
	ImGui::PopStyleVar();
}

void WorleyGeneratorLayer::DrawDisplayChannelMaskSelector(const std::string& label)
{
	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(2, 2));
	if (ImGui::BeginTable("##Channel Mask Table", 2, ImGuiTableFlags_BordersOuter | ImGuiTableFlags_Resizable))
	{
		ImGui::TableNextRow();
		ImGui::TableSetColumnIndex(0);
		ImGui::AlignTextToFramePadding();
		ImGui::Text(label.c_str());
		ImGui::TableSetColumnIndex(1);
		ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x);

		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(2, 2));
		if (ImGui::BeginTable("##Button Table", 5, ImGuiTableFlags_BordersOuter | ImGuiTableFlags_Resizable | ImGuiTableFlags_BordersInner))
		{
			float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;
			ImVec2 buttonSize = { lineHeight + 10.0f, lineHeight };

			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);

			glm::vec4 activeColor = ColorFromMask(m_ActiveMask);

			if (ImGui::Button("R", buttonSize))
				m_ActiveMask = ChannelMask::R;
			ImGui::TableSetColumnIndex(1);
			if (ImGui::Button("G", buttonSize))
				m_ActiveMask = ChannelMask::G;
			ImGui::TableSetColumnIndex(2);
			if (ImGui::Button("B", buttonSize))
				m_ActiveMask = ChannelMask::B;
			ImGui::TableSetColumnIndex(3);
			if (ImGui::Button("A", buttonSize))
				m_ActiveMask = ChannelMask::A;
			ImGui::TableSetColumnIndex(4);
			ImGui::Image((void*)m_WhiteTexture->GetID(), buttonSize, { 0.0f, 0.0f }, { 1.0f, 1.0f }, { activeColor.r, activeColor.g, activeColor.b, 1.0f });

			ImGui::EndTable();
		}
		ImGui::PopStyleVar();

		ImGui::PopItemWidth();
		ImGui::EndTable();
	}
	ImGui::PopStyleVar();
}

void WorleyGeneratorLayer::OnImGuiRender()
{
	ImGui::Begin("Worley Generator");

	ImGui::DragFloat("Depth Viewer", &m_DepthViewer, 0.01, 0.0f, 1.0f);
	if (ImGui::DragFloat("Texture Scale", &m_TextureTiling, 0.01))
		UpdateChannel(m_ActiveMask, m_Persistence, m_LayerCells);

	DrawChannelSelector("Channel Mask");

	ImGui::DragInt("DivisionsA", &m_LayerCells.x, 0.1, 1);
	ImGui::DragInt("DivisionsB", &m_LayerCells.y, 0.1, 1);
	ImGui::DragInt("DivisionsC", &m_LayerCells.z, 0.1, 1);

	ImGui::DragFloat("Persistence", &m_Persistence, 0.001, 0.01);
	ImGui::DragInt("Seed", &m_Seed, 0.1);

	if(ImGui::Button("Update Channel"))
		UpdateChannel(m_ActiveMask, m_Persistence, m_LayerCells);

	ImGui::End();
}
