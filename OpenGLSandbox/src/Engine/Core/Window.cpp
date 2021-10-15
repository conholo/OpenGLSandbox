#include "Engine/Core/Window.h"
#include "Engine/Event/MouseEvent.h"
#include "Engine/Event/KeyEvent.h"
#include "Engine/Event/WindowEvent.h"

#include <iostream>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

namespace Engine
{
	static void GLFWErrorCallback(int error, const char* description)
	{
	}


	Window::Window(const std::string& name, uint32_t width, uint32_t height)
		:m_WindowData({ name, width, height })
	{
		Initialize();
	}

	Window::~Window()
	{
		Shutdown();
	}

	void Window::Initialize()
	{
		if (!glfwInit())
		{
			std::cout << "Failed to initialize GLFW" << std::endl;
			return;
		}

		glfwSetErrorCallback(GLFWErrorCallback);
		glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);
		m_WindowHandle = glfwCreateWindow(m_WindowData.Width, m_WindowData.Height, m_WindowData.Name.c_str(), nullptr, nullptr);

		glfwMakeContextCurrent(m_WindowHandle);

		int gladStatus = gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);

		if (!gladStatus)
		{
			std::cout << "Failed to initialize OpenGL context" << std::endl;
			return;
		}

		SetVSync(m_WindowData.VSync);
		glfwSetWindowUserPointer(m_WindowHandle, static_cast<void*>(&m_WindowData));

		glfwSetWindowCloseCallback(m_WindowHandle, [](GLFWwindow* window)
			{
				WindowData& data = *(static_cast<WindowData*>(glfwGetWindowUserPointer(window)));

				WindowClosedEvent windowClosedEvent;
				data.Callback(windowClosedEvent);
			});

		glfwSetWindowSizeCallback(m_WindowHandle, [](GLFWwindow* window, int width, int height)
			{
				WindowData& data = *(static_cast<WindowData*>(glfwGetWindowUserPointer(window)));
				data.Width = width;
				data.Height = height;

				WindowResizedEvent windowResizedEvent(width, height);
				data.Callback(windowResizedEvent);
			});

		glfwSetWindowPosCallback(m_WindowHandle, [](GLFWwindow* window, int xPosition, int yPosition)
			{
				WindowData& data = *(static_cast<WindowData*>(glfwGetWindowUserPointer(window)));

				WindowMovedEvent windowMovedEvent(xPosition, yPosition);
				data.Callback(windowMovedEvent);
			});

		glfwSetKeyCallback(m_WindowHandle, [](GLFWwindow* window, int keyCode, int scanCode, int action, int mods)
			{
				WindowData& data = *(static_cast<WindowData*>(glfwGetWindowUserPointer(window)));

				switch (action)
				{
				case GLFW_PRESS:
				{
					KeyPressedEvent keyPressedEvent(keyCode, 0);
					data.Callback(keyPressedEvent);
					break;
				}
				case GLFW_REPEAT:
				{
					KeyPressedEvent keyPressedEvent(keyCode, 1);
					data.Callback(keyPressedEvent);
					break;
				}
				case GLFW_RELEASE:
				{
					KeyReleasedEvent keyReleasedEvent(keyCode);
					data.Callback(keyReleasedEvent);
					break;
				}
				}
			});

		glfwSetMouseButtonCallback(m_WindowHandle, [](GLFWwindow* window, int button, int action, int mods)
			{
				WindowData& data = *(static_cast<WindowData*>(glfwGetWindowUserPointer(window)));

				switch (action)
				{
				case GLFW_PRESS:
				{
					MouseButtonPressedEvent mouseButtonPressedEvent(button);
					data.Callback(mouseButtonPressedEvent);
					break;
				}
				case GLFW_RELEASE:
				{
					MouseButtonReleasedEvent mouseButtonReleasedEvent(button);
					data.Callback(mouseButtonReleasedEvent);
					break;
				}
				}
			});

		glfwSetScrollCallback(m_WindowHandle, [](GLFWwindow* window, double xOffset, double yOffset)
			{
				WindowData& data = *(static_cast<WindowData*>(glfwGetWindowUserPointer(window)));

				MouseScrolledEvent mouseScrolledEvent(static_cast<float>(xOffset), static_cast<float>(yOffset));
				data.Callback(mouseScrolledEvent);
			});


		glfwSetCursorPosCallback(m_WindowHandle, [](GLFWwindow* window, double xPosition, double yPosition)
			{
				WindowData& data = *(static_cast<WindowData*>(glfwGetWindowUserPointer(window)));

				MouseMovedEvent mouseMovedEvent(static_cast<float>(xPosition), static_cast<float>(yPosition));
				data.Callback(mouseMovedEvent);
			});
	}

	void Window::Update()
	{
		glfwPollEvents();
		glfwSwapBuffers(m_WindowHandle);
	}

	void Window::SetVSync(bool enable)
	{
		glfwSwapInterval(enable ? 1 : 0);
		m_WindowData.VSync = enable;
	}

	void Window::Shutdown()
	{
		glfwDestroyWindow(m_WindowHandle);
		glfwTerminate();
	}
}

