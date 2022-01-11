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

static std::vector<glm::vec4> CreateWorleyPoints(uint32_t cells, int seed)
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
	case ChannelMask::All: return { 1.0f, 1.0f, 1.0f, 1.0f };
	}

	return { 0.0f, 0.0f, 0.0f, 0.0f };
}
static glm::vec4 DisplayMaskFromMask(ChannelMask mask)
{
	switch (mask)
	{
	case ChannelMask::R: return { 1.0f, 0.0f, 0.0f, 1.0f };
	case ChannelMask::G: return { 0.0f, 1.0f, 0.0f, 1.0f };
	case ChannelMask::B: return { 0.0f, 0.0f, 1.0f, 1.0f };
	case ChannelMask::A: return { 1.0f, 1.0f, 1.0f, 1.0f };
	case ChannelMask::All: return { 1.0f, 1.0f, 1.0f, 1.0f };
	}

	return { 0.0f, 0.0f, 0.0f, 0.0f };
}
static std::string ChannelMaskToString(ChannelMask mask)
{
	switch (mask)
	{
	case ChannelMask::R: return "R";
	case ChannelMask::G: return "G";
	case ChannelMask::B: return "B";
	case ChannelMask::A: return "A";
	case ChannelMask::All: return "All";
	}

	return "";
}

