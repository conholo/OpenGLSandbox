#include "PragueDriver.h"
#include "PragueUI.h"
#include "Engine/Core/Application.h"
#include "Engine/Rendering/UniformBuffer.h"
#include "Engine/Rendering/ShaderStorageBuffer.h"
#include <glm/glm.hpp>

#include "Engine/Rendering/RenderCommand.h"

PragueDriver::PragueDriver()
{
	Engine::ShaderLibrary::Load("assets/shaders/Atmosphere/PragueTransmittance.glsl");
	Engine::ShaderLibrary::Load("assets/shaders/Atmosphere/PragueSkyRadiance.glsl");

	m_ViewportWidth = Engine::Application::GetApplication().GetWindow().GetWidth();
	m_ViewportHeight = Engine::Application::GetApplication().GetWindow().GetHeight();
	m_SkyModel = new PragueSkyModel;
	m_UI = new PragueUI;
	m_Input = new PragueInput;
	m_DatasetUniforms = new DatasetUniforms;
	
	m_PragueFSQ = Engine::CreateRef<Engine::SimpleEntity>(Engine::PrimitiveType::FullScreenQuad, "PragueTransmittance");
	Engine::FramebufferSpecification inScatterSpec =
	{
		m_ViewportWidth, m_ViewportHeight,
		{ Engine::FramebufferTextureFormat::RGBA32F, Engine::FramebufferTextureFormat::RGBA32F }
	};
	m_PragueFBO = Engine::CreateRef<Engine::Framebuffer>(inScatterSpec);

	m_Input->OriginData = glm::vec4(0.0, m_Input->Altitude, 0.0, 0.0);
    UploadDataset();

	const Engine::DispatchCreateRadianceMapFn PreDispatchFn =
	([this](auto&& UnFilteredCubeTexture, auto&& FilteredCubeTexture)
	{
	});

	const Engine::DispatchCreateRadianceMapFn PostDispatchFn =
	([this](auto&& UnFilteredCubeTexture, auto&& FilteredCubeTexture)
	{
	});
    
	m_EnvironmentSpec = Engine::CreateRef<Engine::EnvironmentMapSpecification>("PragueSky", PreDispatchFn, PostDispatchFn, 512);
	m_EnvironmentMapPipeline = Engine::CreateRef<Engine::EnvironmentMapPipeline>();
}

PragueDriver::~PragueDriver()
{
   delete m_SkyModel;
   delete m_UI;
   delete m_Input;
   delete m_DatasetUniforms;
}

void PragueDriver::RenderUI(bool* ValueChanged)
{
	if(!m_IsLoaded) return;
	m_UI->Begin();
	m_UI->DrawInputUI(m_Input, m_AvailableData, ValueChanged);
	*ValueChanged |= ImGui::DragFloat("Transmittance Scale", &m_TransmittanceScale, 0.01f, 0.0f);
	*ValueChanged |= ImGui::DragFloat("Sun Radiance Transmittance Scale", &m_SunRadianceTransmittanceScale, 0.01f, 0.0f);
	*ValueChanged |= ImGui::DragFloat("Sun Radiance Modifier", &m_TotalSunRadianceModifier, 0.01f, 0.0f);
	m_UI->End();
}

