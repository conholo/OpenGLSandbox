#include "Layers/PragueSkyDemoLayer.h"
#include "imgui.h"

// Maximum distance to the edge of the atmosphere in the transmittance model
constexpr double DIST_TO_EDGE = 1571524.413613; 
#define PI 3.14159265359

PragueSkyDemoLayer::PragueSkyDemoLayer()
{
    
}

PragueSkyDemoLayer::~PragueSkyDemoLayer() = default;


void PragueSkyDemoLayer::InitializeViewParameters()
{
    Engine::Application::GetApplication().GetWindow().ToggleMaximize(true);
    m_ViewportWidth = Engine::Application::GetApplication().GetWindow().GetWidth();
    m_ViewportHeight = Engine::Application::GetApplication().GetWindow().GetHeight();
    m_Camera = Engine::Camera(45.0f, (float)m_ViewportWidth / (float)m_ViewportHeight, 1.0f, DIST_TO_EDGE);
    glm::vec3 CameraPosition { m_Camera.GetPosition() };
    m_Camera.SetPosition({20000.0f, 1.0f, CameraPosition.z});
}

void PragueSkyDemoLayer::LoadEntities()
{
    //Engine::Ref<Engine::Mesh> testMesh = Engine::CreateRef<Engine::Mesh>("assets/models/monster/munster/Plant Monster.obj");
    //Engine::Ref<Engine::Mesh> testMesh = Engine::CreateRef<Engine::Mesh>("assets/models/landscape/mountain.obj");
    //Engine::Ref<Engine::Mesh> testMesh = Engine::CreateRef<Engine::Mesh>("assets/models/plane/x-35_fbx.FBX");
    Engine::Ref<Engine::Mesh> testMesh = Engine::CreateRef<Engine::Mesh>("assets/models/tower-a.obj");
    for(int i = 0; i < 20; i++)
    {
        constexpr int Step = 20000;
        const Engine::Ref<Engine::SimpleEntity> Tower = m_Towers.emplace_back(Engine::CreateRef<Engine::SimpleEntity>(testMesh));
        Tower->GetEntityTransform()->SetScale({5.0f, 5.0f, 5.0f});
        Tower->GetEntityTransform()->SetRotation({0.0f, 45.0f, 0.0f});
        Tower->GetEntityTransform()->SetPosition({0.0f, 0.0f, -i * Step - Step});
    }
    
    {
        Engine::LightSpecification LightSpec = 
        {
            Engine::LightType::Directional,
            glm::vec3(1.0f),
            3.0f
        };
        m_Light = Engine::CreateRef<Engine::Light>(LightSpec);
    }
}

void PragueSkyDemoLayer::GenerateGlobalRenderResources()
{
    Engine::FramebufferSpecification DepthVisualizerSpec =
    {
        Engine::Application::GetApplication().GetWindow().GetWidth(), Engine::Application::GetApplication().GetWindow().GetHeight(),
        {Engine::FramebufferTextureFormat::RGBA32F}
    };
    Engine::FramebufferSpecification SkyRadianceSpec =
    {
        Engine::Application::GetApplication().GetWindow().GetWidth(), Engine::Application::GetApplication().GetWindow().GetHeight(),
        {Engine::FramebufferTextureFormat::RGBA32F, Engine::FramebufferTextureFormat::RGBA32F, Engine::FramebufferTextureFormat::RGBA32F}
    };

    Engine::FramebufferSpecification ScenePBRFBO =
    {
        Engine::Application::GetApplication().GetWindow().GetWidth(), Engine::Application::GetApplication().GetWindow().GetHeight(),
        { Engine::FramebufferTextureFormat::Depth, Engine::FramebufferTextureFormat::RGBA32F, Engine::FramebufferTextureFormat::RGBA32F }
    };

    m_SkyRadianceFBO = Engine::CreateRef<Engine::Framebuffer>(SkyRadianceSpec);
    m_ScenePBRFBO = Engine::CreateRef<Engine::Framebuffer>(ScenePBRFBO);

    m_DepthFBO = Engine::CreateRef<Engine::Framebuffer>(DepthVisualizerSpec);
    m_DepthVisualizerFSQ = Engine::CreateRef<Engine::SimpleEntity>(Engine::PrimitiveType::FullScreenQuad, "LinearDepthVisualizer");
    m_CompositeFSQ = Engine::CreateRef<Engine::SimpleEntity>(Engine::PrimitiveType::FullScreenQuad, "EngineSceneComposite");
}

