
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

//#include "Layers/410/CloudsLayer.h"
//#include "Layers/410/Planet/PlanetLayer.h"
#include "Layers/AtmosphereLayer.h"

//#include "Layers/410/CurlTestLayer.h"
//#include "Layers/410/SkyVolumeGeneratorLayer.h"
//#include "Layers/410/TerrainTestLayer.h"
//#include "Layers/Testing/TestSSBOReadWriteLayer.h"

//#include "Layers/VolumetricLayer.h"
//#include "Layers/Testing/AssimpTestLayer.h"

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
		//PushLayer(new SkyVolumeGeneratorLayer);
		//PushLayer(new CurlTestLayer);
		//PushLayer(new TerrainTestLayer);

		//PushLayer(new CloudsLayer);
		//PushLayer(new TestSSBOReadWriteLayer);
		//PushLayer(new PlanetLayer);
		//PushLayer(new VolumetricLayer);

		//PushLayer(new AssimpTestLayer);
		PushLayer(new AtmosphereLayer);
	}
	
	~SandboxApplication()
	{

	}
};

Engine::Application* Engine::CreateApplication()
{
	return new SandboxApplication;
}