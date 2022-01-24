#pragma once

#include "Engine/Core/Memory.h"
#include <stdint.h>
#include <vector>
#include <string>
#include <glm/glm.hpp>


namespace Engine
{
	enum class PrimitiveType { None = 0, Triangle, Quad, FullScreenQuad, Plane, Cube, Sphere, TessellatedQuad, Icosphere };

	struct Vertex
	{
		glm::vec3 Position;
		glm::vec2 TexCoord;
		glm::vec3 Normal;
	};

	class Mesh
	{
	public:
		Mesh();
		Mesh(float* vertices, uint32_t vertexCount, uint32_t* indices, uint32_t indexCount);
		Mesh(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices)
			:m_Vertices(vertices), m_Indices(indices) { }

		void ResetVertices(float* vertices, uint32_t vertexCount);
		void ResetIndices(uint32_t* indices, uint32_t indexCount);

		std::vector<Vertex>& GetVertices() { return m_Vertices; }
		std::vector<uint32_t>& GetIndices() { return m_Indices; }

		void ResizeAndClearVertices(uint32_t size) { m_Vertices.clear(); m_Vertices.resize(size); }
		void ResizeAndClearIndices(uint32_t size) { m_Indices.clear(); m_Indices.resize(size); }

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
		static Ref<Mesh> FullScreenQuad();
		static Ref<Mesh> Cube();
		static Ref<Mesh> TessellatedQuad(uint32_t resolution);
		static Ref<Mesh> Sphere(float radius);
		static Ref<Mesh> Icosphere(uint32_t level, float radius);

		static std::string PrimitiveTypeToString(PrimitiveType type);
	};
}

