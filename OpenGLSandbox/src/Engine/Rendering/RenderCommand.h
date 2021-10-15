#pragma once

#include <stdint.h>

#include <glm/glm.hpp>
#include "Engine/Rendering/VertexArray.h"

namespace Engine
{
	class RenderCommand
	{
	public:
		static void Initialize();
		static void Clear(bool colorBufferBit, bool depthBufferBit);
		static void SetViewport(uint32_t width, uint32_t height);
		static void ClearColor(const glm::vec4& clearColor);
		static void DrawIndexed(const Ref<VertexArray>& vertexArray, uint32_t indexCount = 0);
	};
}

