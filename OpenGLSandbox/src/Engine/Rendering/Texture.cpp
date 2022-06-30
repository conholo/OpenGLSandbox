#include "Engine/Rendering/Texture.h"
#include <iostream>
#include <utility>

#include <memory>
#include <stb_image.h>
#include <stb_image_write.h>
#include <glad/glad.h>

namespace Engine
{
	Texture2D::Texture2D(const Texture2DSpecification& specification)
		:m_Specification(specification)
	{
		glCreateTextures(GL_TEXTURE_2D, 1, &m_ID);
		glBindTexture(GL_TEXTURE_2D, m_ID);

		GLenum wrapS = ImageUtils::ConvertWrapMode(specification.WrapModeS);
		GLenum wrapT = ImageUtils::ConvertWrapMode(specification.WrapModeT);
		GLenum minFilter = ImageUtils::ConvertMinMagFilterMode(specification.MinFilterMode);
		GLenum magFilter = ImageUtils::ConvertMinMagFilterMode(specification.MagFilterMode);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapS);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minFilter);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, magFilter);

		// Immutable-format Texture
		// Contents of the image can be modified, but it's storage requirements may not change.
		// https://www.khronos.org/registry/OpenGL-Refpages/gl4/html/glTexStorage2D.xhtml

		uint32_t mips = GetMipLevelCount();
		glTextureStorage2D(m_ID, mips, ImageUtils::ConvertInternalFormatMode(specification.InternalFormat), specification.Width, specification.Height);
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

	void Texture2D::Invalidate()
	{
		if (m_ID)
			glDeleteTextures(1, &m_ID);

		glCreateTextures(GL_TEXTURE_2D, 1, &m_ID);
		glBindTexture(GL_TEXTURE_2D, m_ID);

		GLenum wrapS = ImageUtils::ConvertWrapMode(m_Specification.WrapModeS);
		GLenum wrapT = ImageUtils::ConvertWrapMode(m_Specification.WrapModeT);
		GLenum minFilter = ImageUtils::ConvertMinMagFilterMode(m_Specification.MinFilterMode);
		GLenum magFilter = ImageUtils::ConvertMinMagFilterMode(m_Specification.MagFilterMode);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapS);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minFilter);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, magFilter);

		// Immutable-format Texture
		// Contents of the image can be modified, but it's storage requirements may not change.
		// https://www.khronos.org/registry/OpenGL-Refpages/gl4/html/glTexStorage2D.xhtml

		uint32_t mips = GetMipLevelCount();
		glTextureStorage2D(m_ID, mips, ImageUtils::ConvertInternalFormatMode(m_Specification.InternalFormat), m_Specification.Width, m_Specification.Height);
	}

	void Texture2D::Clear()
	{
		glClearTexImage(m_ID, 0, ImageUtils::ConverDataLayoutMode(m_Specification.PixelLayoutFormat), ImageUtils::ConvertImageDataType(m_Specification.DataType), nullptr);
	}

	void Texture2D::Resize(uint32_t width, uint32_t height)
	{
		m_Specification.Width = width;
		m_Specification.Height = height;
		Invalidate();
	}

	std::pair<uint32_t, uint32_t> Texture2D::GetMipSize(uint32_t mip) const
	{
		uint32_t width = m_Specification.Width;
		uint32_t height = m_Specification.Height;
		while (mip != 0)
		{
			width /= 2;
			height /= 2;
			mip--;
		}

		return { width, height };
	}

	uint32_t Texture2D::GetMipLevelCount() const
	{
		return ImageUtils::CalculateMipLevelCount(m_Specification.Width, m_Specification.Height);
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

	Ref<Texture2D> Texture2D::CreateBlackTexture()
	{
		Texture2DSpecification blackTextureSpec =
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

		Ref<Texture2D> blackTexture = Engine::CreateRef<Engine::Texture2D>(blackTextureSpec);
		uint32_t blackTextureData = 0xff000000;
		blackTexture->SetData(&blackTextureData, sizeof(uint32_t));

		return blackTexture;
	}

	TextureCube::TextureCube(const TextureSpecification& specification, const std::vector<std::string>& cubeFaceFiles)
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

	TextureCube::TextureCube(const TextureSpecification& specification, const void* data)
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

	TextureCube::~TextureCube()
	{
		glDeleteTextures(1, &m_ID);
	}

	void TextureCube::BindToSamplerSlot(uint32_t slot)
	{
		glActiveTexture(GL_TEXTURE0 + slot);
		glBindTextureUnit(slot, m_ID);
	}

	void TextureCube::Unbind() const
	{
		glBindTexture(GL_TEXTURE_2D, 0);
	}

	void TextureCube::BindToImageSlot(uint32_t unit, uint32_t level, ImageUtils::TextureAccessLevel access, ImageUtils::TextureShaderDataFormat shaderDataFormat)
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

	Texture2DImageView::Texture2DImageView(const Ref<Texture2D>& original, uint32_t baseMip, uint32_t mipCount, uint32_t baseLayer, uint32_t layerCount)
		:m_BaseMip(baseMip)
	{
		glGenTextures(1, &m_ID);
		GLenum internalFormat = ImageUtils::ConvertInternalFormatMode(original->GetSpecification().InternalFormat);
		glTextureView(m_ID, GL_TEXTURE_2D, original->GetID(), internalFormat, baseMip, mipCount, baseLayer, layerCount);
	}

	Texture2DImageView::~Texture2DImageView()
	{
		glDeleteTextures(1, &m_ID);
	}

	void Texture2DImageView::ChangeToMip(const Ref<Texture2D>& original, uint32_t baseMip, uint32_t mipCount, uint32_t baseLayer, uint32_t layerCount)
	{
		if(m_ID)
			glDeleteTextures(1, &m_ID);

		original->BindToSamplerSlot(0);
		GLint hasMips = 0;
		m_BaseMip = baseMip;
		glGenTextures(1, &m_ID);
		GLenum internalFormat = ImageUtils::ConvertInternalFormatMode(original->GetSpecification().InternalFormat);
		glTextureView(m_ID, GL_TEXTURE_2D, original->GetID(), internalFormat, baseMip, mipCount, baseLayer, layerCount);
	}

	void Texture2DImageView::Bind() const
	{
		glBindTexture(GL_TEXTURE_2D, m_ID);
	}

	void Texture2DImageView::Unbind() const
	{
		glBindTexture(GL_TEXTURE_2D, 0);
	}

	Texture3D::Texture3D(const TextureSpecification& spec)
		:m_Specification(spec)
	{
		glCreateTextures(GL_TEXTURE_3D, 1, &m_ID);
		glBindTexture(GL_TEXTURE_3D, m_ID);

		GLenum wrapS = ImageUtils::ConvertWrapMode(spec.SamplerWrap);
		GLenum wrapT = ImageUtils::ConvertWrapMode(spec.SamplerWrap);
		GLenum minFilter = ImageUtils::ConvertMinMagFilterMode(spec.Filter);
		GLenum magFilter = ImageUtils::ConvertMinMagFilterMode(spec.Filter);

		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, wrapS);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, wrapT);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, wrapT);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, minFilter);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, magFilter);

		// Immutable-format Texture
		// Contents of the image can be modified, but it's storage requirements may not change.
		// https://www.khronos.org/registry/OpenGL-Refpages/gl4/html/glTexStorage2D.xhtml

		uint32_t mips = GetMipLevelCount();

		// TODO:: Provide Texture3DSpecification
		glTextureStorage3D(m_ID, 1, ImageUtils::ConvertInternalFormatMode(spec.InternalFormat), spec.Width, spec.Height, spec.Width);
	}

	Texture3D::Texture3D(const Texture3DSpecification& spec)
	{
		glCreateTextures(GL_TEXTURE_3D, 1, &m_ID);
		glBindTexture(GL_TEXTURE_3D, m_ID);

		GLenum wrapS = ImageUtils::ConvertWrapMode(spec.WrapModeS);
		GLenum wrapT = ImageUtils::ConvertWrapMode(spec.WrapModeT);
		GLenum minFilter = ImageUtils::ConvertMinMagFilterMode(spec.MinFilterMode);
		GLenum magFilter = ImageUtils::ConvertMinMagFilterMode(spec.MagFilterMode);

		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, wrapS);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, wrapT);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, wrapT);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, minFilter);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, magFilter);

		// Immutable-format Texture
		// Contents of the image can be modified, but it's storage requirements may not change.
		// https://www.khronos.org/registry/OpenGL-Refpages/gl4/html/glTexStorage2D.xhtml

		uint32_t mips = GetMipLevelCount();

		// TODO:: Provide Texture3DSpecification
		glTextureStorage3D(m_ID, 1, ImageUtils::ConvertInternalFormatMode(spec.InternalFormat), spec.Width, spec.Height, spec.Depth);
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
		glBindTexture(GL_TEXTURE_3D, 0);
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

	int Texture3D::WriteToFile(const std::string& assetPath)
	{
		GLenum dataFormat = ImageUtils::ConverDataLayoutMode(m_Specification.DataLayout);
		GLenum dataType = ImageUtils::ConvertImageDataType(m_Specification.DataType);
		GLsizei textureSize = m_Specification.Width * m_Specification.Height * m_Specification.Width * 16;
		void* pixels = malloc(textureSize);
		glGetTextureSubImage(m_ID, 0, 0, 0, 0, m_Specification.Width, m_Specification.Height, m_Specification.Width, dataFormat, dataType, textureSize, pixels);

		int result = stbi_write_png(assetPath.c_str(), m_Specification.Width, m_Specification.Height, 4, pixels, 4);

		free(pixels);

		return result;
	}

	std::pair<glm::uint32_t, glm::uint32_t> Texture3D::GetMipSize(uint32_t mip) const
	{
		uint32_t width = m_Specification.Width;
		uint32_t height = m_Specification.Height;
		while (mip != 0)
		{
			width /= 2;
			height /= 2;
			mip--;
		}

		return { width, height };
	}

	uint32_t Texture3D::GetMipLevelCount() const
	{
		return ImageUtils::CalculateMipLevelCount(m_Specification.Width, m_Specification.Height);
	}

	Texture2DArray::Texture2DArray(const std::vector<std::string>& filePaths, const Texture2DSpecification& specification)
		:m_Specification(specification)
	{
		glCreateTextures(GL_TEXTURE_2D_ARRAY, 1, &m_ID);
		glBindTexture(GL_TEXTURE_2D_ARRAY, m_ID);
		glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_RGBA8, m_Specification.Width, m_Specification.Height, filePaths.size(), 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

		int width, height, channels;

		for (uint32_t i = 0; i < filePaths.size(); i++)
		{
			unsigned char* data = stbi_load(filePaths[i].c_str(), &width, &height, &channels, 0);
			m_Specification.InternalFormat = channels == 4 ? ImageUtils::ImageInternalFormat::RGBA8 : ImageUtils::ImageInternalFormat::RGB8;
			m_Specification.PixelLayoutFormat = channels == 4 ? ImageUtils::ImageDataLayout::RGBA : ImageUtils::ImageDataLayout::RGB;

			glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, i, m_Specification.Width, m_Specification.Height, 1, GL_RGBA, GL_UNSIGNED_BYTE, data);
			stbi_image_free(data);
		}

		GLenum minFilter = ImageUtils::ConvertMinMagFilterMode(m_Specification.MinFilterMode);
		GLenum magFilter = ImageUtils::ConvertMinMagFilterMode(m_Specification.MagFilterMode);
		GLenum wrapModeS = ImageUtils::ConvertWrapMode(m_Specification.WrapModeS);
		GLenum wrapModeT = ImageUtils::ConvertWrapMode(m_Specification.WrapModeT);

		glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, wrapModeS);
		glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, wrapModeT);
		glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, minFilter);
		glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, magFilter);
		glGenerateMipmap(GL_TEXTURE_2D_ARRAY);
	}

	Texture2DArray::~Texture2DArray()
	{
		glDeleteTextures(1, &m_ID);
	}

	std::pair<glm::uint32_t, glm::uint32_t> Texture2DArray::GetMipSize(uint32_t mip) const
	{
		uint32_t width = m_Specification.Width;
		uint32_t height = m_Specification.Height;
		while (mip != 0)
		{
			width /= 2;
			height /= 2;
			mip--;
		}

		return { width, height };
	}

	uint32_t Texture2DArray::GetMipLevelCount() const
	{
		return ImageUtils::CalculateMipLevelCount(m_Specification.Width, m_Specification.Height);
	}

	void Texture2DArray::BindToSamplerSlot(uint32_t slot)
	{
		glActiveTexture(GL_TEXTURE0 + slot);
		glBindTextureUnit(slot, m_ID);
	}

	void Texture2DArray::Unbind() const
	{
		glBindTexture(GL_TEXTURE_3D, 0);
	}

	void Texture2DArray::BindToImageSlot(uint32_t unit, uint32_t level, ImageUtils::TextureAccessLevel access, ImageUtils::TextureShaderDataFormat shaderDataFormat)
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