void WorleyGeneratorLayer::UpdateWorleyChannel(ChannelMask mask, float persistence, const glm::ivec3& cells)
{
	uint32_t threadGroups = glm::ceil(m_Resolution / (float)m_ThreadGroupSize);

	std::vector<glm::vec4> pointsA = CreateWorleyPoints(cells.x, m_RandomSeed);
	std::vector<glm::vec4> pointsB = CreateWorleyPoints(cells.y, m_RandomSeed);
	std::vector<glm::vec4> pointsC = CreateWorleyPoints(cells.z, m_RandomSeed);

	m_WorleyPointsBufferA = Engine::CreateRef<Engine::ShaderStorageBuffer>(pointsA.data(), sizeof(glm::vec4) * pointsA.size());
	m_WorleyPointsBufferA->BindToComputeShader(0, Engine::ShaderLibrary::Get("WorleyGenerator")->GetID());
	m_WorleyPointsBufferB = Engine::CreateRef<Engine::ShaderStorageBuffer>(pointsB.data(), sizeof(glm::vec4) * pointsB.size());
	m_WorleyPointsBufferB->BindToComputeShader(1, Engine::ShaderLibrary::Get("WorleyGenerator")->GetID());
	m_WorleyPointsBufferC = Engine::CreateRef<Engine::ShaderStorageBuffer>(pointsC.data(), sizeof(glm::vec4) * pointsC.size());
	m_WorleyPointsBufferC->BindToComputeShader(2, Engine::ShaderLibrary::Get("WorleyGenerator")->GetID());

	Engine::ShaderLibrary::Get("WorleyGenerator")->Bind();
	m_PerlinTestTexture->BindToSamplerSlot(0);
	Engine::ShaderLibrary::Get("WorleyGenerator")->UploadUniformBool("u_InvertNoise", m_InvertWorley);
	Engine::ShaderLibrary::Get("WorleyGenerator")->UploadUniformInt("u_CellsA", cells.x);
	Engine::ShaderLibrary::Get("WorleyGenerator")->UploadUniformInt("u_CellsB", cells.y);
	Engine::ShaderLibrary::Get("WorleyGenerator")->UploadUniformInt("u_CellsC", cells.z);
	Engine::ShaderLibrary::Get("WorleyGenerator")->UploadUniformFloat("u_Tiling", m_TextureTiling);
	Engine::ShaderLibrary::Get("WorleyGenerator")->UploadUniformFloat("u_PerlinWorleyMix", m_PerlinWorleyMix);
	Engine::ShaderLibrary::Get("WorleyGenerator")->UploadUniformInt("u_PerlinTexture", 0);
	Engine::ShaderLibrary::Get("WorleyGenerator")->UploadUniformFloat("u_Persistence", persistence);
	Engine::ShaderLibrary::Get("WorleyGenerator")->UploadUniformFloat4("u_ChannelMask", ColorFromMask(mask));
	Engine::ShaderLibrary::Get("WorleyGenerator")->DispatchCompute(threadGroups, threadGroups, threadGroups);
	Engine::ShaderLibrary::Get("WorleyGenerator")->EnableShaderImageAccessBarrierBit();
}
static std::vector<glm::vec4> GeneratePerlinOffsets(int octaves)
{
	std::vector<glm::vec4> result;
	result.resize(octaves);

	for (int i = 0; i < octaves; i++)
	{
		glm::vec4 random = { Engine::Random::RandomRange(0, 1), Engine::Random::RandomRange(0, 1), Engine::Random::RandomRange(0, 1), Engine::Random::RandomRange(0, 1) };
		result[i] = (random * glm::vec4(2.0) - glm::vec4(1.0)) * glm::vec4(1000.0);
	}

	return result;
}
void WorleyGeneratorLayer::UpdatePerlinTexture(bool updatePoints)
{
	uint32_t threadGroups = glm::ceil(m_Resolution / (float)m_ThreadGroupSize);

	if(updatePoints)
		m_RandomPerlinOffsets = GeneratePerlinOffsets(m_PerlinSettings.Octaves);

	m_RandomPerlinOffsetsBuffer = Engine::CreateRef<Engine::ShaderStorageBuffer>(m_RandomPerlinOffsets.data(), sizeof(glm::vec4) * m_RandomPerlinOffsets.size());
	m_RandomPerlinOffsetsBuffer->BindToComputeShader(3, Engine::ShaderLibrary::Get("Perlin2D")->GetID());

	Engine::ShaderLibrary::Get("Perlin2D")->Bind();
	Engine::ShaderLibrary::Get("Perlin2D")->UploadUniformFloat("u_Settings.NoiseScale", m_PerlinSettings.NoiseScale);
	Engine::ShaderLibrary::Get("Perlin2D")->UploadUniformFloat("u_Settings.Lacunarity", m_PerlinSettings.Lacunarity);
	Engine::ShaderLibrary::Get("Perlin2D")->UploadUniformFloat("u_Settings.Persistence", m_PerlinSettings.Persistence);
	Engine::ShaderLibrary::Get("Perlin2D")->UploadUniformInt("u_Settings.Octaves", m_PerlinSettings.Octaves);
	Engine::ShaderLibrary::Get("Perlin2D")->UploadUniformFloat2("u_Settings.TextureOffset", m_PerlinSettings.TextureOffset);
	Engine::ShaderLibrary::Get("Perlin2D")->DispatchCompute(threadGroups, threadGroups, threadGroups);
	Engine::ShaderLibrary::Get("Perlin2D")->EnableShaderImageAccessBarrierBit();
}
void WorleyGeneratorLayer::InitializeTextures()
{
	m_WorleyDisplayEntity = Engine::CreateRef<Engine::SimpleEntity>(Engine::PrimitiveType::Quad, "3DTextureViewer");
	m_WorleyDisplayEntity->GetEntityTransform()->SetScale({ 1.5f, 1.5f, 1.5f });
	m_PerlinDisplayEntity = Engine::CreateRef<Engine::SimpleEntity>(Engine::PrimitiveType::Quad, "FlatColor");
	m_PerlinDisplayEntity->GetEntityTransform()->SetScale({ 1.5f, 1.5f, 1.5f });

	Engine::Texture2DSpecification perlinSpec =
	{
		Engine::ImageUtils::WrapMode::Repeat,
		Engine::ImageUtils::WrapMode::Repeat,
		Engine::ImageUtils::FilterMode::Linear,
		Engine::ImageUtils::FilterMode::Linear,
		Engine::ImageUtils::ImageInternalFormat::RGBA32F,
		Engine::ImageUtils::ImageDataLayout::RGBA,
		Engine::ImageUtils::ImageDataType::Float,
		m_Resolution, m_Resolution
	};

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

	m_PerlinTestTexture = Engine::CreateRef<Engine::Texture2D>(perlinSpec);
	m_PerlinTestTexture->BindToImageSlot(1, 0, Engine::ImageUtils::TextureAccessLevel::WriteOnly, Engine::ImageUtils::TextureShaderDataFormat::RGBA32F);

	UpdatePerlinTexture(true);
	UpdateWorleyChannel(ChannelMask::R, 0.1f, { 2, 3, 5 });
	UpdateWorleyChannel(ChannelMask::G, 0.3f, { 3, 5, 8 });
	UpdateWorleyChannel(ChannelMask::B, 0.5f, { 7, 8, 9 });
	UpdateWorleyChannel(ChannelMask::A, 0.9f, { 8, 10, 11 });
}

