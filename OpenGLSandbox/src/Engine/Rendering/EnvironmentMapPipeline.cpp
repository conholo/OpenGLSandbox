#include "epch.h"
#include "EnvironmentMapPipeline.h"
#include "Shader.h"

namespace Engine
{
    EnvironmentMapPipeline::EnvironmentMapPipeline() = default;
    EnvironmentMapPipeline::~EnvironmentMapPipeline() = default;

    void EnvironmentMapPipeline::BuildFromShader(const std::string& CreationShaderName, const Ref<EnvironmentMapSpecification>& Specification)
    {
        m_Specification = Specification;
        GenerateFromShader(CreationShaderName, *m_Specification);
    }

    void EnvironmentMapPipeline::BuildFromEquirectangularImage(const std::string& FilePath)
    {
        m_Specification = Engine::CreateRef<EnvironmentMapSpecification>("ImageFileEnvironmentMap", nullptr, nullptr);
        GenerateFromFile(FilePath);
    }

    void EnvironmentMapPipeline::GenerateFromFile(const std::string& filePath) const
    {
        LOG_TRACE("-----Starting Environment Map Pipeline using file '{}'...-----", filePath);
        const Texture2DSpecification Specification
        {
            ImageUtils::WrapMode::Repeat,
            ImageUtils::WrapMode::Repeat,
            ImageUtils::FilterMode::Linear,
            ImageUtils::FilterMode::Linear,
            ImageUtils::ImageInternalFormat::FromImage,
            ImageUtils::ImageDataLayout::FromImage,
            ImageUtils::ImageDataType::UByte
        };
        
        const Ref<Texture2D> Equirectangular = TextureLibrary::LoadTexture2D(Specification, filePath);
        ASSERT(Equirectangular, "\tEnvironment Pipeline Error: Unable to create Equirectangular Texture from path '{}'.", filePath)

        uint32_t EnvironmentMapResolution = m_Specification->EnvironmentMapResolution;
        uint32_t ThreadGroupSize = m_Specification->GetThreadGroupSize();
        
        TextureCubeSpecification EnvironmentCubeSpec =
        {
            ImageUtils::WrapMode::ClampToEdge,
            ImageUtils::WrapMode::ClampToEdge,
            ImageUtils::WrapMode::ClampToEdge,
            ImageUtils::FilterMode::LinearMipLinear,
            ImageUtils::FilterMode::Linear,
            ImageUtils::ImageInternalFormat::RGBA32F,
            ImageUtils::ImageDataLayout::RGBA,
            ImageUtils::ImageDataType::Float,
            EnvironmentMapResolution
        };
        const glm::ivec3 ThreadGroups = glm::ivec3(
            glm::ceil(EnvironmentCubeSpec.Dimension / ThreadGroupSize),
            glm::ceil(EnvironmentCubeSpec.Dimension / ThreadGroupSize),
            6);

        EnvironmentCubeSpec.Name = m_Specification->EnvironmentMapName + "-EnvironmentRadianceCubeUnfiltered";
        const Ref<TextureCube> EnvironmentCubeUnfiltered = TextureLibrary::LoadTextureCube(EnvironmentCubeSpec, true);
        EnvironmentCubeSpec.Name = m_Specification->EnvironmentMapName + "-EnvironmentRadianceCubeFiltered";
        const Ref<TextureCube> EnvironmentCubeFiltered = TextureLibrary::LoadTextureCube(EnvironmentCubeSpec, true);

        // EquirectangularToCubemap
        {
            LOG_TRACE("\tDispatching 'EquirectangularToCubemap' Shader...");
            EnvironmentCubeUnfiltered->BindToImageSlot(0, 0, ImageUtils::TextureAccessLevel::WriteOnly, ImageUtils::TextureShaderDataFormat::RGBA32F);
            ShaderLibrary::Get("EquirectangularToCubemap")->Bind();
            ShaderLibrary::Get("EquirectangularToCubemap")->UploadUniformInt("sampler_EquirectangularTexture", 0);
            ShaderLibrary::Get("EquirectangularToCubemap")->DispatchCompute(ThreadGroups.x, ThreadGroups.y, ThreadGroups.z);
        }
        // EquirectangularToCubemap

        // EnvironmentFilter
        {
            const uint32_t MipCount = ImageUtils::CalculateMipLevelCount(EnvironmentMapResolution, EnvironmentMapResolution);
            float deltaRoughness = 1.0f / glm::max(static_cast<float>(MipCount) - 1.0f, 1.0f);
            
            for(uint32_t i = 0, size = EnvironmentMapResolution; i < MipCount; i++, size /= 2)
            {
                LOG_TRACE("\tDispatching 'EnvironmentFilter' at the {}th mip of the unfiltered environment map...", i);
                const uint32_t ThreadGroup = glm::max(1u, size / 32);
                float Roughness = i * deltaRoughness;
                Roughness = glm::max(Roughness, 0.05f);
                
                EnvironmentCubeFiltered->BindToImageSlot(0, i, ImageUtils::TextureAccessLevel::ReadWrite, ImageUtils::TextureShaderDataFormat::RGBA32F);
                EnvironmentCubeUnfiltered->BindToSamplerSlot(0);
                ShaderLibrary::Get("EnvironmentMipFilter")->Bind();
                ShaderLibrary::Get("EnvironmentMipFilter")->UploadUniformInt("sampler_InputCube", 0);
                ShaderLibrary::Get("EnvironmentMipFilter")->UploadUniformInt("MipOutputWidth", size);
                ShaderLibrary::Get("EnvironmentMipFilter")->UploadUniformInt("MipOutputHeight", size);
                ShaderLibrary::Get("EnvironmentMipFilter")->UploadUniformFloat("Roughness", Roughness);
                ShaderLibrary::Get("EnvironmentMipFilter")->DispatchCompute(ThreadGroup, ThreadGroup, ThreadGroups.z);
            }
        }
        // EnvironmentFilter

        // EnvironmentIrradiance
        {
            uint32_t IrradianceMapSize = m_Specification->GetIrradianceMapSize();
            uint32_t IrradianceMapComputeSamples = m_Specification->IrradianceMapComputeSamples;
            LOG_TRACE("\tDispatching 'EnvironmentIrradiance'...");

            TextureCubeSpecification IrradianceSpec =
            {
                ImageUtils::WrapMode::ClampToEdge,
                ImageUtils::WrapMode::ClampToEdge,
                ImageUtils::WrapMode::ClampToEdge,
                ImageUtils::FilterMode::LinearMipLinear,
                ImageUtils::FilterMode::Linear,
                ImageUtils::ImageInternalFormat::RGBA32F,
                ImageUtils::ImageDataLayout::RGBA,
                ImageUtils::ImageDataType::Float,
                IrradianceMapSize,
            };

            IrradianceSpec.Name = m_Specification->EnvironmentMapName + "-EnvironmentIrradianceCube";
            
            const glm::ivec3 IrradianceThreadGroups = glm::ivec3(
            glm::ceil(IrradianceSpec.Dimension / ThreadGroupSize),
            glm::ceil(IrradianceSpec.Dimension / ThreadGroupSize),
            6);

            Ref<TextureCube> IrradianceMap = TextureLibrary::LoadTextureCube(IrradianceSpec, true);
            IrradianceMap->BindToImageSlot(0, 0, ImageUtils::TextureAccessLevel::WriteOnly, ImageUtils::TextureShaderDataFormat::RGBA32F);
            EnvironmentCubeFiltered->BindToSamplerSlot(0);
            ShaderLibrary::Get("EnvironmentIrradiance")->Bind();
            ShaderLibrary::Get("EnvironmentIrradiance")->UploadUniformInt("sampler_RadianceMap", 0);
            ShaderLibrary::Get("EnvironmentIrradiance")->UploadUniformInt("EnvironmentSamples", IrradianceMapComputeSamples);
            ShaderLibrary::Get("EnvironmentIrradiance")->DispatchCompute(IrradianceThreadGroups.x, IrradianceThreadGroups.y, IrradianceThreadGroups.z);
        }
        // EnvironmentIrradiance

        LOG_TRACE("\t-----EnvironmentMapPipeline complete.  Radiance & Irradiance Maps are ready for use.-----");
    }

