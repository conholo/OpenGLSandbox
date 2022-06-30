#pragma once

#include "Engine/Rendering/VertexArray.h"
#include "Engine/Rendering/Camera.h"
#include "Engine/Rendering/Mesh.h"

namespace Engine
{
	class EditorGrid
	{
	public:
		EditorGrid(float scale = 1.0f);

		void Draw(const Camera& camera);
		void SetOuter(float outerGridScale) { m_OuterGridScale = outerGridScale; }
		void SetInner(float innerGridScale) { m_InnerGridScale = innerGridScale; }
		void SetScale(float scale) { m_Scale = scale; }

		void SetShader(const std::string& shaderName);

	private:
		float m_InnerGridScale = 1.0f;
		float m_OuterGridScale = 0.1f;
		float m_Scale = 1.0f;

	private:
		std::string m_ShaderName = "InfiniteGrid";
		std::vector<glm::vec3> m_PointVertices;
		Ref<VertexArray> m_DebugPointVAO;
		Ref<VertexBuffer> m_DebugPointVBO;


		std::vector<glm::vec3> m_LineVertices;
		Ref<VertexArray> m_DebugLineVAO;
		Ref<VertexBuffer> m_DebugLineVBO;


		Ref<Mesh> m_FSQ;
		Ref<VertexArray> m_VAO;
		Ref<IndexBuffer> m_EBO;
		Ref<VertexBuffer> m_VBO;
	};
}