void PragueDriver::UploadDataset()
{
	const std::string filePath = "assets/data/SkyModelDataset.dat";
    m_IsLoaded = false;
	m_SkyModel->initialize(filePath, 23.8);
	m_AvailableData = m_SkyModel->getAvailableData();

	m_RadianceDataSSBO = Engine::CreateRef<Engine::ShaderStorageBuffer>((void*)m_SkyModel->GetRadianceData().data(), m_SkyModel->GetRadianceDataSize());
	m_SunMetaDataSSBO = Engine::CreateRef<Engine::ShaderStorageBuffer>((void*)m_SkyModel->GetSunMetaDataBreaks().data(), m_SkyModel->GetSunMetaDataBreaksSize());
	m_ZenithMetaDataSSBO = Engine::CreateRef<Engine::ShaderStorageBuffer>((void*)m_SkyModel->GetZenithMetaDataBreaks().data(), m_SkyModel->GetZenithMetaDataBreaksSize());
	m_EmphMetaDataSSBO = Engine::CreateRef<Engine::ShaderStorageBuffer>((void*)m_SkyModel->GetEmphMetaDataBreaks().data(), m_SkyModel->GetEmphMetaDataBreaksSize());
	m_VisibilitiesRadianceSSBO = Engine::CreateRef<Engine::ShaderStorageBuffer>((void*)m_SkyModel->GetVisibilitiesRadiance().data(), m_SkyModel->GetVisibilitiesRadianceSize());
	m_AlbedosRadianceSSBO = Engine::CreateRef<Engine::ShaderStorageBuffer>((void*)m_SkyModel->GetAlbedosRadiance().data(), m_SkyModel->GetAlbedosRadianceSize());
	m_AltitudesRadianceSSBO = Engine::CreateRef<Engine::ShaderStorageBuffer>((void*)m_SkyModel->GetAltitudesRadiance().data(), m_SkyModel->GetAltitudesRadianceSize());
	m_ElevationsRadianceSSBO = Engine::CreateRef<Engine::ShaderStorageBuffer>((void*)m_SkyModel->GetElevationsRadiance().data(), m_SkyModel->GetElevationsRadianceSize());
	m_DataTransVSSBO = Engine::CreateRef<Engine::ShaderStorageBuffer>((void*)m_SkyModel->GetTransV().data(), m_SkyModel->GetTransVSize());
	m_DataTransUSSBO = Engine::CreateRef<Engine::ShaderStorageBuffer>((void*)m_SkyModel->GetTransU().data(), m_SkyModel->GetTransUSize());
	m_AltitudeTransSSBO = Engine::CreateRef<Engine::ShaderStorageBuffer>((void*)m_SkyModel->GetTransAltitudes().data(), m_SkyModel->GetTransAltitudeSize());
	m_VisibilitiesTransSSBO = Engine::CreateRef<Engine::ShaderStorageBuffer>((void*)m_SkyModel->GetTransVisibilities().data(), m_SkyModel->GetTransVisibilitiesSize());

	m_RadianceDataSSBO->UseAndSetBinding(1);
	m_SunMetaDataSSBO->UseAndSetBinding(2);
	m_ZenithMetaDataSSBO->UseAndSetBinding(3);
	m_EmphMetaDataSSBO->UseAndSetBinding(4);
	m_VisibilitiesRadianceSSBO->UseAndSetBinding(5);
	m_AlbedosRadianceSSBO->UseAndSetBinding(6);
	m_AltitudesRadianceSSBO->UseAndSetBinding(7);
	m_ElevationsRadianceSSBO->UseAndSetBinding(8);
	m_DataTransVSSBO->UseAndSetBinding(9);
	m_DataTransUSSBO->UseAndSetBinding(10);
	m_AltitudeTransSSBO->UseAndSetBinding(11);
	m_VisibilitiesTransSSBO->UseAndSetBinding(12);
	m_InputUniformBuffer = Engine::CreateRef<Engine::UniformBuffer>(sizeof(PragueInput), 13);
	m_SkyDatasetConstantsUniformBuffer = Engine::CreateRef<Engine::UniformBuffer>(sizeof(DatasetUniforms), 14);

	m_DatasetUniforms->aDim = m_SkyModel->GetADim();
	m_DatasetUniforms->dDim = m_SkyModel->GetDDim();
	m_DatasetUniforms->RankTrans = m_SkyModel->GetTransRank();
	m_DatasetUniforms->Channels = m_SkyModel->GetChannels();
	m_DatasetUniforms->ChannelStart = m_SkyModel->GetChannelStart();
	m_DatasetUniforms->ChannelWidth = m_SkyModel->GetChannelWidth();
	m_DatasetUniforms->TotalCoefsSingleConfigRad = m_SkyModel->GetRadianceMetaData().totalCoefsSingleConfig;
	m_DatasetUniforms->TotalCoefsAllConfigsRad = m_SkyModel->GetRadianceMetaData().totalCoefsAllConfigs;
	m_DatasetUniforms->RadRank = m_SkyModel->GetRadianceMetaData().rank;
	m_DatasetUniforms->sunOffset = m_SkyModel->GetRadianceMetaData().sunOffset;
	m_DatasetUniforms->sunStride = m_SkyModel->GetRadianceMetaData().sunStride;
	m_DatasetUniforms->zenithOffset = m_SkyModel->GetRadianceMetaData().zenithOffset;
	m_DatasetUniforms->zenithStride = m_SkyModel->GetRadianceMetaData().zenithStride;
	m_DatasetUniforms->emphOffset = m_SkyModel->GetRadianceMetaData().emphOffset;
	m_SkyDatasetConstantsUniformBuffer->SetData(m_DatasetUniforms, sizeof(DatasetUniforms));
	m_InputUniformBuffer->SetData(m_Input, sizeof(PragueInput));
	m_IsLoaded = true;
}

