#pragma once
#include <stdint.h>
#include <string>

#include <functional>

#include "Engine/Event/Event.h"

struct GLFWwindow;

namespace Engine
{
	class Window
	{
	public:

		using EventCallbackFunction = std::function<void(Event&)>;

		Window(const std::string& name = "Ohm Engine", uint32_t width = 1920, uint32_t height = 1080, bool fullScreen = false, bool maximized = false);
		~Window();

		void Update();
		void SetEventCallbackFunction(const EventCallbackFunction& callback) { m_WindowData.Callback = callback; }
		void ToggleMaximize(bool maximize);

		uint32_t GetWidth() const { return m_WindowData.Width; }
		uint32_t GetHeight() const { return m_WindowData.Height; }
		float GetAspectRatio() const { return (float)m_WindowData.Width / (float)m_WindowData.Height; }

		void SetVSync(bool enable);
		bool IsVSync() const { return m_WindowData.VSync; }

		GLFWwindow* GetWindowHandle() const { return m_WindowHandle; }

	private:
		void Initialize();
		void Shutdown();

	private:
		GLFWwindow* m_WindowHandle;

		struct WindowData
		{
			std::string Name;
			uint32_t Width, Height;
			EventCallbackFunction Callback;
			bool FullScreenBorderless;
			bool Maximized;
			bool VSync;


			WindowData(const std::string& name = "Ohm Engine", uint32_t width = 1920, uint32_t height = 1080, bool fullScreen = false, bool maximized = false, bool vSync = true)
				:Name(name), Width(width), Height(height), FullScreenBorderless(fullScreen), Maximized(maximized), VSync(vSync)
			{
			}
		};

		WindowData m_WindowData;
	};
}
