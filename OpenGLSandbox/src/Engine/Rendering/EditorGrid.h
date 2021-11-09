#pragma once

#include "Engine/Rendering/VertexArray.h"
#include "Engine/Rendering/Camera.h"

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
	private:
		float m_InnerGridScale = 1.0f;
		float m_OuterGridScale = 0.1f;
		float m_Scale = 1.0f;

	private:
		Ref<VertexArray> m_VAO;
		Ref<VertexBuffer> m_VBO;
	};
}