#pragma once
#include "PragueSkyModel.h"
#include "Engine/Core/Memory.h"
#include "Engine/Rendering/Camera.h"
#include "Engine/Rendering/EnvironmentMapPipeline.h"
#include "Engine/Rendering/FrameBuffer.h"
#include "Engine/Scene/SimpleECS/SimpleEntity.h"

class PragueSkyModel;
struct PragueInput;
class PragueUI;

namespace Engine
{
    class UniformBuffer;
    class ShaderStorageBuffer;
    class SimpleEntity;
    class Framebuffer;
    class Camera;
}

struct DatasetUniforms
{
    float	ChannelStart;
    float	ChannelWidth;
    int		aDim;
    int		dDim;
    int		RankTrans;
    int		Channels;
    int	    TotalCoefsSingleConfigRad;
    int	    TotalCoefsAllConfigsRad;
    int	    RadRank;
    int		sunOffset;
    int		sunStride;
    int		zenithOffset;
    int		zenithStride;
    int		emphOffset;
};


class PragueDriver
{
public:
    PragueDriver();
    ~PragueDriver();
    void RenderUI(bool* ValueChanged);
    void Execute(const Engine::Camera& Camera, const Engine::Ref<Engine::Framebuffer>& SceneFBO) const;
    void UpdateTransmittance(const Engine::Camera& Camera, const Engine::Ref<Engine::Framebuffer>& SceneFBO) const;
    void BindTransmittanceAttachment(uint32_t slot = 0) const;
    void BindSunRadianceAttachment(uint32_t slot = 0) const;
    const Engine::Ref<Engine::EnvironmentMapPipeline>& GetEnvironmentMapPipeline() const { return m_EnvironmentMapPipeline; }
    PragueInput* GetInput() const { return m_Input; }
    uint32_t GetTransmittanceAttachmentID() const { return m_PragueFBO->GetColorAttachmentID(0); }
    uint32_t GetSunRadianceAttachmentID() const { return m_PragueFBO->GetColorAttachmentID(1); }

private:
    void UploadDataset();

private:
    bool m_IsLoaded = false;
    float m_TransmittanceScale = 0.01f;
    float m_SunRadianceTransmittanceScale = 1.0f;
    float m_TotalSunRadianceModifier = 1.0f;
    uint32_t m_ViewportWidth, m_ViewportHeight;

    Engine::Ref<Engine::SimpleEntity> m_PragueFSQ;
    Engine::Ref<Engine::Framebuffer> m_PragueFBO;

    Engine::Ref<Engine::ShaderStorageBuffer> m_RadianceDataSSBO;
    Engine::Ref<Engine::ShaderStorageBuffer> m_SunMetaDataSSBO;
    Engine::Ref<Engine::ShaderStorageBuffer> m_ZenithMetaDataSSBO;
    Engine::Ref<Engine::ShaderStorageBuffer> m_EmphMetaDataSSBO;
    Engine::Ref<Engine::ShaderStorageBuffer> m_VisibilitiesRadianceSSBO;
    Engine::Ref<Engine::ShaderStorageBuffer> m_AlbedosRadianceSSBO;
    Engine::Ref<Engine::ShaderStorageBuffer> m_AltitudesRadianceSSBO;
    Engine::Ref<Engine::ShaderStorageBuffer> m_ElevationsRadianceSSBO;

    Engine::Ref<Engine::ShaderStorageBuffer> m_DataTransUSSBO;
    Engine::Ref<Engine::ShaderStorageBuffer> m_DataTransVSSBO;
    Engine::Ref<Engine::ShaderStorageBuffer> m_AltitudeTransSSBO;
    Engine::Ref<Engine::ShaderStorageBuffer> m_VisibilitiesTransSSBO;

    Engine::Ref<Engine::UniformBuffer> m_InputUniformBuffer;
    Engine::Ref<Engine::UniformBuffer> m_SkyDatasetConstantsUniformBuffer;

    Engine::Ref<Engine::EnvironmentMapPipeline> m_EnvironmentMapPipeline;
    Engine::Ref<Engine::EnvironmentMapSpecification> m_EnvironmentSpec;
    
    DatasetUniforms* m_DatasetUniforms;
    PragueUI* m_UI;
    PragueInput* m_Input;
    PragueSkyModel* m_SkyModel;
    PragueSkyModel::AvailableData m_AvailableData;
};
