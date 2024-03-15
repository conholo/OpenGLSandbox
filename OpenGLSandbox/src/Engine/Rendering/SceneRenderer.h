#pragma once

#include "Engine/Scene/Scene.h"
#include "Engine/Scene/Entity.h"
#include "Engine/Rendering/RenderPass.h"
#include "Engine/Rendering/Camera.h"
#include "Engine/Rendering/UniformBuffer.h"
#include "Engine/Event/Event.h"
#include "Engine/Rendering/Texture.h"

namespace Engine
{
	class SceneRenderer
	{
	public:
		static void LoadScene(const Ref<Scene>& runtimeScene);
		static void UnloadScene();

		static void UpdateCamera(float deltaTime);

		static void InitializePipeline();
		static void SubmitPipeline();
		static void OnEvent(Event& e);

		static void ValidateResize(glm::vec2 viewportSize);
	private:
		static void InitializeShadowPass();
		static void InitializeGeometryPass();
		static void UploadMaterialProperties(const RendererMaterialProperties& properties);
		static void ShadowPass();
		static void GeometryPass();

	private:
		static Ref<Scene> s_ActiveScene;
		static Camera s_Camera;

		static Ref<RenderPass> s_GeometryPass;
		static Ref<RenderPass> s_ShadowPass;
		static Ref<Texture2D> s_WhiteTexture;
		static Ref<UniformBuffer> s_ShadowUniformbuffer;

		static Ref<Shader> s_DebugDepthShader;
	};
}