void PragueSkyDemoLayer::InitializeEnvironmentAndAtmosphereResources()
{
    //m_EnvironmentMapPipeline = Engine::CreateRef<Engine::EnvironmentMapPipeline>("assets/textures/equi-test-cube.png");
    m_PragueDriver = Engine::CreateRef<PragueDriver>();
    m_PragueDriver->Execute(m_Camera, m_ScenePBRFBO);
    m_PreviousAltitude = m_PragueDriver->GetInput()->Altitude;

    const Engine::Ref<Engine::EnvironmentMapSpecification>& PipelineSpec = m_PragueDriver->GetEnvironmentMapPipeline()->GetSpecification();
    const std::string RadianceTextureName = PipelineSpec->GetEnvironmentRadianceCubeFilteredTextureName();
    const Engine::Ref<Engine::TextureCube> RadianceMap = Engine::TextureLibrary::GetCube(RadianceTextureName);
    m_Skybox = Engine::CreateRef<Engine::CubeMap>(RadianceMap, Engine::ShaderLibrary::Get("Skybox"));
}

void PragueSkyDemoLayer::OnAttach()
{
    InitializeViewParameters();
    LoadEntities();
    GenerateGlobalRenderResources();
    InitializeEnvironmentAndAtmosphereResources();
}

void PragueSkyDemoLayer::OnDetach()
{
    
}

void PragueSkyDemoLayer::TickAtmosphereParamsUpdate(float deltaTime)
{
    if(m_PreviousAltitude != m_PragueDriver->GetInput()->Altitude)
    {
        glm::vec3 CurrentPosition = m_Camera.GetPosition();

        m_PreviousAltitude = m_PragueDriver->GetInput()->Altitude;
        m_Camera.SetPosition({CurrentPosition.x, m_PreviousAltitude, CurrentPosition.z});
        if(m_LockCameraToAltitudeAndTarget)
        {
            glm::vec3 AverageTargetPosition{};
            for(const auto& Tower : m_Towers)
                AverageTargetPosition += Tower->GetEntityTransform()->GetPosition();

            AverageTargetPosition /= m_Towers.size();
            
            m_Camera.SetLookAtAndPosition({ CurrentPosition.x, m_PreviousAltitude, CurrentPosition.z }, AverageTargetPosition);
        }
    }

    if(m_RequestCameraReset)
    {
        m_Camera.SetDefaultView();
        m_RequestCameraReset = false;
    }

    if(m_AnimateAltitude)
    {
        m_PragueDriver->GetInput()->Altitude += deltaTime * m_AnimationSpeed;
        m_UpdateEnvironmentMaps = true;
    }
}

