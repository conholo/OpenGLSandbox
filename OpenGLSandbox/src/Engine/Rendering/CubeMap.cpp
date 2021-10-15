#include "Engine/Rendering/CubeMap.h"
#include "Engine/Rendering/Mesh.h"
#include "Engine/Rendering/RenderCommand.h"

#include <iostream>

#include <stb_image.h>
#include <glad/glad.h>

namespace Engine
{
	uint32_t SkyBoxIndices[] =
	{
		// Back Face
		0, 1, 2,
		2, 3, 0,

		// Left Face
		4, 1, 0,
		0, 5, 4,

		// Right Face
		2, 6, 7,
		7, 3, 2,

		// Top Face
		0, 3, 7,
		7, 5, 0,

		// Front Face
		4, 5, 7,
		7, 6, 4,

		// Bottom Face
		4, 6, 2,
		2, 1, 4
	};

	float SkyBoxVertices[] = {
		-1.0f,  1.0f, -1.0f, // 0
		-1.0f, -1.0f, -1.0f, // 1
		 1.0f, -1.0f, -1.0f, // 2
		 1.0f,  1.0f, -1.0f, // 3

		-1.0f, -1.0f,  1.0f, // 4
		-1.0f,  1.0f,  1.0f, // 5
		 1.0f, -1.0f,  1.0f, // 6
		 1.0f,  1.0f,  1.0f  // 7
	};

	CubeMap::CubeMap(const Ref<Texture3D>& textureCube, const Ref<Shader>& shader)
		:m_TextureCube(textureCube), m_Shader(shader)
	{
		ConstructPipelinePrimitives();
	}

	CubeMap::~CubeMap()
	{
		glDeleteTextures(1, &m_ID);
	}

	void CubeMap::ConstructPipelinePrimitives()
	{
		m_VAO = CreateRef<VertexArray>();
		m_VBO = CreateRef<VertexBuffer>(SkyBoxVertices, sizeof(SkyBoxVertices));

		m_VBO->SetLayout
		(
			{
				{ "a_Position", ShaderAttributeType::Float3 },
			}
		);
		m_VAO->EnableVertexAttributes(m_VBO);

		Ref<IndexBuffer> ebo = CreateRef<IndexBuffer>(SkyBoxIndices, sizeof(SkyBoxIndices) / sizeof(uint32_t));
		m_VAO->SetIndexBuffer(ebo);
	}

	void CubeMap::Submit(const glm::mat4& viewProjection)
	{
		glDepthFunc(GL_LEQUAL);  // change depth function so depth test passes when values are equal to depth buffer's content
		m_Shader->Bind();
		m_Shader->UploadUniformMat4("u_ViewProjection", viewProjection);
		m_Shader->UploadUniformInt("u_Skybox", 0);
		m_TextureCube->BindToSamplerSlot(0);
		m_VAO->Bind();
		RenderCommand::DrawIndexed(m_VAO);
		m_TextureCube->Unbind();
		m_VAO->Unbind();
		m_Shader->Unbind();
		glDepthFunc(GL_LESS);
	}
}