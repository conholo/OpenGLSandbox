#pragma once

#include "Engine/Rendering/VertexBuffer.h"
#include "Engine/Rendering/IndexBuffer.h"
#include "Engine/Core/Memory.h"


namespace Engine
{
	class VertexArray
	{
	public:
		VertexArray();
		~VertexArray();

		void Bind() const;
		void Unbind() const;

		void ClearIndexBuffer() { m_IndexBuffer = nullptr; }
		void SetIndexBuffer(const Ref<IndexBuffer>& indexBuffer);
		const Ref<IndexBuffer>& GetIndexBuffer() const { return m_IndexBuffer; }
		void EnableVertexAttributes(const Ref<VertexBuffer>& vertexBuffer);

	private:
		Ref<IndexBuffer> m_IndexBuffer;
		uint32_t m_ID;
	};
}

