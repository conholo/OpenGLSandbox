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
		ShaderLibrary::Get("InfiniteGrid")->Bind();
		ShaderLibrary::Get("InfiniteGrid")->UploadUniformFloat("u_InnerGridScale", m_InnerGridScale * m_Scale);
		ShaderLibrary::Get("InfiniteGrid")->UploadUniformFloat("u_OuterGridScale", m_OuterGridScale * m_Scale);
		ShaderLibrary::Get("InfiniteGrid")->UploadUniformFloat("u_NearClip", camera.GetNearClip());
		ShaderLibrary::Get("InfiniteGrid")->UploadUniformFloat("u_FarClip", camera.GetFarClip());
		ShaderLibrary::Get("InfiniteGrid")->UploadUniformMat4("u_ViewMatrix", camera.GetView());
		ShaderLibrary::Get("InfiniteGrid")->UploadUniformMat4("u_ProjectionMatrix", camera.GetProjection());
		ShaderLibrary::Get("InfiniteGrid")->UploadUniformMat4("u_InverseViewMatrix", glm::inverse(camera.GetView()));
		ShaderLibrary::Get("InfiniteGrid")->UploadUniformMat4("u_InverseProjectionMatrix", glm::inverse(camera.GetProjection()));

		RenderCommand::DrawIndexed(m_VAO);
		m_VAO->Unbind();


		//m_DebugLineVAO->Bind();
		//ShaderLibrary::Get("LineShader")->Bind();
		//ShaderLibrary::Get("LineShader")->UploadUniformMat4("u_MVP", camera.GetViewProjection());
		//ShaderLibrary::Get("LineShader")->UploadUniformFloat3("u_Color", {1.0f, 0.0f, 0.0f});
		//
		//
		//uint32_t vertexIndex = 0;
		//for (uint32_t i = 0; i < m_FSQ->GetVertices().size(); i++)
		//{
		//	glm::vec3 pos = m_FSQ->GetVertices()[i].Position;
		//
		//	glm::mat4 inverseVP = glm::inverse(camera.GetViewProjection());
		//	glm::vec4 unprojectedNear = inverseVP * glm::vec4(glm::vec3(pos.x, pos.y, 0.0), 1.0);
		//	glm::vec3 near = glm::vec3(unprojectedNear) / unprojectedNear.w;
		//	glm::vec4 unprojectedFar = inverseVP * glm::vec4(glm::vec3(pos.x, pos.y, 1.0), 1.0);
		//	glm::vec3 far = glm::vec3(unprojectedFar) / unprojectedFar.w;
		//
		//	float t = -near.y / (far.y - near.y);
		//	std::cout << t << std::endl;
		//	glm::vec3 fragPosition = near + t * (far - near);
		//
		//	m_PointVertices[i] = fragPosition;
		//	m_LineVertices[vertexIndex] = glm::vec3(unprojectedNear);
		//	m_LineVertices[vertexIndex + 1] = glm::vec3(unprojectedFar);
		//	vertexIndex += 2;
		//}
		//
		//m_DebugLineVBO->SetData(m_LineVertices.data(), sizeof(glm::vec3) * m_LineVertices.size());
		//
		//for (uint32_t i = 0; i < m_LineVertices.size(); i += 2)
		//	RenderCommand::DrawLine(LineTopology::Lines, 2, i);
		//
		//m_DebugLineVAO->Unbind();
		//
		//
		//m_DebugPointVAO->Bind();
		//RenderCommand::SetPointSize(50.0f);
		//ShaderLibrary::Get("LineShader")->Bind();
		//ShaderLibrary::Get("LineShader")->UploadUniformMat4("u_MVP", camera.GetViewProjection());
		//ShaderLibrary::Get("LineShader")->UploadUniformFloat3("u_Color", { 0.0f, 1.0f, 0.0f });
		//
		//m_DebugPointVBO->SetData(m_PointVertices.data(), sizeof(glm::vec3) * m_PointVertices.size());
		//RenderCommand::DrawPoints(m_PointVertices.size());
		//m_DebugPointVAO->Unbind();
	}
}