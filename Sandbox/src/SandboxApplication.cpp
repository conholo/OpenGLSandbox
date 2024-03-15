
#include "Engine.h"
#include "Engine/Core/EntryPoint.h"
//#include <Layers/BloomDemoLayer.h>
//#include <Layers/BlinnPhongLightingDemoLayer.h>
#include <Layers/SceneRendererPipelineDemoLayer.h>
//#include <Layers/TerrainDemoLayer.h>
//#include <Layers/VolumetricCloudsDemoLayer.h>


class SandboxApplication : public Engine::Application
{
public:
	SandboxApplication()
		:Engine::Application("Sandbox")
	{
		PushLayer(new SceneRendererPipelineDemoLayer);
	}
	
	~SandboxApplication()
	{

	}
};

Engine::Application* Engine::CreateApplication()
{
	return new SandboxApplication;
}