void PragueSkyDemoLayer::OnUpdate(float deltaTime)
{
    TickAtmosphereParamsUpdate(deltaTime);

    const float Azimuth = m_PragueDriver->GetInput()->Azimuth;
    const float Elevation = m_PragueDriver->GetInput()->Elevation;
    
    const glm::vec3 DirectionToLight =
    {
        sin(Azimuth) * cos(Elevation),
        sin(Elevation),
        -cos(Azimuth) * cos(Elevation)
    };

    m_Light->GetLightTransform()->SetPosition(DirectionToLight * 100000.0f);
    m_Camera.Update(deltaTime);

    // PBR Scene Geometry Pass (RGBA32F) && IBL Contribution (RGBA32F)
    m_ScenePBRFBO->Bind();
    {
        Engine::RenderCommand::ClearColor({ 0.0f, 0.0f, 0.0f, 0.0f });
        Engine::RenderCommand::Clear(true, true);

        for(const auto& Tower : m_Towers)
            Tower->Draw(m_Camera, m_Light);
    }
    m_ScenePBRFBO->Unbind();
    // PBR Scene Geometry Pass && IBL Contribution

    // Sky Radiance Pass (RGBA32F)
    m_SkyRadianceFBO->Bind();
    {
        Engine::RenderCommand::ClearColor({ 0.0f, 0.0f, 0.0f, 0.0f });
        Engine::RenderCommand::Clear(true, true);

        const Engine::Ref<Engine::EnvironmentMapSpecification>& PipelineSpec = m_PragueDriver->GetEnvironmentMapPipeline()->GetSpecification();
        
        const std::string UnfilteredRadianceTextureName = PipelineSpec->GetEnvironmentRadianceCubeUnfilteredTextureName();
        const Engine::Ref<Engine::TextureCube> UnfilteredRadianceMap = Engine::TextureLibrary::GetCube(UnfilteredRadianceTextureName);
        const std::string IrradianceTextureName = PipelineSpec->GetEnvironmentIrradianceCubeTextureName();
        const Engine::Ref<Engine::TextureCube> IrradianceMap = Engine::TextureLibrary::GetCube(IrradianceTextureName);

        UnfilteredRadianceMap->BindToSamplerSlot(1);
        IrradianceMap->BindToSamplerSlot(2);
        Engine::ShaderLibrary::Get("Skybox")->Bind();
        Engine::ShaderLibrary::Get("Skybox")->UploadUniformFloat3("u_LODs", m_EnvironmentMapSampleLODs);
        Engine::ShaderLibrary::Get("Skybox")->UploadUniformFloat3("u_Intensities", m_EnvironmentMapSampleIntensities);
        Engine::ShaderLibrary::Get("Skybox")->UploadUniformInt("u_UnfilteredRadianceMap", 1);
        Engine::ShaderLibrary::Get("Skybox")->UploadUniformInt("u_IrradianceMap", 2);
        const glm::mat4 View = glm::mat4(glm::mat3(m_Camera.GetView()));
        m_Skybox->Submit(m_Camera.GetProjection() * View);
    }
    m_SkyRadianceFBO->Unbind();
    // Sky Radiance Pass

    // Transmittance Pass (RGBA32F)
    if(m_UpdateEnvironmentMaps)
    {
        m_PragueDriver->Execute(m_Camera, m_ScenePBRFBO);
        m_UpdateEnvironmentMaps = false;
    }
    // Transmittance Pass

    // Optional Passes Begin
    {
        // Depth Visualization Pass (RGBA32F)
        m_DepthFBO->Bind();
        {
            m_ScenePBRFBO->BindDepthTexture(0);
            m_DepthVisualizerFSQ->GetEntityRenderer()->GetShader()->Bind();
            m_DepthVisualizerFSQ->GetEntityRenderer()->GetShader()->UploadUniformInt("u_DepthTexture", 0);
            m_DepthVisualizerFSQ->GetEntityRenderer()->GetShader()->UploadUniformFloat("u_Near", m_Camera.GetNearClip());
            m_DepthVisualizerFSQ->GetEntityRenderer()->GetShader()->UploadUniformFloat("u_Far", m_Camera.GetFarClip());
            m_DepthVisualizerFSQ->DrawEntity(m_Camera.GetViewProjection());
        }
        m_DepthFBO->Unbind();
        // Depth Visualization Pass
    }
    // Optional Passes End

    // Scene Composite Pass (Default FBO)
    {
        Engine::RenderCommand::ClearColor({ 0.0f, 0.0f, 0.0f, 0.0f });
        Engine::RenderCommand::Clear(true, true);
        m_SkyRadianceFBO->BindColorAttachment(0, 0);
        m_ScenePBRFBO->BindColorAttachment(0, 1);
        m_PragueDriver->BindTransmittanceAttachment(2);                                                
        m_ScenePBRFBO->BindColorAttachment(1, 3);
        m_PragueDriver->BindSunRadianceAttachment(4);                                                
        m_CompositeFSQ->GetEntityRenderer()->Bind();
        m_CompositeFSQ->GetEntityRenderer()->GetShader()->UploadUniformFloat("u_Exposure", m_Exposure);
        m_CompositeFSQ->GetEntityRenderer()->GetShader()->UploadUniformInt("u_ApplyTransmittance", m_ApplyTransmittance ? 1 : 0);
        m_CompositeFSQ->GetEntityRenderer()->GetShader()->UploadUniformInt("u_SkyRadianceMap", 0);
        m_CompositeFSQ->GetEntityRenderer()->GetShader()->UploadUniformInt("u_SceneRadianceMap", 1);
        m_CompositeFSQ->GetEntityRenderer()->GetShader()->UploadUniformInt("u_TransmittanceMap", 2);
        m_CompositeFSQ->GetEntityRenderer()->GetShader()->UploadUniformInt("u_SkyRadianceGeometryMap", 3);
        m_CompositeFSQ->GetEntityRenderer()->GetShader()->UploadUniformInt("u_SunRadianceMap", 4);
        m_CompositeFSQ->DrawEntity(m_Camera.GetViewProjection());
    }
    // Scene Composite Pass
}

