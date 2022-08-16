#include "Engine/Rendering/Texture.h"
#include <iostream>
#include <utility>

#include <memory>
#include <stb_image.h>
#include <stb_image_write.h>
#include <glad/glad.h>
#include <glm/glm.hpp>

namespace Engine
{
	Texture2D::Texture2D(const Texture2DSpecification& specification)
		:m_Specification(specification), m_Name(specification.Name)
	{
		glCreateTextures(GL_TEXTURE_2D, 1, &m_ID);
		glBindTexture(GL_TEXTURE_2D, m_ID);

		GLenum wrapS = ConvertWrapMode(specification.WrapModeS);
		GLenum wrapT = ConvertWrapMode(specification.WrapModeT);
		GLenum minFilter = ConvertMinMagFilterMode(specification.MinFilterMode);
		GLenum magFilter = ConvertMinMagFilterMode(specification.MagFilterMode);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapS);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minFilter);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, magFilter);

		// Immutable-format Texture
		// Contents of the image can be modified, but it's storage requirements may not change.
		// https://www.khronos.org/registry/OpenGL-Refpages/gl4/html/glTexStorage2D.xhtml

		uint32_t mips = GetMipLevelCount();
		glTextureStorage2D(m_ID, mips, ConvertInternalFormatMode(specification.InternalFormat), specification.Width, specification.Height);
	}

	Texture2D::Texture2D(const Texture2DSpecification& specification, void* data)
		:m_Specification(specification), m_Name(specification.Name)
	{
		glCreateTextures(GL_TEXTURE_2D, 1, &m_ID);
		glBindTexture(GL_TEXTURE_2D, m_ID);

		const GLenum wrapS = ConvertWrapMode(specification.WrapModeS);
		const GLenum wrapT = ConvertWrapMode(specification.WrapModeT);
		const GLenum minFilter = ConvertMinMagFilterMode(specification.MinFilterMode);
		const GLenum magFilter = ConvertMinMagFilterMode(specification.MagFilterMode);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapS);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minFilter);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, magFilter);

		const GLenum internalFormat = ConvertInternalFormatMode(m_Specification.InternalFormat);
		const GLenum dataFormat = ConverDataLayoutMode(m_Specification.PixelLayoutFormat);
		const GLenum dataType = ConvertImageDataType(m_Specification.DataType);

		glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, m_Specification.Width, m_Specification.Height, 0, dataFormat, dataType, data);
		glGenerateMipmap(GL_TEXTURE_2D);
	}

	Texture2D::Texture2D(const std::string& filePath, const Texture2DSpecification& specification)
		:m_Specification(specification), m_FilePath(filePath)
	{
		if(specification.Name == "Texture")
		{
			size_t pos = m_FilePath.find_last_of("/") + 1;
			size_t size = m_FilePath.size() - pos;
			m_Name = m_FilePath.substr(pos, size);
		}
		else
			m_Name = specification.Name;

		glCreateTextures(GL_TEXTURE_2D, 1, &m_ID);
		glBindTexture(GL_TEXTURE_2D, m_ID);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, ConvertWrapMode(specification.WrapModeS));
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, ConvertWrapMode(specification.WrapModeT));
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, ConvertMinMagFilterMode(specification.MinFilterMode));
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, ConvertMinMagFilterMode(specification.MagFilterMode));

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
			GLenum internalFormat = ConvertInternalFormatMode(m_Specification.InternalFormat);
			GLenum dataFormat = ConverDataLayoutMode(m_Specification.PixelLayoutFormat);
			GLenum dataType = ConvertImageDataType(m_Specification.DataType);
			glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, m_Specification.Width, m_Specification.Height, 0, dataFormat, dataType, data);
			glGenerateMipmap(GL_TEXTURE_2D);
			stbi_image_free(data);
		}
		else
		{
			std::cout << "Failed to load texture. " << m_Specification.Name << "\n";
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
			1, 1,
			"White Texture"
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
			1, 1,
			"Black Texture"
		};

		Ref<Texture2D> blackTexture = Engine::CreateRef<Engine::Texture2D>(blackTextureSpec);
		uint32_t blackTextureData = 0xff000000;
		blackTexture->SetData(&blackTextureData, sizeof(uint32_t));

		return blackTexture;
	}

	TextureCube::TextureCube(const TextureSpecification& specification, const std::vector<std::string>& cubeFaceFiles)
		:m_Specification(specification), m_Name(specification.Name)
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

				const auto MipCount = CalculateMipCount(specification.Width, specification.Height);
				glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, MipCount, internalFormat, width, height, 0, dataFormat, dataType, data);
				stbi_image_free(data);
			}
			else
			{
				std::cout << "Failed to load face: " << i << " for Cube Map Texture.  Aborting process." << std::endl;
				stbi_image_free(data);
			}
		}
		glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
		glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	}

	TextureCube::TextureCube(const TextureSpecification& specification)
		:m_Specification(specification), m_Name(specification.Name)
	{
		glCreateTextures(GL_TEXTURE_CUBE_MAP, 1, &m_ID);
		glBindTexture(GL_TEXTURE_CUBE_MAP, m_ID);

		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		const auto MipCount = CalculateMipCount(specification.Width, specification.Height);
		glTexStorage2D(GL_TEXTURE_CUBE_MAP, MipCount, ConvertInternalFormatMode(specification.InternalFormat), specification.Width, specification.Height);
		glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
		glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
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
		GLenum glShaderDataFormat = ConvertShaderFormatType(shaderDataFormat);
		GLenum internalFormat = ConvertInternalFormatMode(m_Specification.InternalFormat);

		if (glShaderDataFormat != internalFormat)
		{
			std::cout << "Shader Data Format and Internal format must match!" << "\n";
			return;
		}

		glBindImageTexture(unit, m_ID, level, GL_TRUE, 0, ImageUtils::ConvertTextureAccessLevel(access), ImageUtils::ConvertShaderFormatType(shaderDataFormat));
	}

	uint32_t TextureCube::CalculateMipCount(uint32_t Width, uint32_t Height)
	{
		return (uint32_t)std::floor(std::log2(glm::min(Width, Height))) + 1;
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
		:m_Specification(spec), m_Name(spec.Name)
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
		:m_Name(spec.Name)
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

		const GLenum minFilter = ConvertMinMagFilterMode(m_Specification.MinFilterMode);
		const GLenum magFilter = ConvertMinMagFilterMode(m_Specification.MagFilterMode);
		const GLenum wrapModeS = ConvertWrapMode(m_Specification.WrapModeS);
		const GLenum wrapModeT = ConvertWrapMode(m_Specification.WrapModeT);

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

	bool TextureLibrary::Has2D(const std::string& Name)
	{
		return s_NameToTexture2DLibrary.find(Name) != s_NameToTexture2DLibrary.end();
	}

	void TextureLibrary::AddTexture2D(const Ref<Texture2D>& texture)
	{
		if (s_NameToTexture2DLibrary.find(texture->GetName()) == s_NameToTexture2DLibrary.end())
		{
			s_NameToTexture2DLibrary[texture->GetName()] = texture;
			s_IdToNameLibrary[texture->GetID()] = texture->GetName();
			std::cout << "Added Texture2D with name: '" << texture->GetName() << "' to the Texture Library.\n";
		}
		else
		{
			std::cout << "Texture2D already contained in Texture Library." << "\n";
		}
	}

	Ref<Texture2D> TextureLibrary::LoadTexture2D(const Texture2DSpecification& spec, const std::string& filePath)
	{
		if (!filePath.empty())
		{
			Ref<Texture2D> texture = CreateRef<Texture2D>(filePath, spec);
			AddTexture2D(texture);
			return texture;
		}

		Ref<Texture2D> texture = CreateRef<Texture2D>(spec);
		AddTexture2D(texture);
		return texture;
	}

	bool TextureLibrary::HasCube(const std::string& Name)
	{
		return s_NameToTextureCubeLibrary.find(Name) != s_NameToTextureCubeLibrary.end();
	}

	void TextureLibrary::AddTextureCube(const Ref<TextureCube>& texture)
	{
		if (s_NameToTextureCubeLibrary.find(texture->GetName()) == s_NameToTextureCubeLibrary.end())
		{
			s_NameToTextureCubeLibrary[texture->GetName()] = texture;
			s_IdToNameLibrary[texture->GetID()] = texture->GetName();
			std::cout << "Added TextureCube with name: '" << texture->GetName() << "' to the Texture Library." << "\n";
		}
		else
		{
			std::cout << "TextureCube already contained in Texture Library - " << texture->GetName() << "\n";
		}
	}

	Ref<TextureCube> TextureLibrary::LoadTextureCube(const TextureSpecification& spec)
	{
		if(HasCube(spec.Name))
			return s_NameToTextureCubeLibrary[spec.Name];
		Ref<TextureCube> texture = CreateRef<TextureCube>(spec);
		AddTextureCube(texture);
		return texture;
	}

	Ref<Texture2D> TextureLibrary::Load(const std::string& filePath)
	{
		Texture2DSpecification defaultFromFileSpec =
		{
			ImageUtils::WrapMode::Repeat,
			ImageUtils::WrapMode::Repeat,
			ImageUtils::FilterMode::LinearMipLinear,
			ImageUtils::FilterMode::Linear,
			ImageUtils::ImageInternalFormat::FromImage,
			ImageUtils::ImageDataLayout::FromImage,
			ImageUtils::ImageDataType::UByte,
		};

		Ref<Texture2D> texture = CreateRef<Texture2D>(filePath, defaultFromFileSpec);
		AddTexture2D(texture);

		return texture;
	}

	Ref<Texture2D> TextureLibrary::LoadTexture2D(const Texture2DSpecification& Spec, void* Data)
	{
		if(Has2D(Spec.Name))
			return s_NameToTexture2DLibrary[Spec.Name];

		Texture2DSpecification defaultFromFileSpec =
		{
			ImageUtils::WrapMode::Repeat,
			ImageUtils::WrapMode::Repeat,
			ImageUtils::FilterMode::LinearMipLinear,
			ImageUtils::FilterMode::Linear,
			ImageUtils::ImageInternalFormat::FromImage,
			ImageUtils::ImageDataLayout::FromImage,
			ImageUtils::ImageDataType::UByte,
		};

		Ref<Texture2D> texture = CreateRef<Texture2D>(Spec, Data);
		AddTexture2D(texture);

		return texture;
	}


	const Ref<Texture2D>& TextureLibrary::Get2D(const std::string& name)
	{
		if (s_NameToTexture2DLibrary.find(name) == s_NameToTexture2DLibrary.end())
		{
			std::cout << "No Texture2D with name \"{}\" found in Texture Library." <<  name << "\n";
			return nullptr;
		}

		return s_NameToTexture2DLibrary.at(name);
	}

	const Ref<TextureCube>& TextureLibrary::GetCube(const std::string& name)
	{
		if (s_NameToTextureCubeLibrary.find(name) == s_NameToTextureCubeLibrary.end())
		{
			std::cout << "No TextureCube with name \"{}\" found in Texture Library." <<  name << "\n";
			return nullptr;
		}

		return s_NameToTextureCubeLibrary.at(name);
	}

	void TextureLibrary::BindTexture2DToSlot(const std::string& TwoDimensionTextureName, uint32_t Slot)
	{
		if(s_NameToTexture2DLibrary.find(TwoDimensionTextureName) == s_NameToTexture2DLibrary.end())
		{
			std::cout << "TextureLibrary: Unable to bind Texture2D with name '" << TwoDimensionTextureName << "'.  This texture has not been registered." << "\n";
			return;
		}
		
		const auto& Texture = s_NameToTexture2DLibrary[TwoDimensionTextureName];
		glBindTextureUnit(Slot, Texture->GetID());
	}

	void TextureLibrary::BindTextureCubeToSlot(const std::string& CubeTextureName, uint32_t Slot)
	{
		if(s_NameToTextureCubeLibrary.find(CubeTextureName) == s_NameToTextureCubeLibrary.end())
		{
			std::cout << "TextureLibrary: Unable to bind TextureCube with name '" << CubeTextureName << "'.  This texture has not been registered." << "\n";
			return;
		}
		
		const auto& Texture = s_NameToTextureCubeLibrary[CubeTextureName];
		glBindTextureUnit(Slot, Texture->GetID());
	}

	void TextureLibrary::BindTextureToSlot(uint32_t TexID, uint32_t Slot)
	{
		glBindTextureUnit(Slot, TexID);
	}

	std::string TextureLibrary::GetNameFromID(uint32_t TextureID)
	{
		if(s_IdToNameLibrary.find(TextureID) == s_IdToNameLibrary.end())
		{
			std::cout << "TextureLibrary: Unable to find texture with ID '" << TextureID << "'.";
			return {};
		}
		return s_IdToNameLibrary[TextureID];
	}

	std::unordered_map<std::string, Ref<Texture2D>> TextureLibrary::s_NameToTexture2DLibrary;
	std::unordered_map<std::string, Ref<TextureCube>> TextureLibrary::s_NameToTextureCubeLibrary;
	std::unordered_map<uint32_t, std::string> TextureLibrary::s_IdToNameLibrary;
}

