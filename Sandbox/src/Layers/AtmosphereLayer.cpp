#include "Layers/AtmosphereLayer.h"

AtmosphereLayer::AtmosphereLayer()
{

}

AtmosphereLayer::~AtmosphereLayer()
{

}

void AtmosphereLayer::OnAttach()
{
	Engine::ShaderLibrary::Load("assets/shaders/prague_sky/PragueSky.shader");
	Engine::TextureSpecification skyBoxSpec =
	{
		Engine::ImageUtils::Usage::Storage,
		Engine::ImageUtils::WrapMode::ClampToEdge,
		Engine::ImageUtils::FilterMode::Linear,
		Engine::ImageUtils::ImageInternalFormat::RGBA8,
		Engine::ImageUtils::ImageDataLayout::RGBA,
		Engine::ImageUtils::ImageDataType::UByte,
		128, 128
	};

	m_CubeTexture = Engine::CreateRef<Engine::TextureCube>(skyBoxSpec, nullptr);
	m_SkyBox = Engine::CreateRef<Engine::CubeMap>(m_CubeTexture, Engine::ShaderLibrary::Get("Skybox"));
	m_CubeTexture->BindToImageSlot(0, 0, Engine::ImageUtils::TextureAccessLevel::WriteOnly, Engine::ImageUtils::TextureShaderDataFormat::RGBA8);
	m_EditorGrid = Engine::CreateRef<Engine::EditorGrid>();

	Engine::Application::GetApplication().GetWindow().ToggleMaximize(true);
	m_WhiteTexture = Engine::Texture2D::CreateWhiteTexture();
	m_Planet = Engine::CreateRef<Engine::SimpleEntity>(Engine::PrimitiveType::Sphere, "FlatColor");
	m_Ground = Engine::CreateRef<Engine::SimpleEntity>(Engine::PrimitiveType::Plane, "FlatColor");

	m_Planet->GetEntityTransform()->SetPosition({ 0.0f, 5.0f, 0.0f });
	m_Ground->GetEntityTransform()->SetPosition({ 0.0f, 0.0f, 0.0f });
	m_Ground->GetEntityTransform()->SetScale({ 5.0f, 5.0f, 5.0f });

	m_SkyModel = new PragueSkyModel;

	// Create a file browser instance for selecting dataset file
	fileDialogOpen.SetTitle("Select dataset file");
	fileDialogOpen.SetTypeFilters({ ".dat" });

	m_AvailableData.albedoMin = 0.0;
	m_AvailableData.albedoMax = 1.0;
	m_AvailableData.altitudeMin = 0.0;
	m_AvailableData.altitudeMax = 15000.0;
	m_AvailableData.elevationMin = -4.2;
	m_AvailableData.elevationMax = 90.0;
	m_AvailableData.visibilityMin = 20.0;
	m_AvailableData.visibilityMax = 131.8;
	m_AvailableData.polarisation = true;
	m_AvailableData.channels = SPECTRUM_CHANNELS;
	m_AvailableData.channelStart = SPECTRUM_WAVELENGTHS[0] - 0.5 * SPECTRUM_STEP;
	m_AvailableData.channelWidth = SPECTRUM_STEP;
}

void AtmosphereLayer::OnDetach()
{
	delete m_SkyModel;
}

void AtmosphereLayer::OnUpdate(float deltaTime)
{
	m_Camera.Update(deltaTime);

	Engine::RenderCommand::ClearColor(m_ClearColor);
	Engine::RenderCommand::Clear(true, true);
	m_WhiteTexture->BindToSamplerSlot(0);

	m_Ground->GetEntityRenderer()->GetShader()->Bind();
	m_Ground->GetEntityRenderer()->GetShader()->UploadUniformInt("u_Texture", 0);
	m_Ground->GetEntityRenderer()->GetShader()->UploadUniformFloat3("u_Color", { 1.0f, 1.0f, 1.0f });
	m_Ground->DrawEntity(m_Camera.GetViewProjection());

	m_Planet->GetEntityRenderer()->GetShader()->Bind();
	m_Planet->GetEntityRenderer()->GetShader()->UploadUniformInt("u_Texture", 0);
	m_Planet->GetEntityRenderer()->GetShader()->UploadUniformFloat3("u_Color", { 1.0f, 0.0f, 0.0f });
	m_Planet->DrawEntity(m_Camera.GetViewProjection());

	if (m_SkyModel == nullptr || !m_SkyModel->isInitialized()) return;

	// Read from & Render Cubemap
	Engine::ShaderLibrary::Get("Skybox")->Bind();
	Engine::ShaderLibrary::Get("Skybox")->UploadUniformFloat("u_TextureLOD", 0);
	Engine::ShaderLibrary::Get("Skybox")->UploadUniformFloat("u_Intensity", 1.0f);
	m_SkyBox->Submit(glm::inverse(m_Camera.GetViewProjection()));

	m_EditorGrid->Draw(m_Camera);
}

