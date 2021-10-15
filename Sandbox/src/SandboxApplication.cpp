
#include "Engine.h"
#include "Engine/Core/EntryPoint.h"

//#include "Layers/ComputeTextureLayer.h"
//#include "Layers/ComputeParticleLayer.h"
#include "Layers/CubeMapLayer.h"

class SandboxApplication : public Engine::Application
{
public:
	SandboxApplication()
	:Engine::Application("Sandbox")
	{

		//PushLayer(new ComputeTextureLayer);
		//PushLayer(new ComputeParticleLayer);
		PushLayer(new CubeMapLayer);
	}
	
	~SandboxApplication()
	{

	}
};

Engine::Application* Engine::CreateApplication()
{
	return new SandboxApplication;
}