#pragma once

#include "Engine/Core/Memory.h"
#include <stdint.h>
#include <vector>
#include <glm/glm.hpp>


namespace Engine
{
	enum class PrimitiveType { None = 0, Triangle, Quad, Plane, Cube, Sphere };

	struct Vertex
	{
		glm::vec3 Position;
		glm::vec2 TexCoord;
		glm::vec3 Normal;
	};

	class Mesh
	{
	public:
		Mesh(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices)
			:m_Vertices(vertices), m_Indices(indices)
		{
		}

		std::vector<Vertex>& GetVertices() { return m_Vertices; }
		std::vector<uint32_t>& GetIndices() { return m_Indices; }

	private:
		std::vector<Vertex> m_Vertices;
		std::vector<uint32_t> m_Indices;
	};

	class MeshFactory
	{
	public:
		static Ref<Mesh> Create(PrimitiveType primitiveType);
		static Ref<Mesh> Plane();
		static Ref<Mesh> Triangle();
		static Ref<Mesh> Quad();
		static Ref<Mesh> Cube();
		static Ref<Mesh> Sphere(float radius);
	};
}

