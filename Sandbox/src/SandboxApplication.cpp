
#include "Engine.h"
#include "Engine/Core/EntryPoint.h"

//#include "Layers/ComputeTextureLayer.h"
//#include "Layers/ComputeParticleLayer.h"
//#include "Layers/CubeMapLayer.h"
//#include "Layers/SkyboxLayer.h"
//#include "Layers/LightingLayer.h"
//#include "Layers/ToonShadingLayer.h"
//#include "Layers/ShaderProjectLayer.h"
//#include "Layers/ShadowsLayer.h"
//#include "Layers/FractalLayer.h"
//#include "Layers/LineLayer.h"
//#include "Layers/TestBezierSurfaceLayer.h"
//#include "Layers/Testing/ComputeTest.h"
//#include "Layers/Testing/StarMapLayer.h"

//#include "Layers/457/Project1.h"
#include "Layers/410/WorleyGeneratorLayer.h"

class SandboxApplication : public Engine::Application
{
public:
	SandboxApplication()
		:Engine::Application("Sandbox")
	{

		//PushLayer(new ComputeTextureLayer);
		//PushLayer(new ComputeParticleLayer);
		//PushLayer(new CubeMapLayer);
		//PushLayer(new SkyboxLayer);
		//PushLayer(new LightingLayer);
		//PushLayer(new ToonShadingLayer);
		//PushLayer(new ShaderProjectLayer);
		//PushLayer(new ShadowsLayer);
		//PushLayer(new FractalLayer);
		//PushLayer(new LineLayer);
		//PushLayer(new TestBezierSurfaceLayer);
		//PushLayer(new ComputeTestLayer);
		//PushLayer(new StarMapLayer);

		//PushLayer(new Project1);
		PushLayer(new WorleyGeneratorLayer);
	}
	
	~SandboxApplication()
	{

	}
};

Engine::Application* Engine::CreateApplication()
{
	return new SandboxApplication;
}