void AtmosphereLayer::OnEvent(Engine::Event& e)
{
	m_Camera.OnEvent(e);
}

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


void AtmosphereLayer::OnImGuiRender()
{
	static std::string loadError = "";
	static bool        loaded = false;
	static bool        loading = false;
	static bool		   loadedToGPU = false;
	static bool		   loadingToGPU = false;
	static std::string datasetName = "PragueSkyModelDatasetGround.dat";
	static std::string datasetPath = "PragueSkyModelDatasetGround.dat";
	static int         mode = 0;
	static std::string outputName = "test.exr";
	static std::string outputPath = "test.exr";
	static int         resolution = 128;
	static int         renderedResolution = resolution;
	static bool        rendered = false;
	static bool        rendering = false;
	static std::string renderError = "";
	static long long   renderTime = 0;
	static bool        saved = false;
	static std::string saveError = "";
	static bool        updateTexture = false;
	static int         view = 0;
	static float       visibility = 59.4f;
	static int         visibilityToLoad = 0;
	static int         wavelength = 280;
	static float       zoom = 1.f;

	ImGui::SetNextWindowPos(ImVec2(0, 0));
	ImGui::Begin("Prague Sky Model");

	/////////////////////////////////////////////
	// Dataset section
	/////////////////////////////////////////////

	// Dataset section begin
	ImGui::Text("Dataset:");

	// Dataset file selection
	if (ImGui::Button(datasetName.c_str(), ImVec2(ImGui::CalcItemWidth(), 20)))
	{
		fileDialogOpen.Open();
	}
	ImGui::SameLine();
	ImGui::Text("file");
	ImGui::SameLine();
	helpMarker("Sky model dataset file");
	fileDialogOpen.Display();
	if (fileDialogOpen.HasSelected())
	{
		datasetPath = fileDialogOpen.GetSelected().string();
		datasetName = datasetPath.substr(datasetPath.find_last_of("\\") + 1);
		fileDialogOpen.ClearSelected();
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
			m_SkyModel->initialize(datasetPath, singleVisibility);
			loaded = true;
			m_AvailableData = m_SkyModel->getAvailableData();
		}
		catch (std::exception& e)
		{
			loadError = e.what();
			loaded = false;
		}
		loading = false;
		loadedToGPU = false;
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



	if (loaded)
	{
		if (ImGui::Button("Upload To GPU"))
		{
			loadingToGPU = true;
			ImGui::SameLine();
			ImGui::Text("Uploading via SSBO ...");
		}
		if (!loadedToGPU && loadingToGPU)
		{
			Engine::ShaderLibrary::Get("PragueSky")->Bind();
			m_RadianceDataSSBO = Engine::CreateRef<Engine::ShaderStorageBuffer>((void*)m_SkyModel->GetRadianceData().data(), m_SkyModel->GetRadianceDataSize());
			m_RadianceDataSSBO->BindToComputeShader(1, Engine::ShaderLibrary::Get("PragueSky")->GetID());

			m_SunMetaDataSSBO = Engine::CreateRef<Engine::ShaderStorageBuffer>((void*)m_SkyModel->GetSunMetaDataBreaks().data(), m_SkyModel->GetSunMetaDataBreaksSize());
			m_SunMetaDataSSBO->BindToComputeShader(2, Engine::ShaderLibrary::Get("PragueSky")->GetID());
			Engine::ShaderLibrary::Get("PragueSky")->UploadUniformInt("sunOffset", m_SkyModel->GetRadianceMetaData().sunOffset);
			Engine::ShaderLibrary::Get("PragueSky")->UploadUniformInt("sunStride", m_SkyModel->GetRadianceMetaData().sunStride);

			m_ZenithMetaDataSSBO = Engine::CreateRef<Engine::ShaderStorageBuffer>((void*)m_SkyModel->GetZenithMetaDataBreaks().data(), m_SkyModel->GetZenithMetaDataBreaksSize());
			m_ZenithMetaDataSSBO->BindToComputeShader(3, Engine::ShaderLibrary::Get("PragueSky")->GetID());
			Engine::ShaderLibrary::Get("PragueSky")->UploadUniformInt("zenithOffset", m_SkyModel->GetRadianceMetaData().zenithOffset);
			Engine::ShaderLibrary::Get("PragueSky")->UploadUniformInt("zenithStride", m_SkyModel->GetRadianceMetaData().zenithStride);

			m_EmphMetaDataSSBO = Engine::CreateRef<Engine::ShaderStorageBuffer>((void*)m_SkyModel->GetEmphMetaDataBreaks().data(), m_SkyModel->GetEmphMetaDataBreaksSize());
			m_EmphMetaDataSSBO->BindToComputeShader(4, Engine::ShaderLibrary::Get("PragueSky")->GetID());
			Engine::ShaderLibrary::Get("PragueSky")->UploadUniformInt("emphOffset", m_SkyModel->GetRadianceMetaData().emphOffset);

			m_VisibilitiesRadianceSSBO = Engine::CreateRef<Engine::ShaderStorageBuffer>((void*)m_SkyModel->GetVisibilitiesRadiance().data(), m_SkyModel->GetVisibilitiesRadianceSize());
			m_VisibilitiesRadianceSSBO->BindToComputeShader(5, Engine::ShaderLibrary::Get("PragueSky")->GetID());

			m_AlbedosRadianceSSBO = Engine::CreateRef<Engine::ShaderStorageBuffer>((void*)m_SkyModel->GetAlbedosRadiance().data(), m_SkyModel->GetAlbedosRadianceSize());
			m_AlbedosRadianceSSBO->BindToComputeShader(6, Engine::ShaderLibrary::Get("PragueSky")->GetID());

			m_AltitudesRadianceSSBO = Engine::CreateRef<Engine::ShaderStorageBuffer>((void*)m_SkyModel->GetAltitudesRadiance().data(), m_SkyModel->GetAltitudesRadianceSize());
			m_AltitudesRadianceSSBO->BindToComputeShader(7, Engine::ShaderLibrary::Get("PragueSky")->GetID());

			m_ElevationsRadianceSSBO = Engine::CreateRef<Engine::ShaderStorageBuffer>((void*)m_SkyModel->GetElevationsRadiance().data(), m_SkyModel->GetElevationsRadianceSize());
			m_ElevationsRadianceSSBO->BindToComputeShader(8, Engine::ShaderLibrary::Get("PragueSky")->GetID());

			Engine::ShaderLibrary::Get("PragueSky")->UploadUniformInt("u_TotalCoefsSingleConfig", m_SkyModel->GetRadianceMetaData().totalCoefsSingleConfig);
			Engine::ShaderLibrary::Get("PragueSky")->UploadUniformInt("u_TotalCoefsAllConfigs", m_SkyModel->GetRadianceMetaData().totalCoefsAllConfigs);
			Engine::ShaderLibrary::Get("PragueSky")->UploadUniformInt("u_Rank", m_SkyModel->GetRadianceMetaData().rank);

			Engine::ShaderLibrary::Get("PragueSky")->UploadUniformInt("u_Channels", m_SkyModel->GetChannels());
			Engine::ShaderLibrary::Get("PragueSky")->UploadUniformFloat("u_ChannelStart", (float)m_SkyModel->GetChannelStart());
			Engine::ShaderLibrary::Get("PragueSky")->UploadUniformFloat("u_ChannelWidth", (float)m_SkyModel->GetChannelWidth());

			UpdateSkyBox();
			loadedToGPU = true;
			loadingToGPU = false;
		}
		if (loadedToGPU && !loadingToGPU)
		{
			ImGui::SameLine();
			ImGui::Text("OK");
		}
	}


	// Dataset section end
	ImGui::Dummy(ImVec2(0.0f, 1.0f));
	ImGui::Separator();

	/////////////////////////////////////////////
// Configuration section
/////////////////////////////////////////////

// Configuration section begin
	ImGui::Text("Configuration:");

	char        label[150];

	if (m_SkyModel->isInitialized() && loadedToGPU)
	{
		PragueSkyModel::AvailableData available = m_SkyModel->getAvailableData();

		if (ImGui::SliderFloat("Albedo", &m_Albedo, available.albedoMin, available.albedoMax, "%.2f", ImGuiSliderFlags_AlwaysClamp))
			UpdateSkyBox();
		ImGui::SameLine();
		sprintf(label, "Ground albedo, value in range [%.1f, %.1f]", m_SkyModel->getAvailableData().albedoMin, m_SkyModel->getAvailableData().albedoMax);
		helpMarker(label);

		if (ImGui::SliderFloat("altitude", &m_Altitude, available.altitudeMin, available.altitudeMax, "%.0f m", ImGuiSliderFlags_AlwaysClamp))
			UpdateSkyBox();
		ImGui::SameLine();
		sprintf(label, "Altitude of view point in meters, value in range [%.1f, %.1f]", available.altitudeMin, available.altitudeMax);
		helpMarker(label);

		if (ImGui::SliderAngle("azimuth", &m_Azimuth, 0.0f, 360.0f, "%.1f deg", ImGuiSliderFlags_AlwaysClamp))
			UpdateSkyBox();
		ImGui::SameLine();
		helpMarker("Sun azimuth at view point in degrees, value in range [0, 360]");

		if (ImGui::SliderAngle("elevation", &m_Elevation, available.elevationMin, available.elevationMax, "%.1f deg", ImGuiSliderFlags_AlwaysClamp))
			UpdateSkyBox();
		ImGui::SameLine();
		sprintf(label, "Sun elevation at view point in degrees, value in range [%.1f, %.1f]", available.elevationMin, available.elevationMax);
		helpMarker(label);

		if (ImGui::SliderFloat("visibility", &m_Visibility, available.visibilityMin, available.visibilityMax, "%.1f km", ImGuiSliderFlags_AlwaysClamp))
			UpdateSkyBox();
		ImGui::SameLine();
		sprintf(label, "Horizontal visibility (meteorological range) at ground level in kilometers, value in range [%.1f, %.1f]", available.visibilityMin, available.visibilityMax);
		helpMarker(label);

		if (ImGui::SliderFloat("exposure", &m_Exposure, -25.0f, 25.0f, "%.1f"))
			UpdateSkyBox();
	}
	else
	{
		ImGui::Text("Please load the dataset to gain access to configuration parameters.");
	}

	ImGui::End();
}

