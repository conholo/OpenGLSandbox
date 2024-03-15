#include "epch.h"
#include "Engine/Rendering/Mesh.h"
#include "Engine/Core/Math.h"
#include "Engine/Rendering/Texture.h"

#define LOG_MESH 1

namespace Engine
{

#define PI 3.14159265359

	Mesh::Mesh()
	{

	}
	
	Mesh::Mesh(float* vertices, uint32_t vertexCount, uint32_t* indices, uint32_t indexCount)
	{
		m_Vertices.resize(vertexCount);
		memcpy(m_Vertices.data(), vertices, vertexCount * sizeof(Vertex));

		m_Indices.resize(indexCount);
		memcpy(m_Indices.data(), indices, indexCount * sizeof(uint32_t));
	}
	
	Mesh::Mesh(const std::string& modelFilePath)
	{
		LOG_INFO("Loading Mesh: {}", modelFilePath);
	}

	static std::tuple<std::string, std::string> TexPathName(const std::string& modelFilePath, const std::string& aiTexPath)
	{
		std::filesystem::path path = modelFilePath;
		const std::string lastForwardSlash = modelFilePath.substr(0, modelFilePath.find_last_of("/") + 1);
		size_t pos = aiTexPath.find_last_of("\\") + 1;
		std::string texName = aiTexPath.substr(pos, aiTexPath.length() - pos);
		std::string texPath = lastForwardSlash + texName;
		return std::make_tuple(texPath, texName);
	}

	static void LogTextureSamplerSet(TextureUniform& Uniform, const Ref<Texture2D>& WhiteTexture)
	{
		LOG_TRACE("\tSetting Sampler Uniform: ");
		LOG_TRACE("\t\tSamplerName: sampler_AlbedoTexture");
		LOG_TRACE("\t\tTextureName: '{}'", WhiteTexture->GetName());
		LOG_TRACE("\t\tSlot: {}", Uniform.TextureUnit);
	}

	void Mesh::ResetVertices(float* vertices, uint32_t vertexCount)
	{
		m_Vertices.clear();
		m_Vertices.resize(vertexCount);
		memcpy(m_Vertices.data(), vertices, vertexCount * sizeof(Vertex));
	}

	void Mesh::ResetIndices(uint32_t* indices, uint32_t indexCount)
	{
		m_Indices.clear();
		m_Indices.resize(indexCount);
		memcpy(m_Indices.data(), indices, indexCount * sizeof(uint32_t));
	}

	Ref<Mesh> MeshFactory::Create(PrimitiveType primitiveType)
	{
		switch (primitiveType)
		{
		case PrimitiveType::Plane:					return Plane();
		case PrimitiveType::Quad:					return Quad();
		case PrimitiveType::FullScreenQuad:			return FullScreenQuad();
		case PrimitiveType::Triangle:				return Triangle();
		case PrimitiveType::Cube:					return Cube();
		case PrimitiveType::Sphere:					return Sphere(1.0f);
		case PrimitiveType::TessellatedQuad:		return TessellatedQuad(10);
		}

		return nullptr;
	}

	Ref<Mesh> MeshFactory::Plane()
	{
		std::vector<Vertex> vertices =
		{
			Vertex{ {-0.5f,  0.0f,  0.5f},  {0.0f, 1.0f, 0.0f}, {}, {}, {0.0f, 0.0f} },
			Vertex{ { 0.5f,  0.0f,  0.5f},  {0.0f, 1.0f, 0.0f}, {}, {}, {1.0f, 0.0f} },
			Vertex{ { 0.5f,  0.0f, -0.5f},  {0.0f, 1.0f, 0.0f}, {}, {}, {1.0f, 1.0f} },
			Vertex{ {-0.5f,  0.0f, -0.5f},  {0.0f, 1.0f, 0.0f}, {}, {}, {0.0f, 1.0f} },
		};

		std::vector<uint32_t> indices =
		{
			 0, 1, 2, 2, 3, 0
		};

		return CreateRef<Mesh>(vertices, indices);
	}

