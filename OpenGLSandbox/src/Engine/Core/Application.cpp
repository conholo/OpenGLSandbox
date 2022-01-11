#include "Engine/Core/Application.h"
#include "Engine/Core/Utility.h"
#include "Engine/Core/Time.h"
#include "Engine/Rendering/RenderCommand.h"
#include "Engine/Rendering/Shader.h"
#include "Engine/Core/Random.h"
#include "Engine/Rendering/Renderer.h"

#include <GLFW/glfw3.h>

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

		Engine::ShaderLibrary::Load("assets/shaders/FlatColor.shader");
		Engine::ShaderLibrary::Load("assets/shaders/TestCompute.shader");
		Engine::ShaderLibrary::Load("assets/shaders/ParticleCompute.shader");
		Engine::ShaderLibrary::Load("assets/shaders/Particle.shader");
		Engine::ShaderLibrary::Load("assets/shaders/SkyboxTest.shader");
		Engine::ShaderLibrary::Load("assets/shaders/EnvironmentReflection.shader");
		Engine::ShaderLibrary::Load("assets/shaders/Preetham.shader");
		Engine::ShaderLibrary::Load("assets/shaders/TestWriteToCube.shader");
		Engine::ShaderLibrary::Load("assets/shaders/Skybox.shader");
		Engine::ShaderLibrary::Load("assets/shaders/Grid.shader");
		Engine::ShaderLibrary::Load("assets/shaders/InfiniteGrid.shader");
		Engine::ShaderLibrary::Load("assets/shaders/Test.shader");
		Engine::ShaderLibrary::Load("assets/shaders/BlinnPhong.shader");
		Engine::ShaderLibrary::Load("assets/shaders/SDFBlinnPhong.shader");
		Engine::ShaderLibrary::Load("assets/shaders/Portal.shader");
		Engine::ShaderLibrary::Load("assets/shaders/NormalMapExample.shader");
		Engine::ShaderLibrary::Load("assets/shaders/ToonShader.shader");
		Engine::ShaderLibrary::Load("assets/shaders/BlinnPhongWS.shader");
		Engine::ShaderLibrary::Load("assets/shaders/Surfaces.shader");
		Engine::ShaderLibrary::Load("assets/shaders/DebugDepth.shader");
		Engine::ShaderLibrary::Load("assets/shaders/Depth.shader");
		Engine::ShaderLibrary::Load("assets/shaders/EngineBP.shader");
		//Engine::ShaderLibrary::Load("assets/shaders/Fractal.shader");
		Engine::ShaderLibrary::Load("assets/shaders/LineShader.shader");
		Engine::ShaderLibrary::Load("assets/shaders/LinePointShader.shader");
		Engine::ShaderLibrary::Load("assets/shaders/PostProcessing.shader");
		Engine::ShaderLibrary::Load("assets/shaders/Bloom.shader");
		Engine::ShaderLibrary::Load("assets/shaders/Bloom2.shader");
		Engine::ShaderLibrary::Load("assets/shaders/SurfaceCurve.shader");

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
