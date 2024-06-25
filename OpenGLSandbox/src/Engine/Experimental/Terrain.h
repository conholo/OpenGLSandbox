#pragma once

#include "Engine/Rendering/VertexArray.h"
#include "Engine/Core/Memory.h"
#include "Engine/Scene/SimpleECS/EntityTransform.h"
#include "Engine/Rendering/ShaderStorageBuffer.h"
#include "Engine/Rendering/Texture.h"

namespace Engine
{
	struct TerrainHeightLayer
	{
		float HeightThreshold;
		float BlendStrength = 0.5f;
		float TextureTiling = 1.0f;
		glm::vec3 TintColor = glm::vec3(1.0f);
		Ref<Texture2D> HeightTexture;
	};

	struct TerrainVertex
	{
		glm::vec4 Position_UV_x;
		glm::vec4 Normal_UV_y;
	};

	struct TerrainNoiseSettings
	{
		int Octaves = 8;
		float Lacunarity = 2.820f;
		float Persistence = 0.340f;;
		float NoiseScale = 8.350f;;
		glm::vec2 TextureOffset{ 0.0f, 0.0f };
	};

	struct TerrainProperties
	{
		glm::vec3 Color{ 1.0f, 1.0f, 1.0f };
		glm::vec3 Scale{ 25.0f, 31.0f, 18.0f };
		glm::vec3 Position{ 0.0f, 0.0f, 0.0f };

		float HeightThreshold = 0.080f;
		float HeightScaleFactor = 46.7f;

		Ref<TerrainNoiseSettings> NoiseSettings;
	};

	class Terrain
	{
	public:
		Terrain();
		~Terrain();

		void Draw();
		void UpdateTerrain();

		const Ref<TerrainProperties>& GetProperties() const { return m_Properties; }
		const Ref<EntityTransform>& GetTransform() const { return m_Transform; }

		uint32_t GetResolution() const { return glm::pow(m_BaseResolution, m_MinLOD - m_MaxLOD - m_LOD + 3); }
		uint32_t GetLOD() const { return m_LOD; }

		void SetLOD(uint32_t lodLevel);
		uint32_t GetMinLOD() const { return m_MinLOD; }
		uint32_t GetMaxLOD() const { return m_MaxLOD; }

		float GetMinHeightLocalSpace() const { return m_MinHeight; }
		float GetMaxHeightLocalSpace() const { return m_MaxHeight; }

	private:
		Ref<VertexArray> m_VAO;
		Ref<VertexBuffer> m_VBO;
		Ref<IndexBuffer> m_EBO;;

		Ref<ShaderStorageBuffer> m_VertexSSBO;
		Ref<ShaderStorageBuffer> m_IndexSSBO;
		Ref<ShaderStorageBuffer> m_MinMaxSSBO;
		float m_MinMax[2];

		std::vector<glm::vec4> m_NoiseOffsets;
		Ref<ShaderStorageBuffer> m_NoiseOffsetsSSBO;

	private:
		Ref<TerrainProperties> m_Properties;
		Ref<EntityTransform> m_Transform;

	private:
		int m_LOD = 0;
		int m_Resolution = 8;

		float m_MinHeight = 0.0f;
		float m_MaxHeight = 0.0f;

		const uint32_t m_BaseResolution = 2;
		const uint32_t m_MinLOD = 8;
		const uint32_t m_MaxLOD = 0;
		const uint32_t m_WorkGroupLocalSize = 8;
	};
}