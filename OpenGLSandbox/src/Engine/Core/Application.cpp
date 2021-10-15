#include "Engine/Core/Application.h"
#include "Engine/Core/Utility.h"
#include "Engine/Core/Time.h"
#include "Engine/Rendering/RenderCommand.h"
#include "Engine/Rendering/Shader.h"
#include "Engine/Core/Random.h"

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
		RenderCommand::Initialize();
		RenderCommand::SetViewport(m_Window->GetWidth(), m_Window->GetHeight());
		Random::Initialize();

		Engine::ShaderLibrary::Load("assets/shaders/FlatColor.shader");
		Engine::ShaderLibrary::Load("assets/shaders/TestCompute.shader");
		Engine::ShaderLibrary::Load("assets/shaders/ParticleCompute.shader");
		Engine::ShaderLibrary::Load("assets/shaders/Particle.shader");
		Engine::ShaderLibrary::Load("assets/shaders/SkyboxTest.shader");
		Engine::ShaderLibrary::Load("assets/shaders/EnvironmentReflection.shader");
		Engine::ShaderLibrary::Load("assets/shaders/Preetham.shader");
	}

	Application::~Application()
	{
	}

	void Application::Run()
	{
		while (m_IsRunning)
		{
			Time::Tick();
			for (auto* layer : m_LayerStack)
				layer->OnUpdate(Time::DeltaTime());

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

		return true;
	}
}
