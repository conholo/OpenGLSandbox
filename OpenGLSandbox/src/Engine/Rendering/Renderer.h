#pragma once

#include "Engine/Rendering/Shader.h"
#include "Engine/Rendering/Camera.h"
#include "Engine/Rendering/RenderPass.h"
#include "Engine/Scene/Component.h"

#include <glm/glm.hpp>

namespace Engine
{
	class Renderer
	{
	public:
		static void Initialize();

		static void BeginPass(const Ref<RenderPass>& renderPass);
		static void EndPass(const Ref<RenderPass>& renderPass);
		static void UploadCameraUniformData(const Camera& camera, const TransformComponent& transform);

		static void Begin();
		static void End();
		static void DrawPrimitive(PrimitiveType type);
		static void DrawFullScreenQuad();

		static void Shutdown();

	private:

		static void CreateRenderPrimitivesForMesh(PrimitiveType type);
	};
}