void AtmosphereLayer::UpdateSkyBox()
{
	Engine::ShaderLibrary::Get("PragueSky")->Bind();
	Engine::ShaderLibrary::Get("PragueSky")->UploadUniformFloat("u_Visibility", m_Visibility);
	Engine::ShaderLibrary::Get("PragueSky")->UploadUniformFloat("u_Elevation", m_Elevation);
	Engine::ShaderLibrary::Get("PragueSky")->UploadUniformFloat("u_Azimuth", m_Azimuth);
	Engine::ShaderLibrary::Get("PragueSky")->UploadUniformFloat("u_Altitude", m_Altitude);
	Engine::ShaderLibrary::Get("PragueSky")->UploadUniformFloat("u_Albedo", m_Albedo);
	Engine::ShaderLibrary::Get("PragueSky")->UploadUniformFloat("u_Exposure", m_Exposure);

	Engine::ShaderLibrary::Get("PragueSky")->DispatchCompute(m_SkyBox->GetTexture3D()->GetWidth() / m_ThreadsPerGroup, m_SkyBox->GetTexture3D()->GetHeight() / m_ThreadsPerGroup, 6);
	Engine::ShaderLibrary::Get("PragueSky")->EnableShaderImageAccessBarrierBit();;
	Engine::ShaderLibrary::Get("PragueSky")->EnableBufferUpdateBarrierBit();
}