	Ref<Mesh> MeshFactory::Triangle()
	{
		std::vector<Vertex> vertices =
		{
			// Front Face
			Vertex{ {-0.5f, -0.5f,  0.0f}, { 0.0f, 0.0f, 1.0f }, {}, {}, {0.0f, 0.0f} },	// 0 0 
			Vertex{ { 0.5f, -0.5f,  0.0f}, { 0.0f, 0.0f, 1.0f }, {}, {}, {1.0f, 0.0f} },	// 1 1
			Vertex{ { 0.0f,  0.5f,  0.0f}, { 0.0f, 0.0f, 1.0f }, {}, {}, {0.5f, 1.0f} },	// 2 2
		};

		std::vector<uint32_t> indices = { 0, 1, 2 };

		return CreateRef<Mesh>(vertices, indices);
	}

	Ref<Mesh> MeshFactory::Quad()
	{
		std::vector<Vertex> vertices =
		{
			Vertex{ {-0.5f, -0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {}, {}, {0.0f, 0.0f} },
			Vertex{ { 0.5f, -0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {}, {}, {1.0f, 0.0f} },
			Vertex{ { 0.5f,  0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {}, {}, {1.0f, 1.0f} },
			Vertex{ {-0.5f,  0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {}, {}, {0.0f, 1.0f} },
		};

		std::vector<uint32_t> indices =
		{
			 0, 1, 2, 2, 3, 0
		};

		return CreateRef<Mesh>(vertices, indices);
	}

	Ref<Mesh> MeshFactory::FullScreenQuad()
	{
		std::vector<Vertex> vertices =
		{
			Vertex{ {-1.0f, -1.0f, 0.0f},  {0.0f, 0.0f, 1.0f}, {}, {}, {0.0f, 0.0f} },
			Vertex{ { 1.0f, -1.0f, 0.0f},  {0.0f, 0.0f, 1.0f}, {}, {}, {1.0f, 0.0f} },
			Vertex{ { 1.0f,  1.0f, 0.0f},  {0.0f, 0.0f, 1.0f}, {}, {}, {1.0f, 1.0f} },
			Vertex{ {-1.0f,  1.0f, 0.0f},  {0.0f, 0.0f, 1.0f}, {}, {}, {0.0f, 1.0f} },
		};

		std::vector<uint32_t> indices =
		{
			 0, 1, 2, 2, 3, 0
		};

		return CreateRef<Mesh>(vertices, indices);
	}


	Ref<Mesh> MeshFactory::Cube()
	{
		std::vector<Vertex> vertices =
		{
			// Front Face
			Vertex{ {-0.5f, -0.5f,  0.5f}, { 0.0f,   0.0f,  1.0f }, {}, {}, {0.0f, 0.0f}  },	// 0 0 
			Vertex{ { 0.5f, -0.5f,  0.5f}, { 0.0f,   0.0f,  1.0f }, {}, {}, {1.0f, 0.0f}  },	// 1 1
			Vertex{ { 0.5f,  0.5f,  0.5f}, { 0.0f,   0.0f,  1.0f }, {}, {}, {1.0f, 1.0f}  },	// 2 2
			Vertex{ {-0.5f,  0.5f,  0.5f}, { 0.0f,   0.0f,  1.0f }, {}, {}, {0.0f, 1.0f}  },	// 3 3

			// Right Face									
			Vertex{ { 0.5f, -0.5f,  0.5f}, { 1.0f,   0.0f,  0.0f }, {}, {}, {0.0f, 0.0f}  },	// 1 4 
			Vertex{ { 0.5f, -0.5f, -0.5f}, { 1.0f,   0.0f,  0.0f }, {}, {}, {1.0f, 0.0f}  },	// 5 5
			Vertex{ { 0.5f,  0.5f, -0.5f}, { 1.0f,   0.0f,  0.0f }, {}, {}, {1.0f, 1.0f}  },	// 6 6
			Vertex{ { 0.5f,  0.5f,  0.5f}, { 1.0f,   0.0f,  0.0f }, {}, {}, {0.0f, 1.0f}  },	// 2 7

			// Back Face									
			Vertex{ { 0.5f, -0.5f, -0.5f},  { 0.0f,   0.0f, -1.0f }, {}, {}, {0.0f, 0.0f} },	// 4 8
			Vertex{ {-0.5f, -0.5f, -0.5f},  { 0.0f,   0.0f, -1.0f }, {}, {}, {1.0f, 0.0f} },	// 5 9
			Vertex{ {-0.5f,  0.5f, -0.5f},  { 0.0f,   0.0f, -1.0f }, {}, {}, {1.0f, 1.0f} },	// 6 10
			Vertex{ { 0.5f,  0.5f, -0.5f},  { 0.0f,   0.0f, -1.0f }, {}, {}, {0.0f, 1.0f} },	// 7 11

			// Left Face									
			Vertex{ {-0.5f, -0.5f, -0.5f},  { -1.0f,  0.0f,  0.0f }, {}, {}, {0.0f, 0.0f} },	// 0 12
			Vertex{ {-0.5f, -0.5f,  0.5f},  { -1.0f,  0.0f,  0.0f }, {}, {}, {1.0f, 0.0f} },	// 4 13
			Vertex{ {-0.5f,  0.5f,  0.5f},  { -1.0f,  0.0f,  0.0f }, {}, {}, {1.0f, 1.0f} },	// 7 14
			Vertex{ {-0.5f,  0.5f, -0.5f},  { -1.0f,  0.0f,  0.0f }, {}, {}, {0.0f, 1.0f} },	// 3 15

			// Bottom Face									
			Vertex{ { 0.5f, -0.5f,  0.5f},  {  0.0f, -1.0f,  0.0f }, {}, {}, {0.0f, 0.0f} },	// 0 16
			Vertex{ {-0.5f, -0.5f,  0.5f},  {  0.0f, -1.0f,  0.0f }, {}, {}, {1.0f, 0.0f} },	// 1 17
			Vertex{ {-0.5f, -0.5f, -0.5f},  {  0.0f, -1.0f,  0.0f }, {}, {}, {1.0f, 1.0f} },	// 5 18
			Vertex{ { 0.5f, -0.5f, -0.5f},  {  0.0f, -1.0f,  0.0f }, {}, {}, {0.0f, 1.0f} },	// 4 19

			// Top Face										
			Vertex{ {-0.5f,  0.5f,  0.5f},  {  0.0f,  1.0f,  0.0f }, {}, {}, {0.0f, 0.0f} },	// 3 20
			Vertex{ { 0.5f,  0.5f,  0.5f},  {  0.0f,  1.0f,  0.0f }, {}, {}, {1.0f, 0.0f} },	// 2 21
			Vertex{ { 0.5f,  0.5f, -0.5f},  {  0.0f,  1.0f,  0.0f }, {}, {}, {1.0f, 1.0f} },	// 6 22
			Vertex{ {-0.5f,  0.5f, -0.5f},  {  0.0f,  1.0f,  0.0f }, {}, {}, {0.0f, 1.0f} }	// 7 23
		};

		std::vector<uint32_t> indices =
		{
			// front
			0, 1, 2,
			2, 3, 0,
			// right
			4, 5, 6,
			6, 7, 4,
			// back
			8, 9, 10,
			10, 11, 8,
			// left
			12, 13, 14,
			14, 15, 12,
			// bottom
			16, 17, 18,
			18, 19, 16,
			// top
			20, 21, 22,
			22, 23, 20
		};

		return CreateRef<Mesh>(vertices, indices);
	}

	Ref<Mesh> MeshFactory::Sphere(float radius)
	{
		std::vector<Vertex> vertices;
		std::vector<uint32_t> indices;

		constexpr float latitudeBands = 30;
		constexpr float longitudeBands = 30;

		for (float latitude = 0.0f; latitude <= latitudeBands; latitude++)
		{
			const float theta = latitude * (float)PI / latitudeBands;
			const float sinTheta = glm::sin(theta);
			const float cosTheta = glm::cos(theta);

			float texT = 1.0f - theta / PI;

			for (float longitude = 0.0f; longitude <= longitudeBands; longitude++)
			{
				const float phi = longitude * 2.0f * (float)PI / longitudeBands;
				const float sinPhi = glm::sin(phi);
				const float cosPhi = glm::cos(phi);

				float texS = 1.0f - (phi / (2 * PI));

				Vertex vertex;
				vertex.Normal = { cosPhi * sinTheta, cosTheta, sinPhi * sinTheta };
				vertex.TexCoord = { texS, texT };
				vertex.Position = { radius * vertex.Normal.x, radius * vertex.Normal.y, radius * vertex.Normal.z };
				vertices.push_back(vertex);
			}
		}

		for (uint32_t latitude = 0; latitude < (uint32_t)latitudeBands; latitude++)
		{
			for (uint32_t longitude = 0; longitude < (uint32_t)longitudeBands; longitude++)
			{
				const uint32_t first = (latitude * ((uint32_t)longitudeBands + 1)) + longitude;
				const uint32_t second = first + (uint32_t)longitudeBands + 1;

				indices.push_back(first);
				indices.push_back(first + 1);
				indices.push_back(second);

				indices.push_back(second);
				indices.push_back(first + 1);
				indices.push_back(second + 1);
			}
		}

		return CreateRef<Mesh>(vertices, indices);
	}

	static int AddIcosphereVertex(const glm::vec3& v, std::vector<Vertex>& vertices, uint32_t* index)
	{
		float length = glm::length(v);
		glm::vec3 unitSphereV = v / length;
		vertices.push_back({unitSphereV, unitSphereV, {}, {}, {0.0f, 0.0f}});
		return (*(index))++;
	}

	static uint32_t GetIcosphereMidpoint(int p1, int p2, std::unordered_map<uint64_t, int>& midpointCache, std::vector<Vertex>& vertices, uint32_t* index)
	{
		bool firstIsSmaller = p1 < p2;
		uint64_t smallerIndex = firstIsSmaller ? p1 : p2;
		uint64_t largerIndex = firstIsSmaller ? p2 : p1;
		uint64_t key = (smallerIndex << 32) + largerIndex;

		if (midpointCache.find(key) != midpointCache.end())
			return midpointCache[key];

		Vertex v1 = vertices[p1];
		Vertex v2 = vertices[p2];
		glm::vec3 mid = (v1.Position + v2.Position) / 2.0f;
		int vertexIndex = AddIcosphereVertex(mid, vertices, index);
		midpointCache[key] = vertexIndex;
		return vertexIndex;
	}

	Ref<Mesh> MeshFactory::Icosphere(uint32_t level, float radius)
	{
		float t = (1.0f + glm::sqrt(5.0f)) / 2.0f;
		
		uint32_t vertexIndex = 0;
		std::vector<Vertex> vertices;

		AddIcosphereVertex({ -1.0f,  t, 0.0f }, vertices, &vertexIndex);
		AddIcosphereVertex({  1.0f,  t, 0.0f }, vertices, &vertexIndex);
		AddIcosphereVertex({ -1.0f, -t, 0.0f }, vertices, &vertexIndex);
		AddIcosphereVertex({  1.0f, -t, 0.0f }, vertices, &vertexIndex);

		AddIcosphereVertex({ -0.0f, -1.0f,  t }, vertices, &vertexIndex);
		AddIcosphereVertex({ -0.0f,  1.0f,  t }, vertices, &vertexIndex);
		AddIcosphereVertex({ -0.0f, -1.0f, -t }, vertices, &vertexIndex);
		AddIcosphereVertex({ -0.0f,  1.0f, -t }, vertices, &vertexIndex);

		AddIcosphereVertex({  t, 0.0f, -1.0f }, vertices, &vertexIndex);
		AddIcosphereVertex({  t, 0.0f,  1.0f }, vertices, &vertexIndex);
		AddIcosphereVertex({ -t, 0.0f, -1.0f }, vertices, &vertexIndex);
		AddIcosphereVertex({ -t, 0.0f,  1.0f }, vertices, &vertexIndex);

		struct Triangle { uint32_t A, B, C; };

		std::vector<Triangle> facesOne;

		facesOne.push_back({ 0, 11, 5 });
		facesOne.push_back({ 0, 5, 1 });
		facesOne.push_back({ 0, 1, 7 });
		facesOne.push_back({ 0, 7, 10 });
		facesOne.push_back({ 0, 10, 11 });

		// 5 adjacent faces
		facesOne.push_back({ 1, 5, 9 });
		facesOne.push_back({ 5, 11, 4 });
		facesOne.push_back({ 11, 10, 2 });
		facesOne.push_back({ 10, 7, 6 });
		facesOne.push_back({ 7, 1, 8 });;

		// 5 faces around point 3
		facesOne.push_back({ 3, 9, 4 });
		facesOne.push_back({ 3, 4, 2 });
		facesOne.push_back({ 3, 2, 6 });
		facesOne.push_back({ 3, 6, 8 });
		facesOne.push_back({ 3, 8, 9 });

		// 5 adjacent faces
		facesOne.push_back({ 4, 9, 5 });
		facesOne.push_back({ 2, 4, 11 });
		facesOne.push_back({ 6, 2, 10 });
		facesOne.push_back({ 8, 6, 7 });
		facesOne.push_back({ 9, 8, 1 });

		std::unordered_map<uint64_t, int> midpointCache;

		for (uint32_t i = 0; i < level; i++)
		{
			std::vector<Triangle> facesTwo;

			for (auto triangle : facesOne)
			{
				uint32_t a = GetIcosphereMidpoint(triangle.A, triangle.B, midpointCache, vertices, &vertexIndex);
				uint32_t b = GetIcosphereMidpoint(triangle.B, triangle.C, midpointCache, vertices, &vertexIndex);
				uint32_t c = GetIcosphereMidpoint(triangle.C, triangle.A, midpointCache, vertices, &vertexIndex);

				facesTwo.push_back({ triangle.A, a, c });
				facesTwo.push_back({ triangle.B, b, a });
				facesTwo.push_back({ triangle.C, c, b });
				facesTwo.push_back({ a, b, c });
			}

			facesOne = facesTwo;
		}

		std::vector<uint32_t> indices;
		for (auto triangle : facesOne)
		{
			indices.push_back(triangle.A);
			indices.push_back(triangle.B);
			indices.push_back(triangle.C);
		}

		return CreateRef<Mesh>(vertices, indices);
	}

	std::string MeshFactory::PrimitiveTypeToString(PrimitiveType type)
	{
		switch (type)
		{
		case PrimitiveType::Cube: return "Cube";
		case PrimitiveType::Sphere: return "Sphere";
		case PrimitiveType::Quad: return "Quad";
		case PrimitiveType::Plane: return "Plane";
		case PrimitiveType::FullScreenQuad: return "Full Screen Quad";
		case PrimitiveType::TessellatedQuad: return "Tessellated Quad";
		}

		return "";
	}

	static glm::vec3 RemapVector3(const glm::vec3& inVector)
	{
		float x = Remap(inVector.x, -0.5f, 0.5f, 0.0f, 1.0f);
		float y = Remap(inVector.y, -0.5f, 0.5f, 0.0f, 1.0f);
		float z = Remap(inVector.z, -0.5f, 0.5f, 0.0f, 1.0f);

		return { x, y, z };
	}

	Ref<Mesh> MeshFactory::TessellatedQuad(uint32_t resolution)
	{
		std::vector<Vertex> vertices;
		std::vector<uint32_t> indices;

		// Space between each vertex.  This is for a unit quad (-0.5f, -0.5f) - (0.5f, 0.5).
		float spacing = 1.0f / (float)resolution;
		
		glm::vec3 bottomLeft{ -0.5f, -0.5f, 0.0f };

		for (uint32_t y = 0; y < resolution; y++)
		{
			for (uint32_t x = 0; x < resolution; x++)
			{
				glm::vec3 position{ bottomLeft.x + (float)x * spacing, bottomLeft.y + (float)y * spacing, 0.0f };

				glm::vec2 texCoord{ (float)x / resolution, (float)y / resolution };
				glm::vec3 normal{ 0.0f, 0.0f, 1.0f };

				vertices.push_back({ position, normal, {}, {}, texCoord });

				if (x == resolution - 1 || y == resolution - 1) continue;

				uint32_t a = y * resolution + x;
				uint32_t b = y * resolution + x + resolution;
				uint32_t c = y * resolution + x + resolution + 1;
				indices.push_back(a);
				indices.push_back(b);
				indices.push_back(c);

				uint32_t d = a;
				uint32_t e = y * resolution + x + resolution + 1;
				uint32_t f = a + 1;
				indices.push_back(d);
				indices.push_back(e);
				indices.push_back(f);
			}
		}

		return CreateRef<Mesh>(vertices, indices);
	}


}

