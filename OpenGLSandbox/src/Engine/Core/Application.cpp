#include "Engine/Core/Application.h"
#include "Engine/Core/Utility.h"
#include "Engine/Core/Time.h"
#include "Engine/Rendering/RenderCommand.h"
#include "Engine/Rendering/Shader.h"
#include "Engine/Core/Random.h"
#include "Engine/Rendering/Renderer.h"

#include <GLFW/glfw3.h>

#include "Engine/Rendering/Texture.h"

namespace Engine
{
	Application* Application::s_Instance = nullptr;

	Application::Application(const std::string& name)
		:m_Name(name)
	{
		s_Instance = this;
		m_Window = CreateScope<Window>(name);
		m_Window->SetEventCallbackFunction(BIND_FN(Application::OnEvent));
		m_ImGuiLayer = new ImGuiLayer;

		PushLayer(m_ImGuiLayer);

		ShaderLibrary::Load("assets/shaders/PBR/Preetham.glsl");
		ShaderLibrary::Load("assets/shaders/PBR/Skybox.glsl");
		ShaderLibrary::Load("assets/shaders/PBR/EnginePBR.glsl");
		ShaderLibrary::Load("assets/shaders/PBR/EngineSceneComposite.glsl");
		ShaderLibrary::Load("assets/shaders/PBR/EquirectangularToCubemap.glsl");
		ShaderLibrary::Load("assets/shaders/PBR/EnvironmentMipFilter.glsl");
		ShaderLibrary::Load("assets/shaders/PBR/EnvironmentIrradiance.glsl");
		ShaderLibrary::Load("assets/shaders/Utility/LinearDepthVisualizer.glsl");
		ShaderLibrary::Load("assets/shaders/Utility/WriteCubemapToAttachment.glsl");
		
		ShaderLibrary::Load("assets/shaders/Misc/FlatColor.glsl");
		ShaderLibrary::Load("assets/shaders/Editor/InfiniteGrid.glsl");
		ShaderLibrary::Load("assets/shaders/PostFX/BloomPostProcessing.glsl");
		ShaderLibrary::Load("assets/shaders/PostFX/Bloom.glsl");
		ShaderLibrary::Load("assets/shaders/Depth/DebugDepth.glsl");
		ShaderLibrary::Load("assets/shaders/Depth/Depth.glsl");


		TextureLibrary::AddTexture2D(Texture2D::CreateWhiteTexture());

		Texture2DSpecification BRDFSpec =
		{
			Engine::ImageUtils::WrapMode::ClampToEdge,
			Engine::ImageUtils::WrapMode::ClampToEdge,
			Engine::ImageUtils::FilterMode::Linear,
			Engine::ImageUtils::FilterMode::Linear,
			Engine::ImageUtils::ImageInternalFormat::FromImage,
			Engine::ImageUtils::ImageDataLayout::FromImage,
			Engine::ImageUtils::ImageDataType::UByte,
		};
		BRDFSpec.Name = "BRDF LUT";
		TextureLibrary::LoadTexture2D(BRDFSpec, "assets/textures/BRDF LUT.png");
		
		RenderCommand::Initialize();
		RenderCommand::SetViewport(m_Window->GetWidth(), m_Window->GetHeight());
		Renderer::Initialize();
		Random::Initialize();
	}

	Application::~Application()
	{
		Renderer::Shutdown();
	}

	void Application::Run()
	{
		while (m_IsRunning)
		{
			Time::Tick();
			for (auto* layer : m_LayerStack)
				layer->OnUpdate(Time::DeltaTime());

			m_ImGuiLayer->Begin();
			for (Layer* layer : m_LayerStack)
				layer->OnImGuiRender();
			m_ImGuiLayer->End();

			m_Window->Update();
		}
	}

	void Application::Close()
	{
		s_Instance->m_IsRunning = false;
	}

	void Application::OnEvent(Event& event)
	{
		EventDispatcher dispatcher(event);

		m_ImGuiLayer->OnEvent(event);

		dispatcher.Dispatch<WindowClosedEvent>(BIND_FN(Application::OnWindowClose));
		dispatcher.Dispatch<WindowResizedEvent>(BIND_FN(Application::OnWindowResize));

		for (auto it = m_LayerStack.rbegin(); it != m_LayerStack.rend(); ++it)
		{
			if (event.Handled)
				break;
			(*it)->OnEvent(event);
		}
	}

	void Application::PushLayer(Layer* layer)
	{
		m_LayerStack.PushLayer(layer);
		layer->OnAttach();
	}

	void Application::PopLayer(Layer* layer)
	{
		m_LayerStack.PopLayer(layer);
	}

	bool Application::OnWindowClose(WindowClosedEvent& windowCloseEvent)
	{
		m_IsRunning = false;
		return true;
	}

	bool Application::OnWindowResize(WindowResizedEvent& windowResizeEvent)
	{
		RenderCommand::SetViewport(windowResizeEvent.GetWidth(), windowResizeEvent.GetHeight());

		return false;
	}
}
