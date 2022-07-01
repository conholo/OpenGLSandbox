#include "Layers/AtmosphereLayer.h"
#include <iostream>
#include <functional>
#include <imgui/imgui.h>

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
		Engine::ImageUtils::ImageInternalFormat::RGBA32F,
		Engine::ImageUtils::ImageDataLayout::RGBA,
		Engine::ImageUtils::ImageDataType::Float,
		256, 256
	};

	m_CubeTexture = Engine::CreateRef<Engine::TextureCube>(skyBoxSpec, nullptr);
	m_SkyBox = Engine::CreateRef<Engine::CubeMap>(m_CubeTexture, Engine::ShaderLibrary::Get("Skybox"));
	m_CubeTexture->BindToImageSlot(0, 0, Engine::ImageUtils::TextureAccessLevel::WriteOnly, Engine::ImageUtils::TextureShaderDataFormat::RGBA32F);
	m_EditorGrid = Engine::CreateRef<Engine::EditorGrid>();

	Engine::Application::GetApplication().GetWindow().ToggleMaximize(true);
	m_WhiteTexture = Engine::Texture2D::CreateWhiteTexture();
	m_Planet = Engine::CreateRef<Engine::SimpleEntity>(Engine::PrimitiveType::Sphere, "FlatColor");
	m_Ground = Engine::CreateRef<Engine::SimpleEntity>(Engine::PrimitiveType::Plane, "FlatColor");

	m_Planet->GetEntityTransform()->SetPosition({ 0.0f, 5.0f, 0.0f });
	m_Ground->GetEntityTransform()->SetPosition({ 0.0f, 0.0f, 0.0f });
	m_Ground->GetEntityTransform()->SetScale({ 5.0f, 5.0f, 5.0f });
	m_Camera.SetPosition({ 0.0f, 3.0f, 10.0f });

	m_SkyModel = new PragueSkyModel;
	m_UI = new PragueUI;
	m_Input = new PragueInput;
}

void AtmosphereLayer::OnDetach()
{
	delete m_SkyModel;
	delete m_UI;
	delete m_Input;
}

void AnimateParam(float* param, float speed, float min, float max)
{
	float t = glm::sin(Engine::Time::Elapsed() * speed) * 0.5 + 0.5;
	*param = Engine::Lerp(min, max, t);
}

void AtmosphereLayer::OnUpdate(float deltaTime)
{
	m_Camera.Update(deltaTime);

	m_Timer += deltaTime;

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

	if (!m_IsLoaded)
	{
		if (m_DisplayGrid)
			m_EditorGrid->Draw(m_Camera);
		return;
	}

	// Read from & Render Cubemap
	Engine::ShaderLibrary::Get("Skybox")->Bind();
	Engine::ShaderLibrary::Get("Skybox")->UploadUniformFloat("u_TextureLOD", 0);
	Engine::ShaderLibrary::Get("Skybox")->UploadUniformFloat("u_Intensity", 1.0f);
	m_SkyBox->Submit(glm::inverse(m_Camera.GetViewProjection()));

	if(m_DisplayGrid)
		m_EditorGrid->Draw(m_Camera);
}

void AtmosphereLayer::OnEvent(Engine::Event& e)
{
	m_Camera.OnEvent(e);
}

void AtmosphereLayer::OnImGuiRender()
{
	m_UI->Begin();
	m_UI->DrawDatasetConfiguration(std::bind(&AtmosphereLayer::LoadDataset, this, std::placeholders::_1, std::placeholders::_2));
	m_UI->DrawCheckbox("Draw Grid", &m_DisplayGrid);
	if (m_IsLoaded)
		m_UI->DrawInputUI(m_Input, m_AvailableData, &m_Update);
	m_UI->End();

	if (m_Update)
		UpdateSkyBox();
}

void AtmosphereLayer::UpdateSkyBox()
{
	Engine::ShaderLibrary::Get("PragueSky")->Bind();
	Engine::ShaderLibrary::Get("PragueSky")->UploadUniformFloat("u_Visibility", m_Input->Visibility);
	Engine::ShaderLibrary::Get("PragueSky")->UploadUniformFloat("u_Elevation", m_Input->Elevation);
	Engine::ShaderLibrary::Get("PragueSky")->UploadUniformFloat("u_Azimuth", m_Input->Azimuth);
	Engine::ShaderLibrary::Get("PragueSky")->UploadUniformFloat("u_Altitude", m_Input->Altitude);
	Engine::ShaderLibrary::Get("PragueSky")->UploadUniformFloat("u_Albedo", m_Input->Albedo);
	Engine::ShaderLibrary::Get("PragueSky")->UploadUniformFloat("u_Exposure", m_Input->Exposure);
	Engine::ShaderLibrary::Get("PragueSky")->UploadUniformInt("u_ApplyTonemap", m_Input->ApplyToneMap ? 1 : 0);

	Engine::ShaderLibrary::Get("PragueSky")->DispatchCompute(m_SkyBox->GetTexture3D()->GetWidth() / m_ThreadsPerGroup, m_SkyBox->GetTexture3D()->GetHeight() / m_ThreadsPerGroup, 6);
	Engine::ShaderLibrary::Get("PragueSky")->EnableShaderImageAccessBarrierBit();;
	Engine::ShaderLibrary::Get("PragueSky")->EnableBufferUpdateBarrierBit();
}

void AtmosphereLayer::LoadDataset(const std::string& datasetPath, double singleVisibility)
{
	m_IsLoaded = false;
	std::cout << "Loading Dataset from file..." << "\n";
	m_SkyModel->initialize(datasetPath, singleVisibility);
	m_AvailableData = m_SkyModel->getAvailableData();
	std::cout << "Dataset loaded." << "\n";

	std::cout << "Uploading dataset to GPU..." << "\n";
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

	std::cout << "GPU upload complete." << "\n";
	m_IsLoaded = true;

	UpdateSkyBox();
}
