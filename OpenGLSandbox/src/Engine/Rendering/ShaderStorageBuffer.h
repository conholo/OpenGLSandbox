#pragma once

namespace Engine
{
	class ShaderStorageBuffer
	{
	public:
		ShaderStorageBuffer(void* data, uint32_t size);
		ShaderStorageBuffer(uint32_t size);

		void Bind() const;
		void Unbind() const;

		void UseAndSetBinding(uint32_t binding) const;
		void SetData(void* data, uint32_t offset, uint32_t size);
		void* GetData();
		void* GetData(uint32_t size, uint32_t offset = 0);
		void BindToComputeShader(uint32_t binding, uint32_t computeShaderID);
		void ExecuteCompute(uint32_t index, uint32_t computeShaderID, uint32_t workGroupX, uint32_t workGroupY, uint32_t workGroupZ);

		void CopyData(uint32_t writeTargetID, uint32_t readOffset, uint32_t writeOffset, uint32_t size);

		uint32_t GetID() const { return m_ID; }

	private:
		uint32_t m_AllocatedSize = 0;
		uint32_t m_ID;
	};
}

