#include "epch.h"
#include "Engine/Scene/SimpleECS/EntityRenderer.h"
#include "Engine/Scene/SimpleECS/EntityTransform.h"
#include "Engine/Scene/SimpleECS/Light.h"
#include "Engine/Rendering/RenderCommand.h"


namespace Engine
{
	EntityRenderer::EntityRenderer(PrimitiveType primitiveType, const std::string& shaderName, const std::string& entityName)
	{
		m_Mesh = MeshFactory::Create(primitiveType);
		const auto& Shader = ShaderLibrary::Get(shaderName);
		m_Material = CreateRef<Material>(entityName, Shader);

		Initialize();
	}

	EntityRenderer::EntityRenderer(const Ref<Mesh>& mesh, const std::string& shaderName, const std::string& entityName)
		:m_Mesh(mesh)
	{
		const auto& Shader = ShaderLibrary::Get(shaderName);
		m_Material = CreateRef<Material>(entityName, Shader);
		
		Initialize();
	}

	EntityRenderer::EntityRenderer(const Ref<Mesh>& mesh)
		:m_Mesh(mesh)
	{
		Initialize();
	}

	void EntityRenderer::Initialize()
	{
		m_VertexArray = CreateRef<VertexArray>();

		float* baseVertexPtr = &m_Mesh->GetVertices().data()->Position.x;
		m_VertexBuffer = CreateRef<VertexBuffer>(baseVertexPtr, m_Mesh->GetVertices().size() * sizeof(Vertex));

		const BufferLayout layout = BufferLayout
		(
			{
				{ "a_Position", ShaderAttributeType::Float3 },
				{ "a_Normal",	ShaderAttributeType::Float3 },
				{ "a_Tangent", ShaderAttributeType::Float3 },
				{ "a_Binormal", ShaderAttributeType::Float3 },
				{ "a_TexCoord", ShaderAttributeType::Float2 }
			}
		);
		m_VertexBuffer->SetLayout(layout);

		uint32_t* indexPtr = m_Mesh->GetIndices().data();
		m_IndexBuffer = CreateRef<IndexBuffer>(indexPtr, m_Mesh->GetIndices().size());

		m_VertexArray->SetIndexBuffer(m_IndexBuffer);
		m_VertexArray->EnableVertexAttributes(m_VertexBuffer);
	}

	void EntityRenderer::Begin() const
	{
		m_VertexArray->Bind();
		m_VertexBuffer->Bind();
		m_IndexBuffer->Bind();
	}

	void EntityRenderer::Draw() const
	{
		RenderCommand::DrawIndexed(m_VertexArray);
	}

	float EntityRenderer::s_EnvironmentMapIntensity = 1.0;
	
	void EntityRenderer::StageSceneUniforms(
		Material& Material,
		const Camera& Camera,
		const Ref<Light>& DirectionalLight,
		const glm::mat4& ModelMatrix)
	{
		Material.Set<glm::mat4>("ViewProjectionMatrix", Camera.GetViewProjection());
		Material.Set<glm::mat4>("ModelMatrix", ModelMatrix);
		Material.Set<glm::mat4>("ViewMatrix", Camera.GetView());
		Material.Set<glm::vec3>("CameraPosition", Camera.GetPosition());

		Material.Set<glm::vec3>("LightPosition", DirectionalLight->GetLightTransform()->GetPosition());
		Material.Set<glm::vec3>("LightColor", DirectionalLight->GetLightColor());
		Material.Set<float>("LightIntensity", DirectionalLight->GetLightIntensity());
		Material.Set<float>("EnvironmentMapIntensity", s_EnvironmentMapIntensity);

		const TextureUniform RadianceUniform { TextureLibrary::GetCube("PragueSky-EnvironmentRadianceCubeFiltered")->GetID(), 15 };
		const TextureUniform IrradianceUniform { TextureLibrary::GetCube("PragueSky-EnvironmentIrradianceCube")->GetID(), 16 };
		Material.Set<TextureUniform>("sampler_RadianceCube", RadianceUniform);
		Material.Set<TextureUniform>("sampler_IrradianceCube", IrradianceUniform);
		Material.Set<TextureUniform>("sampler_BRDFLUT", { TextureLibrary::Get2D("BRDF LUT")->GetID(), 17 });
	}

	void EntityRenderer::Draw(const Camera& Camera, const Ref<Light>& DirectionalLight, const Ref<EntityTransform>& Transform) const
	{
		m_VertexArray->Bind();
		m_VertexBuffer->Bind();
		m_IndexBuffer->Bind();

		if(m_Mesh->HasSubmeshes())
		{
			const auto& SubMeshes = m_Mesh->GetSubmeshes();
			const auto& Materials = m_Mesh->GetMaterials();
			for(auto& SubMesh : SubMeshes)
			{
				Ref<Material> Material = Materials[SubMesh.MaterialIndex];
				Material->Bind();
				StageSceneUniforms(*Material, Camera, DirectionalLight, SubMesh.LocalTransform * Transform->Transform());
				Material->UploadStagedUniforms();
				RenderCommand::DrawIndexedRange(SubMesh.IndexCount, SubMesh.BaseIndex, SubMesh.BaseVertex);
				Material->Unbind();
			}
		}
		
		m_VertexArray->Unbind();
		m_VertexBuffer->Unbind();
		m_IndexBuffer->Unbind();
	}

	void EntityRenderer::End() const
	{
		if(m_Material != nullptr)
			m_Material->GetShader()->Unbind();
		m_VertexArray->Unbind();
		m_VertexBuffer->Unbind();
		m_IndexBuffer->Unbind();
	}

	void EntityRenderer::DrawPoints() const
	{
		RenderCommand::DrawPoints(m_Mesh->GetVertices().size());
	}

	void EntityRenderer::Bind() const
	{
		m_VertexArray->Bind();
		m_Material->GetShader()->Bind();
	}

	void EntityRenderer::Unbind() const
	{
		if(m_Material != nullptr)
			m_Material->GetShader()->Unbind();
		m_Material->GetShader()->Unbind();
		m_VertexArray->Unbind();
		m_VertexBuffer->Unbind();
		m_IndexBuffer->Unbind();
	}

	void EntityRenderer::SetPrimitiveType(PrimitiveType type)
	{
		m_Mesh = MeshFactory::Create(type);
		float* baseVertexPtr = &m_Mesh->GetVertices().data()->Position.x;
		m_VertexBuffer = CreateRef<VertexBuffer>(baseVertexPtr, m_Mesh->GetVertices().size() * sizeof(Vertex));

		BufferLayout layout = BufferLayout
		(
			{
					{ "a_Position", ShaderAttributeType::Float3 },
					{ "a_Normal",	ShaderAttributeType::Float3 },
					{ "a_Tangent", ShaderAttributeType::Float3 },
					{ "a_Binormal", ShaderAttributeType::Float3 },
					{ "a_TexCoord", ShaderAttributeType::Float2 },
				}
		);
		m_VertexBuffer->SetLayout(layout);

		uint32_t* indexPtr = m_Mesh->GetIndices().data();
		m_IndexBuffer = CreateRef<IndexBuffer>(indexPtr, m_Mesh->GetIndices().size());

		m_VertexArray->EnableVertexAttributes(m_VertexBuffer);
		m_VertexArray->SetIndexBuffer(m_IndexBuffer);
	}
}

