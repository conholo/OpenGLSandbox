#pragma once

#include <algorithm>
#include <memory>
#include <assert.h>

using byte = uint8_t;

class Buffer
{
public:

	Buffer()
		:m_Data(nullptr), m_Size(0) { }
	Buffer(void* data, size_t size)
		:m_Data(data), m_Size(size) { }

	Buffer(const Buffer&) = delete;

	void Allocate(size_t size)
	{
		delete[] m_Data;
		m_Data = nullptr;

		if (size == 0)
			return;

		m_Data = new byte[size];
		m_Size = size;
	}

	void Release()
	{
		delete[] m_Data;
		m_Data = nullptr;
		m_Size = 0;
	}

	void ZeroInitialize()
	{
		if (!m_Data) return;

		memset(m_Data, 0, m_Size);
	}

	template<typename T>
	void Write(void* data, size_t size, size_t offset = 0)
	{
		assert(("Buffer Overflow: Size + Offset must be less than allocated buffer size."),  size + offset <= m_Size);

		memcpy((byte*)m_Data + offset, data, size);
	}

	template<typename T>
	T* Read(size_t offset = 0)
	{
		return (T*)((byte*)m_Data + offset);
	}

	byte& operator[](int index)
	{
		return ((byte*)m_Data)[index];
	}

	byte operator[](int index) const
	{
		return ((byte*)m_Data)[index];
	}

	template<typename T>
	T* As()
	{
		return (T*)m_Data;
	}

private:
	void* m_Data = nullptr;
	size_t m_Size = 0;
};