#include "Engine/Scene/SimpleECS/EntityRenderer.h"
#include "Engine/Rendering/RenderCommand.h"


namespace Engine
{
	EntityRenderer::EntityRenderer(PrimitiveType primitiveType, const std::string& shaderName)
	{
		m_Mesh = MeshFactory::Create(primitiveType);
		m_Shader = ShaderLibrary::Get(shaderName);

		Initialize();
	}

	EntityRenderer::EntityRenderer(const Ref<Mesh>& mesh, const std::string& shaderName)
		:m_Mesh(mesh)
	{
		m_Shader = ShaderLibrary::Get(shaderName);

		Initialize();
	}

	void EntityRenderer::Initialize()
	{
		m_VertexArray = CreateRef<VertexArray>();

		float* baseVertexPtr = &m_Mesh->GetVertices().data()->Position.x;
		m_VertexBuffer = CreateRef<VertexBuffer>(baseVertexPtr, m_Mesh->GetVertices().size() * sizeof(Vertex));

		BufferLayout layout = BufferLayout
		(
			{
				{ "a_Position", ShaderAttributeType::Float3 },
				{ "a_TexCoord", ShaderAttributeType::Float2 },
				{ "a_Normal",	ShaderAttributeType::Float3 },
			}
		);
		m_VertexBuffer->SetLayout(layout);

		uint32_t* indexPtr = m_Mesh->GetIndices().data();
		m_IndexBuffer = CreateRef<IndexBuffer>(indexPtr, m_Mesh->GetIndices().size());

		m_VertexArray->EnableVertexAttributes(m_VertexBuffer);
		m_VertexArray->SetIndexBuffer(m_IndexBuffer);
	}

	void EntityRenderer::Begin()
	{
		m_VertexArray->Bind();
		m_Shader->Bind();
	}

	void EntityRenderer::Draw()
	{
		RenderCommand::DrawIndexed(m_VertexArray);
	}

	void EntityRenderer::End()
	{
		m_Shader->Unbind();
		m_VertexArray->Unbind();
		m_VertexBuffer->Unbind();
		m_IndexBuffer->Unbind();
	}

	void EntityRenderer::DrawPoints()
	{
		RenderCommand::DrawPoints(m_Mesh->GetVertices().size());
	}

	void EntityRenderer::SetPrimtiveType(PrimitiveType type)
	{
		m_Mesh = MeshFactory::Create(type);
		float* baseVertexPtr = &m_Mesh->GetVertices().data()->Position.x;
		m_VertexBuffer = CreateRef<VertexBuffer>(baseVertexPtr, m_Mesh->GetVertices().size() * sizeof(Vertex));

		BufferLayout layout = BufferLayout
		(
			{
				{ "a_Position", ShaderAttributeType::Float3 },
				{ "a_TexCoord", ShaderAttributeType::Float2 },
				{ "a_Normal",	ShaderAttributeType::Float3 },
			}
		);
		m_VertexBuffer->SetLayout(layout);

		uint32_t* indexPtr = m_Mesh->GetIndices().data();
		m_IndexBuffer = CreateRef<IndexBuffer>(indexPtr, m_Mesh->GetIndices().size());

		m_VertexArray->EnableVertexAttributes(m_VertexBuffer);
		m_VertexArray->SetIndexBuffer(m_IndexBuffer);
	}
}

