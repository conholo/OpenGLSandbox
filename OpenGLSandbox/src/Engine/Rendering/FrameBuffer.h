#pragma once

#include <stdint.h>
#include <vector>

namespace Engine
{
	enum class FramebufferTextureFormat
	{
		None = 0,
		RGBA8,
		RED_INTEGER,
		DEPTH24STENCIL8,

		Depth = DEPTH24STENCIL8
	};

	struct FramebufferTextureSpecification
	{
		FramebufferTextureSpecification() = default;
		FramebufferTextureSpecification(FramebufferTextureFormat format)
			: TextureFormat(format) { }

		FramebufferTextureFormat TextureFormat;
	};

	struct FramebufferAttachmentSpecification
	{
		FramebufferAttachmentSpecification() = default;
		FramebufferAttachmentSpecification(std::initializer_list<FramebufferTextureSpecification> fboTextureSpecifications)
			:FBOTextureSpecifications(fboTextureSpecifications) { }

		std::vector<FramebufferTextureSpecification> FBOTextureSpecifications;
	};

	struct FramebufferSpecification
	{
		uint32_t Width = 0, Height = 0;
		
		FramebufferAttachmentSpecification AttachmentSpecification;
	};

	class Framebuffer
	{
	public:
		Framebuffer(const FramebufferSpecification& spec);
		~Framebuffer();

		void Bind() const;
		void Unbind() const;

		void Invalidate();
		void Resize(uint32_t width, uint32_t height);

		void BindDepthTexture(uint32_t slot = 0) const;
		void BindColorAttachment(uint32_t index = 0, uint32_t slot = 0) const;
		void ClearAttachment(uint32_t attachmentIndex, int value);
		uint32_t GetColorAttachmentID(uint32_t index = 0) const { return m_ColorAttachmentIDs[index]; }
		uint32_t GetDepthAttachmentID() const { return m_DepthAttachmentID; }
		const FramebufferSpecification& GetFramebufferSpecification() const { return m_Specification; }

	private:
		FramebufferSpecification m_Specification;
		uint32_t m_ID;

		std::vector<FramebufferTextureSpecification> m_ColorAttachmentTextureSpecs;
		std::vector<uint32_t> m_ColorAttachmentIDs;

		FramebufferTextureSpecification m_DepthAttachmentTextureSpec{ FramebufferTextureFormat::None };
		uint32_t m_DepthAttachmentID;
	};
}