    void EnvironmentMapPipeline::GenerateFromShader(const std::string& CreationShader, const EnvironmentMapSpecification& Specification) const
    {
        LOG_TRACE("-----Starting Environment Map Pipeline using shader '{}'...-----", CreationShader);

        const uint32_t EnvironmentMapResolution = Specification.EnvironmentMapResolution;
        const uint32_t ThreadGroupSize = Specification.GetThreadGroupSize();

        TextureCubeSpecification EnvironmentCubeSpec =
        {
            ImageUtils::WrapMode::ClampToEdge,
            ImageUtils::WrapMode::ClampToEdge,
            ImageUtils::WrapMode::ClampToEdge,
            ImageUtils::FilterMode::LinearMipLinear,
            ImageUtils::FilterMode::Linear,
            ImageUtils::ImageInternalFormat::RGBA32F,
            ImageUtils::ImageDataLayout::RGBA,
            ImageUtils::ImageDataType::Float,
            EnvironmentMapResolution
        };
        
        const glm::ivec3 ThreadGroups = glm::ivec3(
            glm::ceil(EnvironmentCubeSpec.Dimension / ThreadGroupSize),
            glm::ceil(EnvironmentCubeSpec.Dimension / ThreadGroupSize),
            6);

        // Recreate Cubemap/Skybox specific resources
        EnvironmentCubeSpec.Name = Specification.EnvironmentMapName + "-EnvironmentRadianceCubeUnfiltered";
        const Ref<TextureCube> EnvironmentCubeUnfiltered = TextureLibrary::LoadTextureCube(EnvironmentCubeSpec, true);
        EnvironmentCubeSpec.Name = Specification.EnvironmentMapName + "-EnvironmentRadianceCubeFiltered";
        const Ref<TextureCube> EnvironmentCubeFiltered = TextureLibrary::LoadTextureCube(EnvironmentCubeSpec, true);

        // User Shader Radiance Cubemap Creation
        {
            ShaderLibrary::Get(CreationShader)->Bind();
            Specification.PreDispatchFn(EnvironmentCubeUnfiltered, EnvironmentCubeFiltered);
            EnvironmentCubeUnfiltered->BindToImageSlot(0, 0, ImageUtils::TextureAccessLevel::WriteOnly, ImageUtils::TextureShaderDataFormat::RGBA32F);
            ShaderLibrary::Get(CreationShader)->DispatchCompute(ThreadGroups.x, ThreadGroups.y, ThreadGroups.z);
            Specification.PostDispatchFn(EnvironmentCubeUnfiltered, EnvironmentCubeFiltered);
        }
        // User Shader Radiance Cubemap Creation

        // EnvironmentFilter
        {
            const uint32_t MaxMipCount = ImageUtils::CalculateMipLevelCount(EnvironmentMapResolution, EnvironmentMapResolution);
            const float DeltaRoughness = 1.0f / glm::max(static_cast<float>(MaxMipCount) - 1.0f, 1.0f);
            
            for(uint32_t Mip = 0; Mip < MaxMipCount; Mip++)
            {
                LOG_TRACE("\tDispatching 'EnvironmentFilter'-{} at the {} th mip of the unfiltered environment map...", Specification.EnvironmentMapName, Mip);
                const int MipMapSize = m_Specification->EnvironmentMapResolution >> Mip;
                const uint32_t ThreadGroupCount = glm::max(1u, MipMapSize / Specification.GetThreadGroupSize());
                float Roughness = Mip * DeltaRoughness;
                Roughness = glm::max(Roughness, 0.05f);
                
                EnvironmentCubeUnfiltered->BindToSamplerSlot(0);
                EnvironmentCubeFiltered->BindToImageSlot(0, Mip, ImageUtils::TextureAccessLevel::ReadWrite, ImageUtils::TextureShaderDataFormat::RGBA32F);
                ShaderLibrary::Get("EnvironmentMipFilter")->Bind();
                ShaderLibrary::Get("EnvironmentMipFilter")->UploadUniformInt("sampler_InputCube", 0);
                ShaderLibrary::Get("EnvironmentMipFilter")->UploadUniformInt("MipOutputWidth", MipMapSize);
                ShaderLibrary::Get("EnvironmentMipFilter")->UploadUniformInt("MipOutputHeight", MipMapSize);
                ShaderLibrary::Get("EnvironmentMipFilter")->UploadUniformFloat("Roughness", Roughness);
                ShaderLibrary::Get("EnvironmentMipFilter")->DispatchCompute(ThreadGroupCount, ThreadGroupCount, ThreadGroups.z);
            }
        }
        // EnvironmentFilter

        // EnvironmentIrradiance
        {
            const uint32_t IrradianceMapSize = Specification.GetIrradianceMapSize();
            const uint32_t IrradianceMapComputeSamples = Specification.IrradianceMapComputeSamples;
            LOG_TRACE("\tDispatching 'EnvironmentIrradiance'-{}...", Specification.EnvironmentMapName);

            TextureCubeSpecification IrradianceSpec =
            {
                ImageUtils::WrapMode::ClampToEdge,
                ImageUtils::WrapMode::ClampToEdge,
                ImageUtils::WrapMode::ClampToEdge,
                ImageUtils::FilterMode::LinearMipLinear,
                ImageUtils::FilterMode::Linear,
                ImageUtils::ImageInternalFormat::RGBA32F,
                ImageUtils::ImageDataLayout::RGBA,
                ImageUtils::ImageDataType::Float,
                IrradianceMapSize,
            };
            
            IrradianceSpec.Name = Specification.EnvironmentMapName + "-EnvironmentIrradianceCube";
            
            const glm::ivec3 IrradianceThreadGroups = glm::ivec3(
            glm::ceil(IrradianceSpec.Dimension / ThreadGroupSize),
            glm::ceil(IrradianceSpec.Dimension / ThreadGroupSize),
            6);

            const Ref<TextureCube> IrradianceMap = TextureLibrary::LoadTextureCube(IrradianceSpec, true);
            IrradianceMap->BindToImageSlot(0, 0, ImageUtils::TextureAccessLevel::WriteOnly, ImageUtils::TextureShaderDataFormat::RGBA32F);
            EnvironmentCubeFiltered->BindToSamplerSlot(0);
            ShaderLibrary::Get("EnvironmentIrradiance")->Bind();
            ShaderLibrary::Get("EnvironmentIrradiance")->UploadUniformInt("sampler_RadianceMap", 0);
            ShaderLibrary::Get("EnvironmentIrradiance")->UploadUniformInt("EnvironmentSamples", IrradianceMapComputeSamples);
            ShaderLibrary::Get("EnvironmentIrradiance")->DispatchCompute(IrradianceThreadGroups.x, IrradianceThreadGroups.y, IrradianceThreadGroups.z);

            LOG_TRACE("\t-----EnvironmentMapPipeline complete.  Radiance & Irradiance Maps are ready for use.-----");
        }
        // EnvironmentIrradiance
    }
}

