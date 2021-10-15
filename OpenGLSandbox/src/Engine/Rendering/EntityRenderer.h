#pragma once

#include "Engine/Rendering/Mesh.h"
#include "Engine/Rendering/Shader.h"
#include "Engine/Rendering/VertexArray.h"

namespace Engine
{
	class EntityRenderer
	{
	public:
		EntityRenderer(PrimitiveType primitiveType, const std::string& shaderName);

		void Begin();
		void Draw();
		void End();

		const Ref<Mesh> GetMesh() const { return m_Mesh; }
		const Ref<Shader> GetShader() const { return m_Shader; }

	private:
		void Initialize();

	private:
		Ref<VertexArray> m_VertexArray;
		Ref<VertexBuffer> m_VertexBuffer;
		Ref<IndexBuffer> m_IndexBuffer;
		Ref<Mesh> m_Mesh;
		Ref<Shader> m_Shader;
	};
}
