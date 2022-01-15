#include "Engine/Rendering/AtomicCounterBuffer.h"
#include <glad/glad.h>

namespace Engine
{
	AtomicCounterBuffer::AtomicCounterBuffer()
	{
		glGenBuffers(1, &m_ID);
		glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, m_ID);
		glBufferData(GL_ATOMIC_COUNTER_BUFFER, sizeof(GLuint), nullptr, GL_DYNAMIC_DRAW);
	}

	AtomicCounterBuffer::~AtomicCounterBuffer()
	{
		glDeleteBuffers(1, &m_ID);
	}

	void AtomicCounterBuffer::SetCounter(uint32_t value)
	{
		glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, m_ID);
		GLuint* counter = (GLuint*)glMapBufferRange(GL_ATOMIC_COUNTER_BUFFER, 0, sizeof(GLuint), GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT | GL_MAP_UNSYNCHRONIZED_BIT);
		counter[0] = value;
		glUnmapBuffer(GL_ATOMIC_COUNTER_BUFFER);
		glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, 0);
	}

	void AtomicCounterBuffer::BindToComputeShader(uint32_t binding)
	{
		glBindBufferBase(GL_ATOMIC_COUNTER_BUFFER, binding, m_ID);
	}
}