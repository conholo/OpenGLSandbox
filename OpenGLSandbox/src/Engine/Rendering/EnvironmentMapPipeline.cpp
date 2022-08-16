#include "EnvironmentMapPipeline.h"

#include <iostream>

#include "Shader.h"

namespace Engine
{
    EnvironmentMapPipeline::EnvironmentMapPipeline(const std::string& CreationShader, const Ref<EnvironmentMapSpecification>& Specification)
        :m_Specification(Specification)
    {
        GenerateFromShader(CreationShader, *Specification);
    }

    EnvironmentMapPipeline::EnvironmentMapPipeline(const std::string& filePath)
    {
        GenerateFromFile(filePath);
    }

    EnvironmentMapPipeline::~EnvironmentMapPipeline() = default;

    void EnvironmentMapPipeline::Rebuild(const std::string& CreationShader, const Ref<EnvironmentMapSpecification>& Specification)
    {
        m_Specification = Specification;
        GenerateFromShader(CreationShader, *Specification);
    }

    void EnvironmentMapPipeline::GenerateFromFile(const std::string& filePath)
    {
        std::cout << "-----Starting Environment Map Pipeline using file '" << filePath << "'...-----" << "\n";
        const Texture2DSpecification Specification
        {
            ImageUtils::WrapMode::Repeat,
            ImageUtils::WrapMode::Repeat,
            ImageUtils::FilterMode::Linear,
            ImageUtils::FilterMode::Linear,
            ImageUtils::ImageInternalFormat::FromImage,
            ImageUtils::ImageDataLayout::FromImage,
            ImageUtils::ImageDataType::UByte,
        };
        
        const Ref<Texture2D> Equirectangular = TextureLibrary::LoadTexture2D(Specification, filePath);

        if(!Equirectangular)
        {
            std::cout << "\tEnvironment Pipeline Error: Unable to create Equirectangular Texture." << "\n";
            return;
        }

        uint32_t EnvironmentMapResolution = m_Specification->EnvironmentMapResolution;
        uint32_t ThreadGroupSize = m_Specification->GetThreadGroupSize();
        
        TextureSpecification EnvironmentCubeSpec =
        {
            ImageUtils::Usage::Storage,
            ImageUtils::WrapMode::Repeat,
            ImageUtils::FilterMode::LinearMipLinear,
            ImageUtils::ImageInternalFormat::RGBA32F,
            ImageUtils::ImageDataLayout::RGBA,
            ImageUtils::ImageDataType::Float,
            EnvironmentMapResolution, EnvironmentMapResolution,
        };
        const glm::ivec3 ThreadGroups = glm::ivec3(
            glm::ceil(EnvironmentCubeSpec.Width / ThreadGroupSize),
            glm::ceil(EnvironmentCubeSpec.Height / ThreadGroupSize),
            6);

        EnvironmentCubeSpec.Name = "EnvironmentRadianceCubeUnfiltered";
        const Ref<TextureCube> EnvironmentCubeUnfiltered = TextureLibrary::LoadTextureCube(EnvironmentCubeSpec);
        EnvironmentCubeSpec.Name = "EnvironmentRadianceCubeFiltered";
        const Ref<TextureCube> EnvironmentCubeFiltered = TextureLibrary::LoadTextureCube(EnvironmentCubeSpec);

        // EquirectangularToCubemap
        {
            std::cout << "\tDispatching 'EquirectangularToCubemap' Shader..." << "\n";
            EnvironmentCubeUnfiltered->BindToImageSlot(0, 0, ImageUtils::TextureAccessLevel::WriteOnly, ImageUtils::TextureShaderDataFormat::RGBA32F);
            ShaderLibrary::Get("EquirectangularToCubemap")->Bind();
            ShaderLibrary::Get("EquirectangularToCubemap")->UploadUniformInt("sampler_EquirectangularTexture", 0);
            ShaderLibrary::Get("EquirectangularToCubemap")->DispatchCompute(ThreadGroups.x, ThreadGroups.y, ThreadGroups.z);
            Shader::EnableAllBarriersBits();
        }
        // EquirectangularToCubemap

        // EnvironmentFilter
        {
            const uint32_t MipCount = TextureCube::CalculateMipCount(EnvironmentMapResolution, EnvironmentMapResolution);
            const float deltaRoughness = 1.0f / glm::max(static_cast<float>(MipCount) - 1.0f, 1.0f);
            
            for(uint32_t i = 0, size = EnvironmentMapResolution; i < MipCount; i++, size /= 2)
            {
                std::cout << "\tDispatching 'EnvironmentFilter' at the " << i << "th mip of the unfiltered environment map..." "\n";
                const uint32_t ThreadGroup = glm::max(1u, size / 32);
                float Roughness = i * deltaRoughness;
                Roughness = glm::max(Roughness, 0.05f);
                
                EnvironmentCubeFiltered->BindToImageSlot(0, i, ImageUtils::TextureAccessLevel::WriteOnly, ImageUtils::TextureShaderDataFormat::RGBA32F);
                EnvironmentCubeUnfiltered->BindToSamplerSlot(0);
                ShaderLibrary::Get("EnvironmentMipFilter")->Bind();
                ShaderLibrary::Get("EnvironmentMipFilter")->UploadUniformInt("sampler_InputCube", 0);
                ShaderLibrary::Get("EnvironmentMipFilter")->UploadUniformInt("MipOutputWidth", size);
                ShaderLibrary::Get("EnvironmentMipFilter")->UploadUniformInt("MipOutputHeight", size);
                ShaderLibrary::Get("EnvironmentMipFilter")->UploadUniformFloat("Roughness", Roughness);
                ShaderLibrary::Get("EnvironmentMipFilter")->DispatchCompute(ThreadGroup, ThreadGroup, ThreadGroups.z);
                Shader::EnableAllBarriersBits();
            }
        }
        // EnvironmentFilter

        // EnvironmentIrradiance
        {
            uint32_t IrradianceMapSize = m_Specification->GetIrradianceMapSize();
            uint32_t IrradianceMapComputeSamples = m_Specification->IrradianceMapComputeSamples;
            std::cout << "\tDispatching 'EnvironmentIrradiance'..." "\n";

            const TextureSpecification IrradianceSpec =
            {
                ImageUtils::Usage::Storage,
                ImageUtils::WrapMode::Repeat,
                ImageUtils::FilterMode::LinearMipLinear,
                ImageUtils::ImageInternalFormat::RGBA32F,
                ImageUtils::ImageDataLayout::RGBA,
                ImageUtils::ImageDataType::Float,
                IrradianceMapSize, IrradianceMapSize,
                "EnvironmentIrradianceCube"
            };

            
            const glm::ivec3 IrradianceThreadGroups = glm::ivec3(
            glm::ceil(IrradianceSpec.Width / ThreadGroupSize),
            glm::ceil(IrradianceSpec.Height / ThreadGroupSize),
            6);

            Ref<TextureCube> IrradianceMap = TextureLibrary::LoadTextureCube(IrradianceSpec);
            IrradianceMap->BindToImageSlot(0, 0, ImageUtils::TextureAccessLevel::WriteOnly, ImageUtils::TextureShaderDataFormat::RGBA32F);
            EnvironmentCubeFiltered->BindToSamplerSlot(0);
            ShaderLibrary::Get("EnvironmentIrradiance")->Bind();
            ShaderLibrary::Get("EnvironmentIrradiance")->UploadUniformInt("sampler_RadianceMap", 0);
            ShaderLibrary::Get("EnvironmentIrradiance")->UploadUniformInt("EnvironmentSamples", IrradianceMapComputeSamples);
            ShaderLibrary::Get("EnvironmentIrradiance")->DispatchCompute(IrradianceThreadGroups.x, IrradianceThreadGroups.y, IrradianceThreadGroups.z);
        }
        // EnvironmentIrradiance

        std::cout << "-----EnvironmentMapPipeline complete.  Radiance & Irradiance Maps are ready for use.-----" << "\n";
    }

