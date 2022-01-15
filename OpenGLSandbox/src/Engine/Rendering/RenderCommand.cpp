#include "Engine/Rendering/RenderCommand.h"

#include <iostream>
#include <glad/glad.h>


namespace Engine
{
	void OpenGLMessageCallback(
		unsigned source,
		unsigned type,
		unsigned id,
		unsigned severity,
		int length,
		const char* message,
		const void* userParam
	)
	{
		std::cout << message << std::endl;
	}

	void RenderCommand::Initialize()
	{
		glEnable(GL_DEBUG_OUTPUT);
		glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
		glDebugMessageCallback(OpenGLMessageCallback, nullptr);

		glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_NOTIFICATION, 0, NULL, GL_FALSE);

		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glEnable(GL_DEPTH_TEST);
	}

	void RenderCommand::SetFaceCullMode(FaceCullMode cullMode)
	{
		glEnable(GL_CULL_FACE);
		glCullFace(cullMode == FaceCullMode::Front ? GL_FRONT : GL_BACK);

		if (cullMode == FaceCullMode::None)
			glDisable(GL_CULL_FACE);
	}

	void RenderCommand::SetFlags(uint32_t flags)
	{
		if (flags & (uint32_t)RenderFlag::DepthTest)
			glEnable(GL_DEPTH_TEST);
		else
			glDisable(GL_DEPTH_TEST);

		if (flags & (uint32_t)RenderFlag::Blend)
			glEnable(GL_BLEND);
		else
			glDisable(GL_BLEND);
	}

	void RenderCommand::SetDrawMode(DrawMode drawMode)
	{
		glPolygonMode(GL_FRONT_AND_BACK, drawMode == DrawMode::Fill ? GL_FILL : GL_LINE);
	}

	void RenderCommand::SetPointSize(float size)
	{
		glPointSize(size);
	}

	void RenderCommand::Clear(bool colorBufferBit, bool depthBufferBit)
	{
		if (colorBufferBit && depthBufferBit)
		{
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		}
		else
		{
			if (colorBufferBit)
			{
				glClear(GL_COLOR_BUFFER_BIT);
			}
			else if (depthBufferBit)
			{
				glClear(GL_DEPTH_BUFFER_BIT);
			}
		}
	}

	void RenderCommand::SetViewport(uint32_t width, uint32_t height)
	{
		glViewport(0, 0, width, height);
	}

	void RenderCommand::ClearColor(const glm::vec4& color)
	{
		glClearColor(color.r, color.g, color.b, color.a);
	}

	static GLenum GLTopologyFromEngineTopology(RenderTopology topology)
	{
		switch (topology)
		{
			case RenderTopology::Quads: return GL_QUADS;
			case RenderTopology::Triangles: return GL_TRIANGLES;
		}

		return 0;
	}

	void RenderCommand::DrawIndexed(const Ref<VertexArray>& vertexArray, uint32_t indexCount, RenderTopology topology)
	{
		uint32_t count = indexCount ? indexCount : vertexArray->GetIndexBuffer()->GetIndexCount();
		glDrawElements(GLTopologyFromEngineTopology(topology), count, GL_UNSIGNED_INT, nullptr);
	}

	void RenderCommand::DrawArrays(uint32_t vertexCount, uint32_t first, RenderTopology topology)
	{
		glDrawArrays(GLTopologyFromEngineTopology(topology), first, vertexCount);
	}

	void RenderCommand::DrawLine(LineTopology topology, uint32_t vertexCount, uint32_t first)
	{
		uint32_t count = topology == LineTopology::Lines ? 2 : vertexCount;
		glDrawArrays(topology == LineTopology::Lines ? GL_LINES : GL_LINE_STRIP, first, count);
	}

	void RenderCommand::DrawPoints(uint32_t vertexCount, uint32_t first)
	{
		glDrawArrays(GL_POINTS, first, vertexCount);
	}
}