void WorleyGeneratorLayer::OnAttach()
{
	Engine::Application::GetApplication().GetWindow().ToggleMaximize(true);
	m_WhiteTexture = Engine::Texture2D::CreateWhiteTexture();
	//Engine::ShaderLibrary::Load("assets/410 shaders/WorleyGenerator.shader");
	//Engine::ShaderLibrary::Load("assets/410 shaders/3DTextureViewer.shader");
	//Engine::ShaderLibrary::Load("assets/410 shaders/Perlin2D.shader");
	//Engine::ShaderLibrary::Load("assets/410 shaders/Clouds.shader");
	//Engine::ShaderLibrary::Load("assets/410 shaders/CloudsComposite.shader");

	uint32_t Width = 0, Height = 0;

	Engine::FramebufferSpecification fboSpec =
	{
		Engine::Application::GetApplication().GetWindow().GetWidth(),
		Engine::Application::GetApplication().GetWindow().GetHeight(),
		{ Engine::FramebufferTextureFormat::Depth, Engine::FramebufferTextureFormat::RGBA32F }
	};

	Engine::FramebufferSpecification cloudFboSpec =
	{
		Engine::Application::GetApplication().GetWindow().GetWidth(),
		Engine::Application::GetApplication().GetWindow().GetHeight(),
		{ Engine::FramebufferTextureFormat::RGBA32F }
	};

	m_CloudQuad = Engine::CreateRef<Engine::SimpleEntity>(Engine::PrimitiveType::FullScreenQuad, "Clouds");
	m_CompositeQuad = Engine::CreateRef<Engine::SimpleEntity>(Engine::PrimitiveType::FullScreenQuad, "CloudsComposite");
	m_SceneFBO = Engine::CreateRef<Engine::Framebuffer>(fboSpec);
	m_CloudFBO = Engine::CreateRef<Engine::Framebuffer>(cloudFboSpec);

	Engine::LightSpecification sunSpec =
	{
		Engine::LightType::Point,
		glm::vec3(1.0f),
		1.0f
	};
	m_Sun = Engine::CreateRef<Engine::Light>(sunSpec);
	m_Sun->GetLightTransform()->SetPosition({ 30.0f, 100.0f, 0.0f });

	InitializeTextures();
	InitializeSceneEntities();

	Resize(Engine::Application::GetApplication().GetWindow().GetWidth(), Engine::Application::GetApplication().GetWindow().GetHeight());
}

void WorleyGeneratorLayer::OnDetach()
{
}

