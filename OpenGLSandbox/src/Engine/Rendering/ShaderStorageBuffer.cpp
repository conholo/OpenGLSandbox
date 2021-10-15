#include "Engine/Rendering/ShaderStorageBuffer.h"

#include <glad/glad.h>


namespace Engine
{
	ShaderStorageBuffer::ShaderStorageBuffer(void* data, uint32_t size)
	{
		glCreateBuffers(1, &m_ID);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_ID);
		glBufferData(GL_SHADER_STORAGE_BUFFER, size, data, GL_STATIC_DRAW);
	}

	void ShaderStorageBuffer::Bind() const
	{
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_ID);
	}

	void ShaderStorageBuffer::Unbind() const
	{
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
	}

	void ShaderStorageBuffer::ReadSubData(void* data, uint32_t offset, uint32_t size)
	{
		glGetNamedBufferSubData(m_ID, offset, size, data);
	}

	void ShaderStorageBuffer::BindToComputeShader(uint32_t binding, uint32_t computeShaderID)
	{
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, binding, m_ID);
	}

	void ShaderStorageBuffer::ExecuteCompute(uint32_t index, uint32_t computeShaderID, uint32_t workGroupX, uint32_t workGroupY, uint32_t workGroupZ)
	{
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, index, m_ID);
		glUseProgram(computeShaderID);
		glDispatchCompute(workGroupX, workGroupY, workGroupZ);
	}
}

