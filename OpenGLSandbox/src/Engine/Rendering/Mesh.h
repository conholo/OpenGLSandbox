#pragma once

#include "Engine/Rendering/Material.h"
#include "Engine/Rendering/Texture.h"

#include <glm/glm.hpp>

namespace Assimp
{
	class Importer;
}
struct aiNode;
struct aiScene;

namespace Engine
{
	enum class PrimitiveType { None = 0, Triangle, Quad, FullScreenQuad, Plane, Cube, Sphere, TessellatedQuad, Icosphere };

	struct Vertex
	{
		glm::vec3 Position;
		glm::vec3 Normal;
		glm::vec3 Tangent;
		glm::vec3 Binormal;
		glm::vec2 TexCoord;
	};

	class Submesh
	{
	public:
		uint32_t BaseVertex;
		uint32_t BaseIndex;
		uint32_t MaterialIndex;
		uint32_t IndexCount;
		uint32_t VertexCount;

		glm::mat4 Transform{ 1.0f }; // World transform
		glm::mat4 LocalTransform{ 1.0f };
		std::string NodeName, MeshName;
	};


	class Mesh
	{
	public:
		Mesh();
		Mesh(float* vertices, uint32_t vertexCount, uint32_t* indices, uint32_t indexCount);
		Mesh(std::vector<Vertex> vertices, std::vector<uint32_t> indices)
			:m_Vertices(std::move(vertices)), m_Indices(std::move(indices)) { }
		Mesh(const std::string& modelFilePath);
		
		void ResetVertices(float* vertices, uint32_t vertexCount);
		void ResetIndices(uint32_t* indices, uint32_t indexCount);

		bool HasSubmeshes() const { return m_Submeshes.size() > 0; }
		std::vector<Submesh>& GetSubmeshes() { return m_Submeshes; }
		const std::vector<Submesh>& GetSubmeshes() const { return m_Submeshes; }

		std::vector<Vertex>& GetVertices() { return m_Vertices; }
		std::vector<uint32_t>& GetIndices() { return m_Indices; }

		void ResizeAndClearVertices(uint32_t size) { m_Vertices.clear(); m_Vertices.resize(size); }
		void ResizeAndClearIndices(uint32_t size) { m_Indices.clear(); m_Indices.resize(size); }

		const Ref<Material>& GetFirstMaterial() const { return m_Materials[0]; }

		const std::vector<Ref<Material>> GetMaterials() const { return m_Materials; }

	private:
		void InitializeMeshFromFile(const aiScene* scene, const std::string& modelFilePath);

	private:
		std::vector<Ref<Texture2D>> m_Textures;
		std::vector<Ref<Texture2D>> m_NormalMaps;
		std::vector<Ref<Material>> m_Materials;
		std::vector<Submesh> m_Submeshes;
		std::vector<Vertex> m_Vertices;
		std::vector<uint32_t> m_Indices;
		Ref<Shader> m_MeshShader;
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

