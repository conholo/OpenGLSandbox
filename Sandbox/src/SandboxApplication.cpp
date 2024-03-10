#include "Engine.h"
#include "Engine/Core/EntryPoint.h"
#include <Layers/BloomDemoLayer.h>


class SandboxApplication : public Engine::Application
{
public:
	SandboxApplication()
		:Engine::Application("Sandbox")
	{
		PushLayer(new BloomDemoLayer);
	}
	
	~SandboxApplication()
	{

	}
};

Engine::Application* Engine::CreateApplication()
{
	return new SandboxApplication;
}