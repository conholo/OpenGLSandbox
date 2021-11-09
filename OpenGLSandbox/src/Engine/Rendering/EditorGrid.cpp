#include "Engine/Rendering/EditorGrid.h"
#include "Engine/Rendering/Shader.h"

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>

namespace Engine
{
	static float GridVertices[]
	{
		 1,  1, 0,
		-1, -1, 0,
		-1,  1, 0,

		-1, -1, 0,
		 1,  1, 0,
		 1, -1, 0
	};


	EditorGrid::EditorGrid(float scale)
	{
		SetScale(scale);
		m_VAO = CreateRef<VertexArray>();

		m_VBO = CreateRef<VertexBuffer>(GridVertices, sizeof(GridVertices));

		m_VBO->SetLayout
		(
			{
				{"a_Position", ShaderAttributeType::Float3}
			}
		);

		m_VAO->EnableVertexAttributes(m_VBO);
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

		glDrawArrays(GL_TRIANGLES, 0, sizeof(GridVertices) / sizeof(float));
		m_VAO->Unbind();
	}
}