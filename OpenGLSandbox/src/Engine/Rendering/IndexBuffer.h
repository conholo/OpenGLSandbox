#pragma once
#include <stdint.h>


namespace Engine
{
	class IndexBuffer
	{
	public:
		IndexBuffer(uint32_t count);
		IndexBuffer(uint32_t* indices, uint32_t count);
		~IndexBuffer();

		void Bind() const;
		void Unbind() const;

		uint32_t GetID() const { return m_ID; }
		uint32_t GetIndexCount() const { return m_Count; }

	private:
		uint32_t m_Count;
		uint32_t m_ID;
	};
}