void PragueSkyDemoLayer::OnEvent(Engine::Event& e)
{
    m_Camera.OnEvent(e);
}

void PragueSkyDemoLayer::OnImGuiRender()
{
    ImGui::Begin("Scene");
    if(ImGui::TreeNodeEx("Light Settings"))
    {
        ImGui::DragFloat3("Light Position", &m_Light->GetLightTransform()->GetPosition().x, 0.1f);
        ImGui::DragFloat3("Light Radiance", &m_Light->GetLightColor().x, 0.1f);
        ImGui::DragFloat("Light Intensity", &m_Light->GetLightIntensity(), 0.01f);
        ImGui::DragFloat("Exposure", &m_Exposure, 0.01f);
        ImGui::DragFloat("Environment Map Intensity", &Engine::EntityRenderer::s_EnvironmentMapIntensity, 0.01f);
        ImGui::TreePop();
    }

    if(ImGui::TreeNodeEx("Scene Settings"))
    {
        if(ImGui::Checkbox("Use Transmittance", &m_ApplyTransmittance))
            if(m_ApplyTransmittance)
                m_UpdateEnvironmentMaps = true;
        
        if(ImGui::Checkbox("Animate Altitude", &m_AnimateAltitude))
        {
            m_LockCameraToAltitudeAndTarget = m_AnimateAltitude;
            if(!m_LockCameraToAltitudeAndTarget)
                m_RequestCameraReset = true;
        }

        if(m_AnimateAltitude)
            m_UpdateEnvironmentMaps |= ImGui::DragFloat("Animation Speed", &m_AnimationSpeed);

        const Engine::Ref<Engine::EnvironmentMapSpecification>& PipelineSpec = m_PragueDriver->GetEnvironmentMapPipeline()->GetSpecification();
        
        const std::string FilteredRadianceTextureName = PipelineSpec->GetEnvironmentRadianceCubeFilteredTextureName();
        const Engine::Ref<Engine::TextureCube> FilteredRadianceMap = Engine::TextureLibrary::GetCube(FilteredRadianceTextureName);

        const std::string UnfilteredRadianceTextureName = PipelineSpec->GetEnvironmentRadianceCubeUnfilteredTextureName();
        const Engine::Ref<Engine::TextureCube> UnfilteredRadianceMap = Engine::TextureLibrary::GetCube(UnfilteredRadianceTextureName);

        const std::string IrradianceTextureName = PipelineSpec->GetEnvironmentIrradianceCubeTextureName();
        const Engine::Ref<Engine::TextureCube> IrradianceMap = Engine::TextureLibrary::GetCube(IrradianceTextureName);

        glm::ivec3 LODS = {m_EnvironmentMapSampleLODs.x, m_EnvironmentMapSampleLODs.y, m_EnvironmentMapSampleLODs.z};
        
        const uint32_t FilteredAvailableMips = FilteredRadianceMap->GetMipLevelCount();
        const uint32_t UnfilteredAvailableMips = UnfilteredRadianceMap->GetMipLevelCount();
        const uint32_t IrradianceAvailableMips = UnfilteredRadianceMap->GetMipLevelCount();
        ImGui::DragInt("Filtered Radiance LOD", &LODS.x, 0.1, 0, FilteredAvailableMips - 1);
        ImGui::DragInt("Unfiltered Radiance LOD", &LODS.y, 0.1, 0, UnfilteredAvailableMips - 1);
        ImGui::DragInt("Irradiance LOD", &LODS.z, 0.1, 0, IrradianceAvailableMips - 1);
        m_EnvironmentMapSampleLODs = LODS;
        
        m_UpdateEnvironmentMaps |= ImGui::Checkbox("Enable Grid", &m_GridEnabled);
        const bool UpdatedCameraLockToggle =  ImGui::Checkbox("Lock Camera to Altitude & Target", &m_LockCameraToAltitudeAndTarget);
        if(UpdatedCameraLockToggle && !m_LockCameraToAltitudeAndTarget)
            m_RequestCameraReset = true;
        
        ImGui::DragFloat3("Camera Position", &m_Camera.GetPosition().x);
        if(ImGui::Button("Move Camera to Altitude"))
        {
            m_Camera.SetPosition({m_Camera.GetPosition().x, m_PragueDriver->GetInput()->Altitude, m_Camera.GetPosition().z});
            m_UpdateEnvironmentMaps = true;
        }
        if(ImGui::Button("Move Camera to Origin"))
        {
            m_Camera.SetPosition({0.0f, 0.0f, 0.0f});
            m_UpdateEnvironmentMaps = true;
        }

        ImGui::TreePop();
    }
    
    ImGui::End();

    ImGui::Begin("Texture Viewer");
    if(ImGui::TreeNodeEx("A.) Filtered Sky Radiance"))
    {
        ImGui::Image(reinterpret_cast<ImTextureID>((m_SkyRadianceFBO->GetColorAttachmentID(0))), {480, 270}, {0, 1}, {1, 0});
        ImGui::TreePop();
    }
    if(ImGui::TreeNodeEx("B.) Unfiltered Sky Radiance"))
    {
        ImGui::Image(reinterpret_cast<ImTextureID>((m_SkyRadianceFBO->GetColorAttachmentID(1))), {480, 270}, {0, 1}, {1, 0});
        ImGui::TreePop();
    }
    if(ImGui::TreeNodeEx("C.) Irradiance"))
    {
        ImGui::Image(reinterpret_cast<ImTextureID>((m_SkyRadianceFBO->GetColorAttachmentID(2))), {480, 270}, {0, 1}, {1, 0});
        ImGui::TreePop();
    }
    if(ImGui::TreeNodeEx("D.) Geometry Sky Radiance"))
    {
        ImGui::Image(reinterpret_cast<ImTextureID>((m_ScenePBRFBO->GetColorAttachmentID(1))), {480, 270}, {0, 1}, {1, 0});
        ImGui::TreePop();
    }
    
    if(ImGui::TreeNodeEx("E.) Scene Geometry -> IBL + PBR"))
    {
        ImGui::Image(reinterpret_cast<ImTextureID>((m_ScenePBRFBO->GetColorAttachmentID(0))), {480, 270}, {0, 1}, {1, 0});
        ImGui::TreePop();
    }
    if(ImGui::TreeNodeEx("F.) Transmittance"))
    {
        ImGui::Image(reinterpret_cast<ImTextureID>(m_PragueDriver->GetTransmittanceAttachmentID()), {480, 270}, {0, 1}, {1, 0});
        ImGui::TreePop();
    }
    if(ImGui::TreeNodeEx("G.) Sun Radiance"))
    {
        ImGui::Image(reinterpret_cast<ImTextureID>(m_PragueDriver->GetSunRadianceAttachmentID()), {480, 270}, {0, 1}, {1, 0});
        ImGui::TreePop();
    }
    if(ImGui::TreeNodeEx("H.) Depth"))
    {
        ImGui::Image(reinterpret_cast<ImTextureID>(m_DepthFBO->GetColorAttachmentID()), {480, 270}, {0, 1}, {1, 0});
        ImGui::TreePop();
    }
    ImGui::End();
    
    m_PragueDriver->RenderUI(&m_UpdateEnvironmentMaps);
}