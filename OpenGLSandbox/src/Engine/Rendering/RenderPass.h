#pragma once

#include "Engine/Core/Memory.h"
#include "Engine/Rendering/Shader.h"
#include "Engine/Rendering/Framebuffer.h"
#include <glm/glm.hpp>

namespace Engine
{
	enum class PassType { None = 0, DefaultFBO, CustomFBO };

	struct RenderPassSpecification
	{
		PassType Type = PassType::CustomFBO;
		Ref<Framebuffer> TargetFramebuffer = nullptr;
		Ref<Shader> Shader = nullptr;
		uint32_t Flags;

		bool DepthWrite = true;
		bool DepthRead = true;
		bool ColorWrite = true;
		glm::vec4 ClearColor{ 0.0f };
	};

	class RenderPass
	{
	public:
		RenderPass(const RenderPassSpecification& specification);

		RenderPassSpecification& GetRenderPassSpecification() { return m_Specification; }
		const RenderPassSpecification& GetRenderPassSpecification() const { return m_Specification; }

	private:
		RenderPassSpecification m_Specification;
	};
}