void WorleyGeneratorLayer::OnUpdate(float deltaTime)
{
	m_Camera.Update(deltaTime);

	m_SceneFBO->Bind();
	Engine::RenderCommand::ClearColor({ 0.1f, 0.1f, 0.1f, 0.1f });
	Engine::RenderCommand::Clear(true, true);
	DrawTextures(deltaTime);
	DrawSceneEntities();
	m_SceneFBO->Unbind();

	m_CloudFBO->Bind();
	Engine::RenderCommand::ClearColor({ 0.1f, 0.1f, 0.1f, 0.1f });
	Engine::RenderCommand::Clear(true, false);

	Engine::RenderCommand::SetFaceCullMode(Engine::FaceCullMode::None);
	m_SceneFBO->BindColorAttachment(0);
	m_SceneFBO->BindDepthTexture(1);
	m_Worley3DTexture->BindToSamplerSlot(2);
	m_PerlinTestTexture->BindToSamplerSlot(3);

	m_CloudQuad->GetEntityRenderer()->GetShader()->Bind();
	m_CloudQuad->GetEntityRenderer()->GetShader()->UploadUniformInt("u_SceneTexture", 0);
	m_CloudQuad->GetEntityRenderer()->GetShader()->UploadUniformInt("u_DepthTexture", 1);
	m_CloudQuad->GetEntityRenderer()->GetShader()->UploadUniformInt("u_BaseShapeTexture", 2);
	m_CloudQuad->GetEntityRenderer()->GetShader()->UploadUniformInt("u_WeatherMap", 3);

	m_CloudQuad->GetEntityRenderer()->GetShader()->UploadUniformFloat("u_NearClip", m_Camera.GetNearClip());
	m_CloudQuad->GetEntityRenderer()->GetShader()->UploadUniformFloat("u_FarClip", m_Camera.GetFarClip());

	m_CloudQuad->GetEntityRenderer()->GetShader()->UploadUniformFloat3("u_SkyColorA", m_SkyColorA);
	m_CloudQuad->GetEntityRenderer()->GetShader()->UploadUniformFloat3("u_SkyColorB", m_SkyColorB);
	m_CloudQuad->GetEntityRenderer()->GetShader()->UploadUniformFloat3("u_Sun.LightPosition", m_Sun->GetLightTransform()->GetPosition());
	m_CloudQuad->GetEntityRenderer()->GetShader()->UploadUniformFloat("u_Sun.Intensity", m_Sun->GetLightIntensity());
	m_CloudQuad->GetEntityRenderer()->GetShader()->UploadUniformFloat3("u_Sun.LightColor", m_Sun->GetLightColor());

	m_CloudQuad->GetEntityRenderer()->GetShader()->UploadUniformFloat("u_ElapsedTime", Engine::Time::Elapsed());
	m_CloudQuad->GetEntityRenderer()->GetShader()->UploadUniformFloat("u_TimeScale", m_TimeScale);
	m_CloudQuad->GetEntityRenderer()->GetShader()->UploadUniformFloat("u_CloudScrollOffsetSpeed", m_CloudScrollOffsetSpeed);
	m_CloudQuad->GetEntityRenderer()->GetShader()->UploadUniformFloat("u_AnimationSpeed", m_AnimationSpeed);
	m_CloudQuad->GetEntityRenderer()->GetShader()->UploadUniformBool("u_Animate", m_AnimateClouds);

	m_CloudQuad->GetEntityRenderer()->GetShader()->UploadUniformInt("u_DensitySteps", m_DensitySteps);
	m_CloudQuad->GetEntityRenderer()->GetShader()->UploadUniformInt("u_LightSteps", m_LightSteps);
	m_CloudQuad->GetEntityRenderer()->GetShader()->UploadUniformFloat("u_ContainerEdgeFadeDistance", m_ContainerEdgeFadeDistance);
	m_CloudQuad->GetEntityRenderer()->GetShader()->UploadUniformFloat("u_PhaseBlend", m_PhaseBlend);
	m_CloudQuad->GetEntityRenderer()->GetShader()->UploadUniformFloat("u_CloudScale", m_CloudScale);
	m_CloudQuad->GetEntityRenderer()->GetShader()->UploadUniformFloat("u_SilverLiningConstant", m_SilverLiningConstant);
	m_CloudQuad->GetEntityRenderer()->GetShader()->UploadUniformFloat4("u_PhaseParams", m_PhaseParams);
	m_CloudQuad->GetEntityRenderer()->GetShader()->UploadUniformFloat("u_PowderConstant", m_PowderConstant);
	m_CloudQuad->GetEntityRenderer()->GetShader()->UploadUniformFloat("u_DensityThreshold", m_DensityThreshold);
	m_CloudQuad->GetEntityRenderer()->GetShader()->UploadUniformFloat("u_DensityMultiplier", m_DensityMultiplier);
	m_CloudQuad->GetEntityRenderer()->GetShader()->UploadUniformFloat3("u_CloudOffset", m_CloudOffset);
	m_CloudQuad->GetEntityRenderer()->GetShader()->UploadUniformFloat4("u_ShapeNoiseWeights", m_ShapeNoiseWeights);

	m_CloudQuad->GetEntityRenderer()->GetShader()->UploadUniformFloat3("u_WorldSpaceCameraPosition", m_Camera.GetPosition());
	m_CloudQuad->GetEntityRenderer()->GetShader()->UploadUniformFloat3("u_BoundsMin", m_CloudContainerPosition - m_CloudContainerScale / 2.0f);
	m_CloudQuad->GetEntityRenderer()->GetShader()->UploadUniformFloat3("u_BoundsMax", m_CloudContainerPosition + m_CloudContainerScale / 2.0f);
	m_CloudQuad->GetEntityRenderer()->GetShader()->UploadUniformMat4("u_InverseProjection", glm::inverse(m_Camera.GetProjection()));
	m_CloudQuad->GetEntityRenderer()->GetShader()->UploadUniformMat4("u_InverseView", glm::inverse(m_Camera.GetView()));
	m_CloudQuad->DrawEntity(m_Camera.GetViewProjection());
	m_CloudFBO->Unbind();
	Engine::RenderCommand::SetFaceCullMode(Engine::FaceCullMode::Back);

	Engine::RenderCommand::ClearColor({ 0.1f, 0.1f, 0.1f, 0.1f });
	Engine::RenderCommand::Clear(true, true);
	m_CloudFBO->BindColorAttachment(0);
	m_CompositeQuad->GetEntityRenderer()->GetShader()->Bind();
	m_CompositeQuad->GetEntityRenderer()->GetShader()->UploadUniformInt("u_SceneTexture", 0);
	m_CompositeQuad->DrawEntity(m_Camera.GetViewProjection());
}

