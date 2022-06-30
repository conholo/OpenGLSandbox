#include "Engine/Rendering/EditorGrid.h"
#include "Engine/Rendering/Shader.h"
#include "Engine/Rendering/RenderCommand.h"

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>
#include <iostream>

namespace Engine
{

	EditorGrid::EditorGrid(float scale)
	{
		SetScale(scale);
		m_VAO = CreateRef<VertexArray>();

		m_FSQ = MeshFactory::Create(PrimitiveType::FullScreenQuad);
		float* baseVertexPtr = &m_FSQ->GetVertices()[0].Position.x;

		m_VBO = CreateRef<VertexBuffer>(baseVertexPtr, sizeof(Vertex) * m_FSQ->GetVertices().size());

		m_VBO->SetLayout
		(
			{
				{"a_Position",  ShaderAttributeType::Float3},
				{"a_TexCoord",	ShaderAttributeType::Float2},
				{"a_Normal",	ShaderAttributeType::Float3}
			}
		);

		m_EBO = CreateRef<IndexBuffer>(m_FSQ->GetIndices().data(), m_FSQ->GetIndices().size());
		m_VAO->EnableVertexAttributes(m_VBO);
		m_VAO->SetIndexBuffer(m_EBO);

		m_DebugLineVAO = CreateRef<VertexArray>();
		m_DebugLineVBO = CreateRef<VertexBuffer>(sizeof(glm::vec3) * 8);
		m_DebugLineVBO->SetLayout
		(
			{
				{"a_Position",  ShaderAttributeType::Float3},
			}
		);
		m_LineVertices.resize(8);

		m_DebugLineVAO->EnableVertexAttributes(m_DebugLineVBO);

		m_DebugPointVAO = CreateRef<VertexArray>();
		m_DebugPointVBO = CreateRef<VertexBuffer>(sizeof(glm::vec3) * 4);
		m_DebugPointVBO->SetLayout
		(
			{
				{"a_Position",  ShaderAttributeType::Float3},
			}
		);
		m_PointVertices.resize(4);

		m_DebugPointVAO->EnableVertexAttributes(m_DebugPointVBO);
	}

	void EditorGrid::Draw(const Camera& camera)
	{
		m_VAO->Bind();
		ShaderLibrary::Get(m_ShaderName)->Bind();
		ShaderLibrary::Get(m_ShaderName)->UploadUniformFloat("u_InnerGridScale", m_InnerGridScale * m_Scale);
		ShaderLibrary::Get(m_ShaderName)->UploadUniformFloat("u_OuterGridScale", m_OuterGridScale * m_Scale);
		ShaderLibrary::Get(m_ShaderName)->UploadUniformFloat("u_NearClip", camera.GetNearClip());
		ShaderLibrary::Get(m_ShaderName)->UploadUniformFloat("u_FarClip", camera.GetFarClip());
		ShaderLibrary::Get(m_ShaderName)->UploadUniformMat4("u_ViewMatrix", camera.GetView());
		ShaderLibrary::Get(m_ShaderName)->UploadUniformMat4("u_ProjectionMatrix", camera.GetProjection());
		ShaderLibrary::Get(m_ShaderName)->UploadUniformMat4("u_InverseViewMatrix", glm::inverse(camera.GetView()));
		ShaderLibrary::Get(m_ShaderName)->UploadUniformMat4("u_InverseProjectionMatrix", glm::inverse(camera.GetProjection()));

		RenderCommand::DrawIndexed(m_VAO);
		m_VAO->Unbind();
	}

	void EditorGrid::SetShader(const std::string& shaderName)
	{
		m_ShaderName = shaderName;
	}

}