#include "Engine/Rendering/FrameBuffer.h"
#include <glad/glad.h>

#include <iostream>

namespace Engine
{
	static GLenum TextureFormatToGLenum(FramebufferTextureFormat format)
	{
		switch (format)
		{
			case FramebufferTextureFormat::RGBA32F:
			case FramebufferTextureFormat::RGBA8: 
			{
				return GL_RGBA;
			}
			case FramebufferTextureFormat::RED_INTEGER: 
			{
				return GL_RED_INTEGER;
			}
			case FramebufferTextureFormat::DEPTH24STENCIL8: 
			{
				return GL_DEPTH24_STENCIL8;
			}
			default:	return 0;
		}
	}

	static void AttachColorTexture(uint32_t attachmentId, GLenum internalFormat, GLenum dataFormat, GLenum dataType, uint32_t width, uint32_t height, uint32_t index)
	{
		glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, dataFormat, dataType, nullptr);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + index, GL_TEXTURE_2D, attachmentId, 0);
	}

	static void AttachDepthTexture(uint32_t attachmentId, GLenum internalFormat, GLenum depthAttachmentType, uint32_t width, uint32_t height)
	{
		glTexStorage2D(GL_TEXTURE_2D, 1, internalFormat, width, height);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		glFramebufferTexture2D(GL_FRAMEBUFFER, depthAttachmentType, GL_TEXTURE_2D, attachmentId, 0);
	}

	Framebuffer::Framebuffer(const FramebufferSpecification& spec)
		:m_Specification(spec)
	{
		for (auto textureFormatSpecification : m_Specification.AttachmentSpecification.FBOTextureSpecifications)
		{
			if (textureFormatSpecification.TextureFormat != FramebufferTextureFormat::DEPTH24STENCIL8)
				m_ColorAttachmentTextureSpecs.emplace_back(textureFormatSpecification);
			else
				m_DepthAttachmentTextureSpec = textureFormatSpecification;
		}

		Invalidate();
	}

	Framebuffer::~Framebuffer()
	{
		glDeleteFramebuffers(1, &m_ID);
	}

	void Framebuffer::Bind() const
	{
		glBindFramebuffer(GL_FRAMEBUFFER, m_ID);
		glViewport(0, 0, m_Specification.Width, m_Specification.Height);
	}

	void Framebuffer::Unbind() const
	{
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	void Framebuffer::Invalidate()
	{
		if (m_ID)
		{
			glDeleteFramebuffers(1, &m_ID);
			glDeleteTextures(m_ColorAttachmentIDs.size(), m_ColorAttachmentIDs.data());
			glDeleteTextures(1, &m_DepthAttachmentID);

			m_ColorAttachmentIDs.clear();
			m_DepthAttachmentID = 0;
		}

		glCreateFramebuffers(1, &m_ID);
		glBindFramebuffer(GL_FRAMEBUFFER, m_ID);

		if (m_ColorAttachmentTextureSpecs.size())
		{
			m_ColorAttachmentIDs.resize(m_ColorAttachmentTextureSpecs.size());

			glCreateTextures(GL_TEXTURE_2D, m_ColorAttachmentIDs.size(), m_ColorAttachmentIDs.data());

			for (size_t i = 0; i < m_ColorAttachmentIDs.size(); i++)
			{
				glBindTexture(GL_TEXTURE_2D, m_ColorAttachmentIDs[i]);

				switch (m_ColorAttachmentTextureSpecs[i].TextureFormat)
				{
					case FramebufferTextureFormat::RGBA8:
					{
						AttachColorTexture(m_ColorAttachmentIDs[i], GL_RGBA8, GL_RGBA, GL_UNSIGNED_INT, m_Specification.Width, m_Specification.Height, i);
						break;
					}
					case FramebufferTextureFormat::RGBA32F:
					{
						AttachColorTexture(m_ColorAttachmentIDs[i], GL_RGBA32F, GL_RGBA, GL_FLOAT, m_Specification.Width, m_Specification.Height, i);
						break;
					}
					case FramebufferTextureFormat::RED_INTEGER:
					{
						AttachColorTexture(m_ColorAttachmentIDs[i], GL_R32I, GL_RED_INTEGER, GL_INT, m_Specification.Width, m_Specification.Height, i);
						break;
					}
				}
			}
		}

		if (m_DepthAttachmentTextureSpec.TextureFormat != FramebufferTextureFormat::None)
		{
			glCreateTextures(GL_TEXTURE_2D, 1, &m_DepthAttachmentID);
			glBindTexture(GL_TEXTURE_2D, m_DepthAttachmentID);

			AttachDepthTexture(m_DepthAttachmentID, GL_DEPTH24_STENCIL8, GL_DEPTH_STENCIL_ATTACHMENT, m_Specification.Width, m_Specification.Height);
		}

		if (m_ColorAttachmentIDs.size() > 1)
		{
			GLenum buffers[4] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3 };
			glDrawBuffers(m_ColorAttachmentIDs.size(), buffers);
		}
		else if (m_ColorAttachmentIDs.empty())
		{
			glDrawBuffer(GL_NONE);
			glReadBuffer(GL_NONE);
		}

		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		{
			// TODO:: Assert
			std::cout << m_ID << ": Framebuffer Incomplete." << "\n";
		}

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	void Framebuffer::Resize(uint32_t width, uint32_t height)
	{
		m_Specification.Width = width;
		m_Specification.Height = height;

		Invalidate();
	}

	void Framebuffer::BindDepthTexture(uint32_t slot) const
	{
		glActiveTexture(GL_TEXTURE0 + slot);
		glBindTextureUnit(slot, m_DepthAttachmentID);
	}

	void Framebuffer::BindColorAttachment(uint32_t index, uint32_t slot) const
	{
		glActiveTexture(GL_TEXTURE0 + slot);
		glBindTextureUnit(slot, m_ColorAttachmentIDs[index]);
	}

	void Framebuffer::UnbindColorAttachment(uint32_t index, uint32_t slot) const
	{
		glActiveTexture(GL_TEXTURE0 + slot);
		glBindTextureUnit(slot, 0);
	}

	void Framebuffer::ClearAttachment(uint32_t attachmentIndex, int value)
	{
		auto& specification = m_ColorAttachmentTextureSpecs[attachmentIndex];
		uint32_t texID = m_ColorAttachmentIDs[attachmentIndex];

		GLenum format = TextureFormatToGLenum(specification.TextureFormat);

		glClearTexImage(texID, 0, format, GL_INT, &value);
	}
}