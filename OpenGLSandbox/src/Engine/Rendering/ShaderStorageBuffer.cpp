#include "Engine/Rendering/ShaderStorageBuffer.h"

#include <iostream>
#include <memory.h>
#include <glad/glad.h>


namespace Engine
{
	ShaderStorageBuffer::ShaderStorageBuffer(void* data, uint32_t size)
		:m_AllocatedSize(size)
	{
		glCreateBuffers(1, &m_ID);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_ID);
		glBufferData(GL_SHADER_STORAGE_BUFFER, size, data, GL_DYNAMIC_DRAW);
	}

	ShaderStorageBuffer::ShaderStorageBuffer(uint32_t size)
		:m_AllocatedSize(size)
	{
		glCreateBuffers(1, &m_ID);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_ID);
		glBufferData(GL_SHADER_STORAGE_BUFFER, size, nullptr, GL_DYNAMIC_DRAW);
	}

	void ShaderStorageBuffer::Bind() const
	{
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_ID);
	}

	void ShaderStorageBuffer::Unbind() const
	{
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
	}

	void ShaderStorageBuffer::SetData(void* data, uint32_t offset, uint32_t size)
	{
		if (offset + size > m_AllocatedSize)
		{
			std::cout << "SSBO buffer overflow at SetData for ssbo: " << m_ID << std::endl;
			return;
		}

		glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_ID);
		glBufferSubData(GL_SHADER_STORAGE_BUFFER, offset, size, data);
	}

	void* ShaderStorageBuffer::GetData()
	{
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_ID);
		void* bufferData = glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_READ_WRITE);
		glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
		return bufferData;
	}

	void* ShaderStorageBuffer::GetData(uint32_t size, uint32_t offset)
	{
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_ID);
		void* bufferData = glMapBufferRange(GL_SHADER_STORAGE_BUFFER, offset, size, GL_MAP_READ_BIT | GL_MAP_WRITE_BIT);
		glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
		return bufferData;
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

	void ShaderStorageBuffer::CopyData(uint32_t writeTargetID, uint32_t readOffset, uint32_t writeOffset, uint32_t size)
	{
		glCopyNamedBufferSubData(m_ID, writeTargetID, readOffset, writeOffset, size);
	}
}

