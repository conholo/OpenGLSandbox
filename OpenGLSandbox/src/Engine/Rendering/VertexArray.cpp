#include "Engine/Rendering/VertexArray.h"
#include "Engine/Rendering/BufferLayout.h"
#include <glad/glad.h>

namespace Engine
{
	GLenum GLEnumFromShaderDataType(ShaderAttributeType type)
	{
		switch (type)
		{
		case ShaderAttributeType::Mat3:
		case ShaderAttributeType::Mat4:
		case ShaderAttributeType::Float:
		case ShaderAttributeType::Float2:
		case ShaderAttributeType::Float3:
		case ShaderAttributeType::Float4:	return GL_FLOAT;
		case ShaderAttributeType::Int:		return GL_INT;
		default:							return GL_FLOAT;
		}
	}

	VertexArray::VertexArray()
	{
		glCreateVertexArrays(1, &m_ID);
		glBindVertexArray(m_ID);
	}

	VertexArray::~VertexArray()
	{
		glDeleteVertexArrays(1, &m_ID);
	}

	void VertexArray::Bind() const
	{
		glBindVertexArray(m_ID);
	}

	void VertexArray::Unbind() const
	{
		glBindVertexArray(0);
	}

	void VertexArray::SetIndexBuffer(const Ref<IndexBuffer>& indexBuffer)
	{
		glBindVertexArray(m_ID);
		m_IndexBuffer = indexBuffer;
	}

	void VertexArray::EnableVertexAttributes(const Ref<VertexBuffer>& vertexBuffer)
	{
		glBindVertexArray(m_ID);
		vertexBuffer->Bind();

		const auto& layout = vertexBuffer->GetLayout();

		uint32_t index = 0;
		for (const auto& element : layout)
		{
			glEnableVertexAttribArray(index);
			glVertexAttribPointer(
				index,
				element.GetComponentCount(),
				GLEnumFromShaderDataType(element.Type),
				element.Normalized ? GL_TRUE : GL_FALSE,
				layout.GetStride(),
				(const void*)element.Offset
			);
			index++;
		}
	}
}

