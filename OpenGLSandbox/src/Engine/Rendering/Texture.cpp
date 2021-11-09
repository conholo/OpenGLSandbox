#include "Engine/Rendering/Texture.h"
#include <iostream>

#include <stb_image.h>
#include <glad/glad.h>

namespace Engine
{
	Texture2D::Texture2D(const Texture2DSpecification& specification)
		:m_Specification(specification)
	{
		glCreateTextures(GL_TEXTURE_2D, 1, &m_ID);
		glBindTexture(GL_TEXTURE_2D, m_ID);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		// Immutable-format Texture
		// Contents of the image can be modified, but it's storage requirements may not change.
		// https://www.khronos.org/registry/OpenGL-Refpages/gl4/html/glTexStorage2D.xhtml
		glTextureStorage2D(m_ID, 1, ImageUtils::ConvertInternalFormatMode(specification.InternalFormat), specification.Width, specification.Height);
	}

	Texture2D::Texture2D(const std::string& filePath, const Texture2DSpecification& specification)
		:m_Specification(specification)
	{
		glCreateTextures(GL_TEXTURE_2D, 1, &m_ID);
		glBindTexture(GL_TEXTURE_2D, m_ID);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, ImageUtils::ConvertWrapMode(specification.WrapModeS));
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, ImageUtils::ConvertWrapMode(specification.WrapModeT));
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, ImageUtils::ConvertMinMagFilterMode(specification.MinFilterMode));
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, ImageUtils::ConvertMinMagFilterMode(specification.MagFilterMode));

		stbi_set_flip_vertically_on_load(1);

		int channels, width, height;
		unsigned char* data = stbi_load(filePath.c_str(), &width, &height, &channels, 0);

		m_Specification.Width = width;
		m_Specification.Height = height;

		if (m_Specification.InternalFormat == ImageUtils::ImageInternalFormat::FromImage && m_Specification.PixelLayoutFormat == ImageUtils::ImageDataLayout::FromImage)
		{
			switch (channels)
			{
				case 1:
				{
					m_Specification.InternalFormat = ImageUtils::ImageInternalFormat::R8;
					m_Specification.PixelLayoutFormat = ImageUtils::ImageDataLayout::Red;
					break;
				}
				case 2:
				{
					m_Specification.InternalFormat = ImageUtils::ImageInternalFormat::RG8;
					m_Specification.PixelLayoutFormat = ImageUtils::ImageDataLayout::RG;
					break;
				}
				case 3:
				{
					m_Specification.InternalFormat = ImageUtils::ImageInternalFormat::RGB8;
					m_Specification.PixelLayoutFormat = ImageUtils::ImageDataLayout::RGB;
					break;
				}
				case 4:
				{
					m_Specification.InternalFormat = ImageUtils::ImageInternalFormat::RGBA8;
					m_Specification.PixelLayoutFormat = ImageUtils::ImageDataLayout::RGBA;
					break;
				}
			}
		}

		if (data)
		{
			GLenum internalFormat = ImageUtils::ConvertInternalFormatMode(m_Specification.InternalFormat);
			GLenum dataFormat = ImageUtils::ConverDataLayoutMode(m_Specification.PixelLayoutFormat);
			GLenum dataType = ImageUtils::ConvertImageDataType(m_Specification.DataType);
			glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, m_Specification.Width, m_Specification.Height, 0, dataFormat, dataType, data);
			glGenerateMipmap(GL_TEXTURE_2D);

			stbi_image_free(data);
		}
		else
		{
			std::cout << "Failed to load texture." << "\n";
		}
	}

	Texture2D::~Texture2D()
	{
		glDeleteTextures(1, &m_ID);
	}

	void Texture2D::BindTextureIDToSamplerSlot(uint32_t slot, uint32_t id)
	{
		glActiveTexture(GL_TEXTURE0 + slot);
		glBindTextureUnit(slot, id);
	}

	void Texture2D::BindToSamplerSlot(uint32_t slot)
	{
		glActiveTexture(GL_TEXTURE0 + slot);
		glBindTextureUnit(slot, m_ID);
	}

	void Texture2D::Unbind() const
	{
		glBindTexture(GL_TEXTURE_2D, 0);
	}

	void Texture2D::BindToImageSlot(uint32_t unit, uint32_t level, ImageUtils::TextureAccessLevel access, ImageUtils::TextureShaderDataFormat shaderDataFormat)
	{
		GLenum glShaderDataFormat = ImageUtils::ConvertShaderFormatType(shaderDataFormat);
		GLenum internalFormat = ImageUtils::ConvertInternalFormatMode(m_Specification.InternalFormat);

		if (glShaderDataFormat != internalFormat)
		{
			std::cout << "Shader Data Format and Internal format must match!" << "\n";
			return;
		}

		glBindImageTexture(unit, m_ID, level, GL_FALSE, 0, ImageUtils::ConvertTextureAccessLevel(access), ImageUtils::ConvertShaderFormatType(shaderDataFormat));
	}

	void Texture2D::SetData(void* data, uint32_t size)
	{
		uint32_t bytesPerPixel = m_Specification.PixelLayoutFormat == ImageUtils::ImageDataLayout::RGBA ? 4 : 3;
		if (size != bytesPerPixel * m_Specification.Width * m_Specification.Height)
		{
			std::cout << "Data size must match entire texture." << std::endl;
			return;
		}

		GLenum pixelLayout = ImageUtils::ConverDataLayoutMode(m_Specification.PixelLayoutFormat);
		GLenum type = ImageUtils::ConvertImageDataType(m_Specification.DataType);
		glTextureSubImage2D(m_ID, 0, 0, 0, m_Specification.Width, m_Specification.Height, pixelLayout, type, data);
	}

	Ref<Texture2D> Texture2D::CreateWhiteTexture()
	{
		Texture2DSpecification whiteTextureSpec =
		{
			Engine::ImageUtils::WrapMode::Repeat,
			Engine::ImageUtils::WrapMode::Repeat,
			Engine::ImageUtils::FilterMode::Linear,
			Engine::ImageUtils::FilterMode::Linear,
			Engine::ImageUtils::ImageInternalFormat::RGBA8,
			Engine::ImageUtils::ImageDataLayout::RGBA,
			Engine::ImageUtils::ImageDataType::UByte,
			1, 1
		};

		Ref<Texture2D> whiteTexture = Engine::CreateRef<Engine::Texture2D>(whiteTextureSpec);
		uint32_t whiteTextureData = 0xffffffff;
		whiteTexture->SetData(&whiteTextureData, sizeof(uint32_t));

		return whiteTexture;
	}

	Texture3D::Texture3D(const TextureSpecification& specification, const std::vector<std::string>& cubeFaceFiles)
		:m_Specification(specification)
	{
		glCreateTextures(GL_TEXTURE_CUBE_MAP, 1, &m_ID);
		glBindTexture(GL_TEXTURE_CUBE_MAP, m_ID);

		int width, height, channels;

		for (uint32_t i = 0; i < cubeFaceFiles.size(); i++)
		{
			unsigned char* data = stbi_load(cubeFaceFiles[i].c_str(), &width, &height, &channels, 0);

			if (data)
			{
				m_Specification.InternalFormat = channels == 4 ? ImageUtils::ImageInternalFormat::RGBA8 : ImageUtils::ImageInternalFormat::RGB8;
				m_Specification.DataLayout = channels == 4 ? ImageUtils::ImageDataLayout::RGBA : ImageUtils::ImageDataLayout::RGB;

				GLenum internalFormat = ImageUtils::ConvertInternalFormatMode(m_Specification.InternalFormat);
				GLenum dataFormat = ImageUtils::ConverDataLayoutMode(m_Specification.DataLayout);
				GLenum dataType = ImageUtils::ConvertImageDataType(m_Specification.DataType);
				m_Specification.Width = width;
				m_Specification.Height = height;

				glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, internalFormat, width, height, 0, dataFormat, dataType, data);
				stbi_image_free(data);
			}
			else
			{
				std::cout << "Failed to load face: " << i << " for Cube Map Texture.  Aborting process." << std::endl;
				stbi_image_free(data);
			}
		}

		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	}

	Texture3D::Texture3D(const TextureSpecification& specification, const void* data)
		:m_Specification(specification)
	{
		glCreateTextures(GL_TEXTURE_CUBE_MAP, 1, &m_ID);
		glBindTexture(GL_TEXTURE_CUBE_MAP, m_ID);

		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glTexStorage2D(GL_TEXTURE_CUBE_MAP, 1, ImageUtils::ConvertInternalFormatMode(specification.InternalFormat), specification.Width, specification.Height);
	}

	Texture3D::~Texture3D()
	{
		glDeleteTextures(1, &m_ID);
	}

	void Texture3D::BindToSamplerSlot(uint32_t slot)
	{
		glActiveTexture(GL_TEXTURE0 + slot);
		glBindTextureUnit(slot, m_ID);
	}

	void Texture3D::Unbind() const
	{
		glBindTexture(GL_TEXTURE_2D, 0);
	}

	void Texture3D::BindToImageSlot(uint32_t unit, uint32_t level, ImageUtils::TextureAccessLevel access, ImageUtils::TextureShaderDataFormat shaderDataFormat)
	{
		GLenum glShaderDataFormat = ImageUtils::ConvertShaderFormatType(shaderDataFormat);
		GLenum internalFormat = ImageUtils::ConvertInternalFormatMode(m_Specification.InternalFormat);

		if (glShaderDataFormat != internalFormat)
		{
			std::cout << "Shader Data Format and Internal format must match!" << "\n";
			return;
		}

		glBindImageTexture(unit, m_ID, level, GL_TRUE, 0, ImageUtils::ConvertTextureAccessLevel(access), ImageUtils::ConvertShaderFormatType(shaderDataFormat));
	}
}

