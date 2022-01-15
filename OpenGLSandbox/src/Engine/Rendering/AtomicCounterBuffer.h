#pragma once

#include <stdint.h>

namespace Engine
{
	class AtomicCounterBuffer
	{
	public:
		AtomicCounterBuffer();
		~AtomicCounterBuffer();

		void SetCounter(uint32_t value);
		void BindToComputeShader(uint32_t binding);

		uint32_t GetID() const { return m_ID; }

	private:
		uint32_t m_ID;
	};
}

