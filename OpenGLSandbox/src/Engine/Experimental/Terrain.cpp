#include "epch.h"
#include "Engine/Experimental/Terrain.h"
#include "Engine/Core/Random.h"
#include "Engine/Rendering/RenderCommand.h"
#include "Engine/Rendering/Shader.h"

namespace Engine
{
	std::vector<glm::vec4> GenerateOffsets(int octaves)
	{
		std::vector<glm::vec4> result;
		result.resize(octaves);

		for (int i = 0; i < octaves; i++)
		{
			glm::vec4 random = { Random::RandomRange(0, 1), Random::RandomRange(0, 1), Random::RandomRange(0, 1), Random::RandomRange(0, 1) };
			result[i] = (random * glm::vec4(2.0) - glm::vec4(1.0)) * glm::vec4(1000.0);
		}

		return result;
	}

	Terrain::Terrain()
	{
		uint32_t maxResolution = glm::pow(m_BaseResolution, (m_MinLOD - m_MaxLOD) + 3);
		uint32_t maxSizeVerticesBuffer = (maxResolution * maxResolution) * sizeof(TerrainVertex);
		uint32_t maxSizeIndicesBuffer = (maxResolution - 1) * (maxResolution - 1) * 6 * sizeof(uint32_t);

		m_VertexSSBO = CreateRef<ShaderStorageBuffer>(maxSizeVerticesBuffer);
		m_IndexSSBO = CreateRef<ShaderStorageBuffer>(maxSizeIndicesBuffer);

		m_MinMax[0] = INT_MAX;
		m_MinMax[1] = INT_MIN;
		m_MinMaxSSBO = CreateRef<ShaderStorageBuffer>(m_MinMax, sizeof(float) * 2);

		m_VAO = CreateRef<VertexArray>();
		m_VBO = CreateRef<VertexBuffer>(maxSizeVerticesBuffer);
		m_EBO = CreateRef<IndexBuffer>(maxSizeIndicesBuffer / sizeof(uint32_t));

		BufferLayout layout = BufferLayout
		(
			{
				{ "a_Position", ShaderAttributeType::Float3 },
				{ "a_TexCoord", ShaderAttributeType::Float2 },
				{ "a_Normal",	ShaderAttributeType::Float3 },
			}
		);
		m_VBO->SetLayout(layout);
		m_VAO->EnableVertexAttributes(m_VBO);
		m_VAO->SetIndexBuffer(m_EBO);
		m_Transform = CreateRef<EntityTransform>();

		m_Properties = CreateRef<TerrainProperties>();
		m_Properties->NoiseSettings = CreateRef<TerrainNoiseSettings>();

		m_NoiseOffsets = GenerateOffsets(m_Properties->NoiseSettings->Octaves);
		m_NoiseOffsetsSSBO = Engine::CreateRef<Engine::ShaderStorageBuffer>(m_NoiseOffsets.data(), m_NoiseOffsets.size() * sizeof(glm::vec4));
		m_Transform->SetScale(m_Properties->Scale);
		m_Transform->SetPosition(m_Properties->Position);

		SetLOD(m_LOD);
		UpdateTerrain();
	}

	Terrain::~Terrain()
	{
	}

	void Terrain::Draw()
	{
		m_VAO->Bind();
		uint32_t indexCount = ((m_Resolution - 1) * (m_Resolution - 1) * 6);
		RenderCommand::DrawIndexed(m_VAO, indexCount);
		m_VAO->Unbind();
	}