void WorleyGeneratorLayer::DrawTextures(float deltaTime)
{
	if (m_DisplayTextures)
	{
		if (m_AnimatePerlinOffsets)
			m_PerlinSettings.TextureOffset += glm::vec2(deltaTime) * m_OffsetAnimateSpeed;

		if (m_DisplayWorley)
		{
			m_WorleyDisplayEntity->GetEntityRenderer()->GetShader()->Bind();
			m_Worley3DTexture->BindToSamplerSlot(0);
			m_WorleyDisplayEntity->GetEntityRenderer()->GetShader()->UploadUniformBool("u_ShowAllChannels", m_ShowAllChannels);
			m_WorleyDisplayEntity->GetEntityRenderer()->GetShader()->UploadUniformBool("u_GreyScale", m_GreyScale);
			m_WorleyDisplayEntity->GetEntityRenderer()->GetShader()->UploadUniformInt("u_Texture", 0);
			m_WorleyDisplayEntity->GetEntityRenderer()->GetShader()->UploadUniformFloat("u_DepthSlice", m_DepthViewer);
			m_WorleyDisplayEntity->GetEntityRenderer()->GetShader()->UploadUniformFloat4("u_ChannelWeights", m_ChannelWeights);
			m_WorleyDisplayEntity->GetEntityRenderer()->GetShader()->UploadUniformFloat3("u_Color", { 1.0f, 1.0f, 1.0f });
			m_WorleyDisplayEntity->DrawEntity(m_Camera.GetViewProjection());
		}
		else
		{
			m_PerlinDisplayEntity->GetEntityRenderer()->GetShader()->Bind();
			m_PerlinTestTexture->BindToSamplerSlot(0);
			m_PerlinDisplayEntity->GetEntityRenderer()->GetShader()->UploadUniformInt("u_Texture", 0);
			m_PerlinDisplayEntity->GetEntityRenderer()->GetShader()->UploadUniformFloat3("u_Color", { 1.0f, 1.0f, 1.0f });
			m_PerlinDisplayEntity->DrawEntity(m_Camera.GetViewProjection());
		}
	}
}


void WorleyGeneratorLayer::InitializeSceneEntities()
{
	m_Cube = Engine::CreateRef<Engine::SimpleEntity>(Engine::PrimitiveType::Cube, "FlatColor");
	m_Cube->GetEntityTransform()->SetPosition({ -3.0f, 2.0f, 1.0f });
	m_Sphere = Engine::CreateRef<Engine::SimpleEntity>(Engine::PrimitiveType::Sphere, "FlatColor");
	m_Sphere->GetEntityTransform()->SetPosition({ 2.0f, 1.5f, -1.0f });
	m_GroudPlane = Engine::CreateRef<Engine::SimpleEntity>(Engine::PrimitiveType::Plane, "FlatColor");
	m_GroudPlane->GetEntityTransform()->SetScale({ 10.0f, 1.0f, 10.0f });
	m_GroudPlane->GetEntityTransform()->SetPosition({ 0.0f, -1.0f, 0.0f });
}

void WorleyGeneratorLayer::DrawSceneEntities()
{
	m_WhiteTexture->BindToSamplerSlot(0);
	m_Cube->GetEntityRenderer()->GetShader()->Bind();
	m_Cube->GetEntityRenderer()->GetShader()->UploadUniformFloat3("u_Color", { 1.0f, 0.0f, 0.0f });
	m_Cube->DrawEntity(m_Camera.GetViewProjection());

	m_Sphere->GetEntityRenderer()->GetShader()->Bind();
	m_Sphere->GetEntityRenderer()->GetShader()->UploadUniformFloat3("u_Color", { 0.0f, 0.0f, 1.0f });
	m_Sphere->DrawEntity(m_Camera.GetViewProjection());

	m_GroudPlane->GetEntityRenderer()->GetShader()->Bind();
	m_GroudPlane->GetEntityRenderer()->GetShader()->UploadUniformFloat3("u_Color", { 0.8f, 0.8f, 0.8f });
	m_GroudPlane->DrawEntity(m_Camera.GetViewProjection());
}

