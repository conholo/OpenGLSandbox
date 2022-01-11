#pragma once

#include <stdint.h>

namespace Engine
{
	class ShaderStorageBuffer
	{
	public:
		ShaderStorageBuffer(void* data, uint32_t size);

		void Bind() const;
		void Unbind() const;

		void SetData(void* data, uint32_t offset, uint32_t size);
		void ReadSubData(void* data, uint32_t offset, uint32_t size);
		void BindToComputeShader(uint32_t binding, uint32_t computeShaderID);
		void ExecuteCompute(uint32_t index, uint32_t computeShaderID, uint32_t workGroupX, uint32_t workGroupY, uint32_t workGroupZ);

		uint32_t GetID() const { return m_ID; }

	private:
		uint32_t m_ID;
	};
}