	void Terrain::UpdateTerrain()
	{
		uint32_t workGroupCount = glm::ceil(m_Resolution / (float)m_WorkGroupLocalSize);

		m_MinMax[0] = INT_MAX;
		m_MinMax[1] = INT_MIN;

		m_MinMaxSSBO->SetData(m_MinMax, 0, sizeof(float) * 2);

		m_Transform->SetPosition(m_Properties->Position);
		m_Transform->SetScale(m_Properties->Scale);

		ShaderLibrary::Get("TerrainGenerator")->Bind();
		glm::vec3 position = m_Transform->GetPosition();
		glm::vec3 scale = m_Transform->GetScale();
		glm::vec2 boundsMin = glm::vec2(position.x - scale.x / 2.0f, position.z - scale.z / 2.0f);
		glm::vec2 boundsMax = glm::vec2(position.x + scale.x / 2.0f, position.z + scale.z / 2.0f);

		ShaderLibrary::Get("TerrainGenerator")->UploadUniformFloat("u_HeightThreshold", m_Properties->HeightThreshold);
		ShaderLibrary::Get("TerrainGenerator")->UploadUniformFloat("u_HeightScaleFactor", m_Properties->HeightScaleFactor);
		ShaderLibrary::Get("TerrainGenerator")->UploadUniformFloat2("u_BoundsMin", boundsMin);
		ShaderLibrary::Get("TerrainGenerator")->UploadUniformFloat2("u_BoundsMax", boundsMax);
		ShaderLibrary::Get("TerrainGenerator")->UploadUniformFloat2("u_Resolution", { m_Resolution, m_Resolution });

		ShaderLibrary::Get("TerrainGenerator")->UploadUniformInt("u_Settings.Octaves", m_Properties->NoiseSettings->Octaves);
		ShaderLibrary::Get("TerrainGenerator")->UploadUniformFloat("u_Settings.NoiseScale", m_Properties->NoiseSettings->NoiseScale);
		ShaderLibrary::Get("TerrainGenerator")->UploadUniformFloat("u_Settings.Lacunarity", m_Properties->NoiseSettings->Lacunarity);
		ShaderLibrary::Get("TerrainGenerator")->UploadUniformFloat("u_Settings.Persistence", m_Properties->NoiseSettings->Persistence);
		ShaderLibrary::Get("TerrainGenerator")->UploadUniformFloat2("u_Settings.TextureOffset", m_Properties->NoiseSettings->TextureOffset);

		m_VertexSSBO->BindToComputeShader(0, ShaderLibrary::Get("TerrainGenerator")->GetID());
		m_IndexSSBO->BindToComputeShader(1, ShaderLibrary::Get("TerrainGenerator")->GetID());
		m_NoiseOffsetsSSBO->BindToComputeShader(2, ShaderLibrary::Get("TerrainGenerator")->GetID());
		m_MinMaxSSBO->BindToComputeShader(3, ShaderLibrary::Get("TerrainGenerator")->GetID());
		ShaderLibrary::Get("TerrainGenerator")->DispatchCompute(workGroupCount, workGroupCount, 1);
		ShaderLibrary::Get("TerrainGenerator")->EnableShaderStorageBarrierBit();
		ShaderLibrary::Get("TerrainGenerator")->EnableBufferUpdateBarrierBit();

		uint32_t indexBufferSize = ((m_Resolution - 1) * (m_Resolution - 1) * 6) * sizeof(uint32_t);
		uint32_t vertexBufferSize = (m_Resolution * m_Resolution) * sizeof(TerrainVertex);
		m_VertexSSBO->CopyData(m_VBO->GetID(), 0, 0, vertexBufferSize);
		m_IndexSSBO->CopyData(m_EBO->GetID(), 0, 0, indexBufferSize);

		float* minMax = (float*)m_MinMaxSSBO->GetData();

		m_MinHeight = minMax[0];
		m_MaxHeight = minMax[1];
	}

	void Terrain::SetLOD(uint32_t lodLevel)
	{
		if (lodLevel > 8)
			lodLevel = 8;
		m_LOD = lodLevel;

		// Add three so the lowest resolution we'll get is 8 ( 2^8).
		// At LOD 0: 2^(8 + 3) = 2048 x 2048
		// At LOD 8: 2^(0 + 3) = 8 x 8
		m_Resolution = glm::pow(m_BaseResolution, m_MinLOD - m_MaxLOD - m_LOD + 3);
	}
}