void WorleyGeneratorLayer::OnEvent(Engine::Event& event)
{
	m_Camera.OnEvent(event);

	Engine::EventDispatcher dispatcher(event);
}

void WorleyGeneratorLayer::DrawChannelSelector(const std::string& label, ChannelMask& mask, bool showAll)
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
		if (ImGui::BeginTable("##Button Table", showAll ? 6 : 5, ImGuiTableFlags_BordersOuter | ImGuiTableFlags_Resizable | ImGuiTableFlags_BordersInner))
		{
			float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;
			ImVec2 buttonSize = { lineHeight + 10.0f, lineHeight };

			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);

			glm::vec4 activeColor = ColorFromMask(mask);

			if (ImGui::Button("R", buttonSize))
				mask = ChannelMask::R;
			ImGui::TableSetColumnIndex(1);
			if (ImGui::Button("G", buttonSize))
				mask = ChannelMask::G;
			ImGui::TableSetColumnIndex(2);
			if (ImGui::Button("B", buttonSize))
				mask = ChannelMask::B;
			ImGui::TableSetColumnIndex(3);
			if (ImGui::Button("A", buttonSize))
				mask = ChannelMask::A;
			ImGui::TableSetColumnIndex(4);
			ImGui::Image((void*)m_WhiteTexture->GetID(), buttonSize, { 0.0f, 0.0f }, { 1.0f, 1.0f }, { activeColor.r, activeColor.g, activeColor.b, 1.0f });

			if (showAll)
			{
				ImGui::TableSetColumnIndex(5);
				if (ImGui::Button("All", buttonSize))
					mask = ChannelMask::All;
			}

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
	ImGui::Begin("Scene Settings");

	ImGui::DragFloat3("Sun Position", &m_Sun->GetLightTransform()->GetPosition().x, 0.1);
	ImGui::DragFloat4("Sun Color", &m_Sun->GetLightColor().x, 0.1);
	ImGui::DragFloat("Sun Intensity", &m_Sun->GetLightIntensity(), 0.1);

	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
	ImGui::ColorEdit3("Sky Color A", &m_SkyColorA.r, ImGuiColorEditFlags_NoInputs);
	ImGui::PopStyleVar();
	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
	ImGui::ColorEdit3("Sky Color B", &m_SkyColorB.r, ImGuiColorEditFlags_NoInputs);
	ImGui::PopStyleVar();
	ImGui::Separator();

	ImGui::DragFloat3("Cube Position", &m_Cube->GetEntityTransform()->GetPosition().x, 0.1);
	ImGui::DragFloat3("Sphere Position", &m_Sphere->GetEntityTransform()->GetPosition().x, 0.1);
	ImGui::DragFloat3("Ground Position", &m_GroudPlane->GetEntityTransform()->GetPosition().x, 0.1);
	ImGui::DragFloat3("Cloud Container Position", &m_CloudContainerPosition.x, 0.1);
	ImGui::DragFloat3("Cloud Container Scale", &m_CloudContainerScale.x, 0.1);
	ImGui::DragFloat("Container Edge Fade Distance", &m_ContainerEdgeFadeDistance, 0.1);

	ImGui::Separator();
	if (ImGui::DragInt("Density Steps", &m_DensitySteps, 0.1, 1))
		if (m_DensitySteps < 1) m_DensitySteps = 1;
	if (ImGui::DragInt("Light Steps", &m_LightSteps, 0.1, 1))
		if (m_LightSteps < 1) m_LightSteps = 1;

	ImGui::Checkbox("Animate Clouds", &m_AnimateClouds);
	ImGui::DragFloat("Animation Speed", &m_AnimationSpeed, 0.1);
	ImGui::DragFloat("Time Scale", &m_TimeScale, 0.001);
	ImGui::DragFloat("Cloud Scroll Offset Speed", &m_CloudScrollOffsetSpeed, 0.1);


	ImGui::DragFloat("Density Multiplier", &m_DensityMultiplier, 0.001, 0.0);
	ImGui::DragFloat("Density Threshold", &m_DensityThreshold, 0.001, 0.0);
	ImGui::DragFloat("Powder Constant", &m_PowderConstant, 0.001, 0.0);
	ImGui::DragFloat("Phase Blend", &m_PhaseBlend, 0.001, 0.0);
	ImGui::DragFloat4("Phase Params", &m_PhaseParams.x, 0.001, 0.0);
	ImGui::DragFloat("Silver Lining Constant", &m_SilverLiningConstant, 0.001, 0.0);
	ImGui::DragFloat("Cloud Scale", &m_CloudScale, 0.01, 0.0);
	ImGui::DragFloat3("Cloud Offset", &m_CloudOffset.x, 0.1, 0.0);
	ImGui::DragFloat4("Shape Noise Weights", &m_ShapeNoiseWeights.x, 0.01, 0.0f, 1.0f);
	if (ImGui::DragFloat("Perlin Worley Mix", &m_PerlinWorleyMix, 0.001, 0.01))
		UpdateWorleyChannel(m_ActiveMask, m_WorleyLayerPersistence, m_LayerCells);
	if (ImGui::Checkbox("Invert Worley Noise", &m_InvertWorley))
		UpdateWorleyChannel(m_ActiveMask, m_WorleyLayerPersistence, m_LayerCells);

	ImGui::Image((void*)m_SceneFBO->GetDepthAttachmentID(), { 100, 100 }, { 0, 1 }, { 1, 0 });

	ImGui::End();



	ImGui::Begin("Texture Settings");
	
	ImGui::Checkbox("Display Textures", &m_DisplayTextures);

	if (!m_DisplayTextures)
	{
		ImGui::End();
		return;
	}

	const char* buttonLabel = m_DisplayWorley ? "Display Perlin2D" : "Display Worley";
	if (ImGui::Button(buttonLabel))
		m_DisplayWorley = !m_DisplayWorley;

	if (m_DisplayWorley)
	{
		ImGui::Checkbox("Show All Channels", &m_ShowAllChannels);
		ImGui::Checkbox("GreyScale", &m_GreyScale);
		ImGui::DragFloat("Depth Viewer", &m_DepthViewer, 0.001, 0.0f, 1.0f);
		if (ImGui::DragFloat("Texture Scale", &m_TextureTiling, 0.01))
			UpdateWorleyChannel(m_ActiveMask, m_WorleyLayerPersistence, m_LayerCells);

		ImGui::DragFloat4("Channel Display Weights", &m_ChannelWeights.x, 0.01, 0.0f, 1.0f);
		DrawChannelSelector("Channel Mask", m_ActiveMask);

		ImGui::DragInt("DivisionsA", &m_LayerCells.x, 0.1, 1);
		ImGui::DragInt("DivisionsB", &m_LayerCells.y, 0.1, 1);
		ImGui::DragInt("DivisionsC", &m_LayerCells.z, 0.1, 1);

		ImGui::DragFloat("Persistence", &m_WorleyLayerPersistence, 0.001, 0.01);

		ImGui::DragInt("Seed", &m_RandomSeed, 0.1);

		if (ImGui::Button("Update Channel"))
			UpdateWorleyChannel(m_ActiveMask, m_WorleyLayerPersistence, m_LayerCells);
	}
	else
	{
		if (ImGui::Button("Randomize Offsets"))
			UpdatePerlinTexture(true);

		ImGui::Checkbox("Animate Offsets", &m_AnimatePerlinOffsets);

		bool change = false;
		if (m_AnimatePerlinOffsets)
		{
			ImGui::DragFloat("Offset Speed", &m_OffsetAnimateSpeed, 0.1);
			change = true;
		}

		if (ImGui::DragFloat("Perlin Texture Scale", &m_PerlinSettings.NoiseScale, 0.001, 0.01))
			change = true;
		if (ImGui::DragInt("Octaves", &m_PerlinSettings.Octaves, 0.1, 0, 8))
			change = true;
		if (ImGui::DragFloat("Perlin Persistence", &m_PerlinSettings.Persistence, 0.001, 0.01, 1.0f))
			change = true;
		if (ImGui::DragFloat("Perlin Lacunarity", &m_PerlinSettings.Lacunarity, 0.001, 1.0f))
			change = true;
		if (ImGui::DragFloat2("Perlin Texture Offset", &m_PerlinSettings.TextureOffset.x, 0.01))
			change = true;

		if (change)
			UpdatePerlinTexture(false);
	}

	ImGui::End();
}


bool WorleyGeneratorLayer::Resize(int width, int height)
{
	m_SceneFBO->Resize(width, height);
	m_CloudFBO->Resize(width, height);

	return false;
}

