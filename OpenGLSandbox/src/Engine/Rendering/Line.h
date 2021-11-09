#pragma once

#include "Engine/Core/Memory.h"
#include "Engine/Rendering/Shader.h"
#include "Engine/Rendering/VertexArray.h"
#include "Engine/Rendering/VertexBuffer.h"
#include "Engine/Rendering/IndexBuffer.h"
#include "Engine/Scene/SimpleECS/EntityTransform.h"

#include <vector>
#include <string>
#include <glm/glm.hpp>

namespace Engine
{
	struct LineVertex
	{
		glm::vec3 Position;
	};

	class Line
	{
	public:
		Line(const std::vector<LineVertex>& vertices, const std::string& shaderName);
		Line(std::initializer_list<LineVertex>& vertices, const std::string& shaderName);
		~Line();

		void AddPoint(const LineVertex& point) { m_Vertices.push_back(point); }
		void Draw(const glm::mat4& viewProjection);
		void Clear();
		const Ref<EntityTransform>& GetEntityTransform() const { return m_Transform; }
		const Ref<Shader>& GetShader() const { return m_Shader; }

	private:
		void Initialize();

	private:
		Ref<Shader> m_Shader;
		Ref<VertexArray> m_VAO;
		Ref<VertexBuffer> m_VBO;
		std::vector<LineVertex> m_Vertices;
		Ref<EntityTransform> m_Transform;
	};
}