    void EnvironmentMapPipeline::GenerateFromShader(const std::string& CreationShader, const EnvironmentMapSpecification& Specification) const
    {
        const uint32_t EnvironmentMapResolution = Specification.EnvironmentMapResolution;
        const uint32_t ThreadGroupSize = Specification.GetThreadGroupSize();

        TextureSpecification EnvironmentCubeSpec =
        {
            ImageUtils::Usage::Storage,
            ImageUtils::WrapMode::Repeat,
            ImageUtils::FilterMode::LinearMipLinear,
            ImageUtils::ImageInternalFormat::RGBA32F,
            ImageUtils::ImageDataLayout::RGBA,
            ImageUtils::ImageDataType::Float,
            EnvironmentMapResolution, EnvironmentMapResolution,
        };
        
        const glm::ivec3 ThreadGroups = glm::ivec3(
            glm::ceil(EnvironmentCubeSpec.Width / ThreadGroupSize),
            glm::ceil(EnvironmentCubeSpec.Height / ThreadGroupSize),
            6);

        // Recreate Cubemap/Skybox specific resources
        EnvironmentCubeSpec.Name = Specification.EnvironmentMapName + "-EnvironmentRadianceCubeUnfiltered";
        const Ref<TextureCube> EnvironmentCubeUnfiltered = TextureLibrary::LoadTextureCube(EnvironmentCubeSpec);
        EnvironmentCubeSpec.Name = Specification.EnvironmentMapName + "-EnvironmentRadianceCubeFiltered";
        const Ref<TextureCube> EnvironmentCubeFiltered = TextureLibrary::LoadTextureCube(EnvironmentCubeSpec);

        // User Shader Radiance Cubemap Creation
        {
            ShaderLibrary::Get(CreationShader)->Bind();
            Specification.PreDispatchFn(EnvironmentCubeUnfiltered, EnvironmentCubeFiltered);
            EnvironmentCubeUnfiltered->BindToImageSlot(0, 0, ImageUtils::TextureAccessLevel::WriteOnly, ImageUtils::TextureShaderDataFormat::RGBA32F);
            ShaderLibrary::Get(CreationShader)->DispatchCompute(ThreadGroups.x, ThreadGroups.y, ThreadGroups.z);
            Shader::EnableAllBarriersBits();
            Specification.PostDispatchFn(EnvironmentCubeUnfiltered, EnvironmentCubeFiltered);
        }
        // User Shader Radiance Cubemap Creation

        // EnvironmentFilter
        {
            const uint32_t MipCount = TextureCube::CalculateMipCount(EnvironmentMapResolution, EnvironmentMapResolution);
            const float deltaRoughness = 1.0f / glm::max(static_cast<float>(MipCount) - 1.0f, 1.0f);
            
            for(uint32_t i = 0, size = EnvironmentMapResolution; i < MipCount; i++, size /= 2)
            {
                std::cout << "\tDispatching 'EnvironmentFilter: " << Specification.EnvironmentMapName << "' at the " << i << "th mip of the unfiltered environment map..." "\n";
                const uint32_t ThreadGroup = glm::max(1u, size / 32);
                float Roughness = i * deltaRoughness;
                Roughness = glm::max(Roughness, 0.05f);
                
                EnvironmentCubeFiltered->BindToImageSlot(0, i, ImageUtils::TextureAccessLevel::WriteOnly, ImageUtils::TextureShaderDataFormat::RGBA32F);
                EnvironmentCubeUnfiltered->BindToSamplerSlot(0);
                ShaderLibrary::Get("EnvironmentMipFilter")->Bind();
                ShaderLibrary::Get("EnvironmentMipFilter")->UploadUniformInt("sampler_InputCube", 0);
                ShaderLibrary::Get("EnvironmentMipFilter")->UploadUniformInt("MipOutputWidth", size);
                ShaderLibrary::Get("EnvironmentMipFilter")->UploadUniformInt("MipOutputHeight", size);
                ShaderLibrary::Get("EnvironmentMipFilter")->UploadUniformFloat("Roughness", Roughness);
                ShaderLibrary::Get("EnvironmentMipFilter")->DispatchCompute(ThreadGroup, ThreadGroup, ThreadGroups.z);
                Shader::EnableAllBarriersBits();
            }
        }
        // EnvironmentFilter

        // EnvironmentIrradiance
        {
            const uint32_t IrradianceMapSize = Specification.GetIrradianceMapSize();
            const uint32_t IrradianceMapComputeSamples = Specification.IrradianceMapComputeSamples;
            std::cout << "\tDispatching 'EnvironmentIrradiance'..." "\n";

            TextureSpecification IrradianceSpec =
            {
                ImageUtils::Usage::Storage,
                ImageUtils::WrapMode::Repeat,
                ImageUtils::FilterMode::LinearMipLinear,
                ImageUtils::ImageInternalFormat::RGBA32F,
                ImageUtils::ImageDataLayout::RGBA,
                ImageUtils::ImageDataType::Float,
                IrradianceMapSize, IrradianceMapSize
            };
            IrradianceSpec.Name = Specification.EnvironmentMapName + "-EnvironmentIrradianceCube";

            
            const glm::ivec3 IrradianceThreadGroups = glm::ivec3(
            glm::ceil(IrradianceSpec.Width / ThreadGroupSize),
            glm::ceil(IrradianceSpec.Height / ThreadGroupSize),
            6);

            const Ref<TextureCube> IrradianceMap = TextureLibrary::LoadTextureCube(IrradianceSpec);
            IrradianceMap->BindToImageSlot(0, 0, ImageUtils::TextureAccessLevel::WriteOnly, ImageUtils::TextureShaderDataFormat::RGBA32F);
            EnvironmentCubeFiltered->BindToSamplerSlot(0);
            ShaderLibrary::Get("EnvironmentIrradiance")->Bind();
            ShaderLibrary::Get("EnvironmentIrradiance")->UploadUniformInt("sampler_RadianceMap", 0);
            ShaderLibrary::Get("EnvironmentIrradiance")->UploadUniformInt("EnvironmentSamples", IrradianceMapComputeSamples);
            ShaderLibrary::Get("EnvironmentIrradiance")->DispatchCompute(IrradianceThreadGroups.x, IrradianceThreadGroups.y, IrradianceThreadGroups.z);
            Shader::EnableAllBarriersBits();
        }
        // EnvironmentIrradiance

    }
}

