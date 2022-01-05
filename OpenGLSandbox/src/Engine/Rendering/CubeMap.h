#pragma once

#include "Engine/Core/Memory.h"
#include "Engine/Rendering/TextureUtils.h"
#include "Engine/Rendering/VertexArray.h"
#include "Engine/Rendering/Shader.h"
#include "Engine/Rendering/Texture.h"

#include <vector>
#include <string>
#include <stdint.h>
#include <glm/glm.hpp>

namespace Engine
{
	class CubeMap
	{
	public:
		CubeMap(const Ref<TextureCube>& textureCube, const Ref<Shader>& shader);
		~CubeMap();

		void Submit(const glm::mat4& viewProjection);
		const Ref<TextureCube> GetTexture3D() const { return m_Texture3D; }

	private:
		void ConstructPipelinePrimitives();

	private:
		uint32_t m_ID;
		Ref<TextureCube> m_Texture3D;
		Ref<Shader> m_Shader;
		Ref<VertexArray> m_VAO;
		Ref<VertexBuffer> m_VBO;
	};
}