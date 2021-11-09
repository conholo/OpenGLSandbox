#pragma once

#include "Engine/Core/Window.h"
#include "Engine/Event/Event.h"
#include "Engine/Event/WindowEvent.h"
#include "Engine/Core/LayerStack.h"
#include "Engine/Core/Memory.h"
#include "Engine/ImGui/ImGuiLayer.h"

namespace Engine
{
	class Application
	{
	public:
		Application(const std::string& name);
		virtual ~Application();

		void Run();
		static void Close();
		void OnEvent(Event& event);

		static Application& GetApplication() { return *s_Instance; }
		Window& GetWindow() const { return *m_Window; }
		const std::string& GetName() const { return m_Name; }

		ImGuiLayer* GetImGuiLayer() { return m_ImGuiLayer; }

		void PushLayer(Layer* layer);
		void PopLayer(Layer* layer);

	private:
		bool OnWindowClose(WindowClosedEvent& windowCloseEvent);
		bool OnWindowResize(WindowResizedEvent& windowResizeEvent);

	private:
		static Application* s_Instance;

	private:
		std::string m_Name;
		bool m_IsRunning = true;
		Scope<Window> m_Window;
		ImGuiLayer* m_ImGuiLayer;
		LayerStack m_LayerStack;
	};

	Application* CreateApplication();
}