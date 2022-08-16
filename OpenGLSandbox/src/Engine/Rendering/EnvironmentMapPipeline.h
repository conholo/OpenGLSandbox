#pragma once
#include <functional>
#include <string>

#include "Texture.h"

namespace Engine
{
    class Shader;
    using DispatchCreateRadianceMapFn = std::function<void(const Ref<TextureCube>&, const Ref<TextureCube>&)>;
    
    
        struct EnvironmentMapSpecification
        {
            EnvironmentMapSpecification(
                std::string EnvironmentName,
                DispatchCreateRadianceMapFn PreDispatchFn,
                DispatchCreateRadianceMapFn PostDispatchFn,
                uint32_t EnvironmentMapResolution = 1024,
                uint32_t IrradianceComputeSamples = 512)
                :
            EnvironmentMapName(std::move(EnvironmentName)),
            EnvironmentMapResolution(EnvironmentMapResolution),
            IrradianceMapComputeSamples(IrradianceComputeSamples),
            PreDispatchFn(std::move(PreDispatchFn)),
            PostDispatchFn(std::move(PostDispatchFn))
            { }
            
            std::string EnvironmentMapName;
            uint32_t EnvironmentMapResolution = 1024;
            uint32_t IrradianceMapComputeSamples = 512;
            DispatchCreateRadianceMapFn PreDispatchFn;
            DispatchCreateRadianceMapFn PostDispatchFn;

            std::string GetEnvironmentRadianceCubeUnfilteredTextureName() const { return EnvironmentMapName + EnvironmentRadianceCubeUnfilteredSuffix; }
            std::string GetEnvironmentRadianceCubeFilteredTextureName() const { return EnvironmentMapName + EnvironmentRadianceCubeFilteredSuffix; }
            std::string GetEnvironmentIrradianceCubeTextureName() const { return EnvironmentMapName + EnvironmentIrradianceCubeSuffix; }

            uint32_t GetThreadGroupSize() const { return m_ThreadGroupSize; }
            uint32_t GetIrradianceMapSize() const { return m_IrradianceMapSize; }
        
        private:
            const std::string EnvironmentRadianceCubeUnfilteredSuffix = "-EnvironmentRadianceCubeUnfiltered";
            const std::string EnvironmentRadianceCubeFilteredSuffix = "-EnvironmentRadianceCubeFiltered";
            const std::string EnvironmentIrradianceCubeSuffix = "-EnvironmentIrradianceCube";
            const uint32_t m_ThreadGroupSize = 32;
            const uint32_t m_IrradianceMapSize = 32;
        };
    
    class EnvironmentMapPipeline
    {
    public:
        EnvironmentMapPipeline(const std::string& CreationShader, const Ref<EnvironmentMapSpecification>& Specification);
        EnvironmentMapPipeline(const std::string& filePath);
        ~EnvironmentMapPipeline();

        void Rebuild(const std::string& CreationShader, const Ref<EnvironmentMapSpecification>& Specification);
        const Ref<EnvironmentMapSpecification>& GetSpecification() const { return m_Specification; }
        
    private:
        void GenerateFromFile(const std::string& filePath);
        void GenerateFromShader(const std::string& CreationShader, const EnvironmentMapSpecification& Specification) const;

        Ref<EnvironmentMapSpecification> m_Specification;
    };
}