void PragueDriver::Execute(const Engine::Camera& Camera, const Engine::Ref<Engine::Framebuffer>& SceneFBO) const
{
	m_Input->OriginData = glm::vec4(0.0f, m_Input->Altitude, 0.0f, 0.0f);
	m_InputUniformBuffer->SetData(m_Input, sizeof(PragueInput));

	m_EnvironmentMapPipeline->BuildFromShader("PragueSkyRadiance", m_EnvironmentSpec);
	UpdateTransmittance(Camera, SceneFBO);
}

void PragueDriver::UpdateTransmittance(const Engine::Camera& Camera, const Engine::Ref<Engine::Framebuffer>& SceneFBO) const
{
	m_PragueFBO->Bind();
	Engine::RenderCommand::ClearColor({0.0f, 0.0f, 0.0f,0.0f});
	Engine::RenderCommand::Clear(true, true);

	{
   		SceneFBO->BindDepthTexture(0);
   		m_PragueFSQ->GetEntityRenderer()->Bind();
   		m_PragueFSQ->GetEntityRenderer()->GetShader()->UploadUniformMat4("u_InverseView", glm::inverse(Camera.GetView()));
   		m_PragueFSQ->GetEntityRenderer()->GetShader()->UploadUniformMat4("u_InverseProjection", glm::inverse(Camera.GetProjection()));
   		m_PragueFSQ->GetEntityRenderer()->GetShader()->UploadUniformInt("u_DepthTexture", 0);
   		m_PragueFSQ->GetEntityRenderer()->GetShader()->UploadUniformFloat("u_TransmittanceScale", m_TransmittanceScale);
   		m_PragueFSQ->GetEntityRenderer()->GetShader()->UploadUniformFloat("u_SunRadianceTransmittanceScale", m_SunRadianceTransmittanceScale);
   		m_PragueFSQ->GetEntityRenderer()->GetShader()->UploadUniformFloat("u_TotalSunRadianceModifier", m_TotalSunRadianceModifier);
   		m_PragueFSQ->GetEntityRenderer()->GetShader()->UploadUniformFloat("u_Near", Camera.GetNearClip());
		m_PragueFSQ->GetEntityRenderer()->GetShader()->UploadUniformFloat("u_Far", Camera.GetFarClip());

   		m_PragueFSQ->DrawEntity(Camera.GetViewProjection());
	}
	m_PragueFBO->Unbind();
}

void PragueDriver::BindTransmittanceAttachment(uint32_t slot) const
{
	m_PragueFBO->BindColorAttachment(0, slot);
}

void PragueDriver::BindSunRadianceAttachment(uint32_t slot) const
{
	m_PragueFBO->BindColorAttachment(1, slot);
}
