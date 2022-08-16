
#include "Engine.h"
#include "Engine/Core/EntryPoint.h"
#include "Layers/PragueSkyDemoLayer.h"

class SandboxApplication : public Engine::Application
{
public:
	SandboxApplication()
		:Engine::Application("Sandbox")
	{
		PushLayer(new PragueSkyDemoLayer);
	}
	
	~SandboxApplication()
	{

	}
};

Engine::Application* Engine::CreateApplication()
{
	return new SandboxApplication;
}