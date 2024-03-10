#include "epch.h"
#include "Engine/Rendering/Texture.h"

#include <stbi/stb_image.h>
#include <stbi/stb_image_write.h>
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
		if(specification.Name == "Texture2D")
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

		GLenum wrapS = ConvertWrapMode(m_Specification.WrapModeS);
		GLenum wrapT = ConvertWrapMode(m_Specification.WrapModeT);
		GLenum minFilter = ConvertMinMagFilterMode(m_Specification.MinFilterMode);
		GLenum magFilter = ConvertMinMagFilterMode(m_Specification.MagFilterMode);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapS);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minFilter);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, magFilter);

		// Immutable-format Texture
		// Contents of the image can be modified, but it's storage requirements may not change.
		// https://www.khronos.org/registry/OpenGL-Refpages/gl4/html/glTexStorage2D.xhtml

		uint32_t mips = GetMipLevelCount();
		glTextureStorage2D(m_ID, mips, ConvertInternalFormatMode(m_Specification.InternalFormat), m_Specification.Width, m_Specification.Height);
	}

	void Texture2D::Clear()
	{
		glClearTexImage(m_ID, 0, ConverDataLayoutMode(m_Specification.PixelLayoutFormat), ImageUtils::ConvertImageDataType(m_Specification.DataType), nullptr);
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

	void Texture2D::BindToSamplerSlot(uint32_t slot) const
	{
		glActiveTexture(GL_TEXTURE0 + slot);
		glBindTextureUnit(slot, m_ID);
	}

	void Texture2D::Unbind()
	{
		glBindTexture(GL_TEXTURE_2D, 0);
	}

	void Texture2D::BindToImageSlot(uint32_t unit, uint32_t level, ImageUtils::TextureAccessLevel access, ImageUtils::TextureShaderDataFormat shaderDataFormat)
	{
		GLenum glShaderDataFormat = ImageUtils::ConvertShaderFormatType(shaderDataFormat);
		GLenum internalFormat = ImageUtils::ConvertInternalFormatMode(m_Specification.InternalFormat);

		if (glShaderDataFormat != internalFormat)
		{
			LOG_ERROR("Failure Binding '{}' Texture2D to Image Slot: Shader Data Format and Internal format must match!", m_Specification.Name);
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

	TextureCube::TextureCube(const TextureCubeSpecification& specification, const std::vector<std::string>& cubeFaceFiles)
		:m_Specification(specification), m_Name(specification.Name)
	{
		glCreateTextures(GL_TEXTURE_CUBE_MAP, 1, &m_ID);
		glBindTexture(GL_TEXTURE_CUBE_MAP, m_ID);

		int Width, Height, Channels;

		for (uint32_t i = 0; i < cubeFaceFiles.size(); i++)
		{
			if (unsigned char* Data = stbi_load(cubeFaceFiles[i].c_str(), &Width, &Height, &Channels, 0))
			{
				m_Specification.InternalFormat = Channels == 4 ? ImageUtils::ImageInternalFormat::RGBA8 : ImageUtils::ImageInternalFormat::RGB8;
				m_Specification.DataLayout = Channels == 4 ? ImageUtils::ImageDataLayout::RGBA : ImageUtils::ImageDataLayout::RGB;

				const GLenum InternalFormat = ConvertInternalFormatMode(m_Specification.InternalFormat);
				const GLenum DataFormat = ConverDataLayoutMode(m_Specification.DataLayout);
				const GLenum DataType = ConvertImageDataType(m_Specification.DataType);
				m_Specification.Dimension = Height;

				const auto MipCount = GetMipLevelCount();
				glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, MipCount, InternalFormat, Width, Height, 0, DataFormat, DataType, Data);
				stbi_image_free(Data);
			}
			else
			{
				LOG_ERROR("Failed to load face: {} for Cube Map Texture from file '{}'.", i, cubeFaceFiles[i]);
				stbi_image_free(Data);
			}
		}
		const GLenum WrapModeS = ConvertWrapMode(m_Specification.SamplerWrapS);
		const GLenum WrapModeT = ConvertWrapMode(m_Specification.SamplerWrapT);
		const GLenum WrapModeR = ConvertWrapMode(m_Specification.SamplerWrapR);
		const GLenum MinFilter = ConvertMinMagFilterMode(m_Specification.MinFilter);
		const GLenum MagFilter = ConvertMinMagFilterMode(m_Specification.MagFilter);

		glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
		glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, WrapModeS);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, WrapModeT);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, WrapModeR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, MinFilter);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, MagFilter);
	}

	/**
	 * \brief A Cubemap.
	 * \param Specification The Cubemap Specification.
	 */
	TextureCube::TextureCube(const TextureCubeSpecification& Specification)
		:m_Specification(Specification), m_ID(0), m_Name(Specification.Name)
	{
		glCreateTextures(GL_TEXTURE_CUBE_MAP, 1, &m_ID);
		glBindTexture(GL_TEXTURE_CUBE_MAP, m_ID);

		const GLenum WrapModeS = ConvertWrapMode(m_Specification.SamplerWrapS);
		const GLenum WrapModeT = ConvertWrapMode(m_Specification.SamplerWrapT);
		const GLenum WrapModeR = ConvertWrapMode(m_Specification.SamplerWrapR);
		const GLenum MinFilter = ConvertMinMagFilterMode(m_Specification.MinFilter);
		const GLenum MagFilter = ConvertMinMagFilterMode(m_Specification.MagFilter);
		const GLenum InternalFormat = ConvertInternalFormatMode(m_Specification.InternalFormat);

		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, WrapModeS);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, WrapModeT);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, WrapModeR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, MinFilter);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, MagFilter);
		const auto MipCount = GetMipLevelCount();
		glTexStorage2D(GL_TEXTURE_CUBE_MAP, MipCount, InternalFormat, Specification.Dimension, Specification.Dimension);
	}

	void TextureCube::Invalidate(const TextureCubeSpecification& Specification)
	{
		LOG_TRACE("Invalidating Cubemap: ", m_Specification.Name);
		if(Specification.Name != m_Specification.Name)
			LOG_TRACE("\t New name for Invalidated Cubemap: {}", m_Specification.Name);
		m_Specification = Specification;
		glCreateTextures(GL_TEXTURE_CUBE_MAP, 1, &m_ID);
		glBindTexture(GL_TEXTURE_CUBE_MAP, m_ID);

		const GLenum WrapModeS = ConvertWrapMode(m_Specification.SamplerWrapS);
		const GLenum WrapModeT = ConvertWrapMode(m_Specification.SamplerWrapT);
		const GLenum WrapModeR = ConvertWrapMode(m_Specification.SamplerWrapR);
		const GLenum MinFilter = ConvertMinMagFilterMode(m_Specification.MinFilter);
		const GLenum MagFilter = ConvertMinMagFilterMode(m_Specification.MagFilter);
		const GLenum InternalFormat = ConvertInternalFormatMode(m_Specification.InternalFormat);

		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, WrapModeS);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, WrapModeT);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, WrapModeR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, MinFilter);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, MagFilter);
		const auto MipCount = GetMipLevelCount();
		glTexStorage2D(GL_TEXTURE_CUBE_MAP, MipCount, InternalFormat, Specification.Dimension, Specification.Dimension);
	}

	TextureCube::~TextureCube()
	{
		glDeleteTextures(1, &m_ID);
	}

	void TextureCube::BindToSamplerSlot(uint32_t slot) const
	{
		glActiveTexture(GL_TEXTURE0 + slot);
		glBindTextureUnit(slot, m_ID);
	}

	void TextureCube::Unbind()
	{
		glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
	}

	void TextureCube::BindToImageSlot(uint32_t Binding, uint32_t MipLevel, ImageUtils::TextureAccessLevel AccessLevel, ImageUtils::TextureShaderDataFormat ShaderDataFormat) const
	{
		const GLenum GLShaderDataFormat = ConvertShaderFormatType(ShaderDataFormat);
		const GLenum InternalFormat = ConvertInternalFormatMode(m_Specification.InternalFormat);

		if (GLShaderDataFormat != InternalFormat)
		{
			LOG_ERROR("Failure Binding '{}' TextureCube to Image Slot: Shader Data Format and Internal format must match!", m_Specification.Name);
			return;
		}

		glBindImageTexture(Binding, m_ID, MipLevel, GL_TRUE, 0, ConvertTextureAccessLevel(AccessLevel), ConvertShaderFormatType(ShaderDataFormat));
	}

	std::pair<glm::uint32_t, glm::uint32_t> TextureCube::GetMipSize(uint32_t Mip) const
	{
		uint32_t Width = m_Specification.Dimension;
		uint32_t Height = m_Specification.Dimension;
		while (Mip != 0)
		{
			Width /= 2;
			Height /= 2;
			Mip--;
		}

		return { Width, Height };
	}
	
	uint32_t TextureCube::GetMipLevelCount() const
	{
			return ImageUtils::CalculateMipLevelCount(m_Specification.Dimension, m_Specification.Dimension);
	}


	
	Texture2DImageView::Texture2DImageView(const Ref<Texture2D>& original, uint32_t baseMip, uint32_t mipCount, uint32_t baseLayer, uint32_t layerCount)
		:m_BaseMip(baseMip)
	{
		glGenTextures(1, &m_ID);
		const GLenum InternalFormat = ConvertInternalFormatMode(original->GetSpecification().InternalFormat);
		glTextureView(m_ID, GL_TEXTURE_2D, original->GetID(), InternalFormat, baseMip, mipCount, baseLayer, layerCount);
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
		m_BaseMip = baseMip;
		glGenTextures(1, &m_ID);
		const GLenum InternalFormat = ConvertInternalFormatMode(original->GetSpecification().InternalFormat);
		glTextureView(m_ID, GL_TEXTURE_2D, original->GetID(), InternalFormat, baseMip, mipCount, baseLayer, layerCount);
	}

	void Texture2DImageView::Bind() const
	{
		glBindTexture(GL_TEXTURE_2D, m_ID);
	}

	void Texture2DImageView::Unbind()
	{
		glBindTexture(GL_TEXTURE_2D, 0);
	}







	
	Texture3D::Texture3D(const Texture3DSpecification& Specification)
		:m_Name(Specification.Name)
	{
		glCreateTextures(GL_TEXTURE_3D, 1, &m_ID);
		glBindTexture(GL_TEXTURE_3D, m_ID);

		const GLenum WrapS = ImageUtils::ConvertWrapMode(Specification.WrapModeS);
		const GLenum WrapT = ImageUtils::ConvertWrapMode(Specification.WrapModeT);
		const GLenum WrapR = ImageUtils::ConvertWrapMode(Specification.WrapModeR);
		const GLenum MinFilter = ImageUtils::ConvertMinMagFilterMode(Specification.MinFilterMode);
		const GLenum MagFilter = ImageUtils::ConvertMinMagFilterMode(Specification.MagFilterMode);

		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, WrapS);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, WrapT);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, WrapR);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, MinFilter);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, MagFilter);

		// Immutable-format Texture
		// Contents of the image can be modified, but it's storage requirements may not change.
		// https://www.khronos.org/registry/OpenGL-Refpages/gl4/html/glTexStorage2D.xhtml

		const uint32_t Mips = GetMipLevelCount();
		glTextureStorage3D(m_ID, Mips, ConvertInternalFormatMode(Specification.InternalFormat), Specification.Width, Specification.Height, Specification.Depth);
	}

	Texture3D::~Texture3D()
	{
		glDeleteTextures(1, &m_ID);
	}

	void Texture3D::BindToSamplerSlot(uint32_t slot) const
	{
		glActiveTexture(GL_TEXTURE0 + slot);
		glBindTextureUnit(slot, m_ID);
	}

	void Texture3D::Unbind()
	{
		glBindTexture(GL_TEXTURE_3D, 0);
	}

	void Texture3D::BindToImageSlot(uint32_t unit, uint32_t level, ImageUtils::TextureAccessLevel access, ImageUtils::TextureShaderDataFormat shaderDataFormat) const
	{
		const GLenum GLShaderDataFormat = ConvertShaderFormatType(shaderDataFormat);
		const GLenum InternalFormat = ConvertInternalFormatMode(m_Specification.InternalFormat);

		if (GLShaderDataFormat != InternalFormat)
		{
			LOG_ERROR("Failure Binding '{}' Texture3D to Image Slot: Shader Data Format and Internal format must match!", m_Specification.Name);
		}

		glBindImageTexture(unit, m_ID, level, GL_TRUE, 0, ConvertTextureAccessLevel(access), ConvertShaderFormatType(shaderDataFormat));
	}

	int Texture3D::WriteToFile(const std::string& assetPath) const
	{
		const GLenum DataFormat = ConverDataLayoutMode(m_Specification.PixelLayoutFormat);
		const GLenum DataType = ConvertImageDataType(m_Specification.DataType);
		const GLsizei TextureDataSize = m_Specification.Width * m_Specification.Height * m_Specification.Depth * 16;
		void* pixels = malloc(TextureDataSize);
		glGetTextureSubImage(m_ID, 0, 0, 0, 0, m_Specification.Width, m_Specification.Height, m_Specification.Width, DataFormat, DataType, TextureDataSize, pixels);
		const int result = stbi_write_png(assetPath.c_str(), m_Specification.Width, m_Specification.Height, 4, pixels, 4);
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

	
	bool TextureLibrary::Has2D(const std::string& Name)
	{
		return s_NameToTexture2DLibrary.find(Name) != s_NameToTexture2DLibrary.end();
	}

	void TextureLibrary::AddTexture2D(const Ref<Texture2D>& texture)
	{
		if (Has2D(texture->GetName()))
		{
			LOG_TRACE("Texture2D with name '{}' already contained in Texture Library.", texture->GetName());
			return;
		}
		
		s_NameToTexture2DLibrary[texture->GetName()] = texture;
		s_IdToNameLibrary[texture->GetID()] = texture->GetName();
		LOG_TRACE("Added Texture2D with name: '{}' to the Texture Library.", texture->GetName());

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
		if (HasCube(texture->GetName()))
		{
			LOG_TRACE("TextureCube with name '{}' already contained in Texture Library.", texture->GetName());
			return;
		}
		
		s_NameToTextureCubeLibrary[texture->GetName()] = texture;
		s_IdToNameLibrary[texture->GetID()] = texture->GetName();
		LOG_TRACE("Added TextureCube with name: '{}' to the Texture Library.", texture->GetName());
	}

	Ref<TextureCube> TextureLibrary::LoadTextureCube(const TextureCubeSpecification& Spec, bool InvalidateIfExists)
	{
		if(HasCube(Spec.Name))
		{
			if(InvalidateIfExists)
				s_NameToTextureCubeLibrary[Spec.Name]->Invalidate(Spec);

			return s_NameToTextureCubeLibrary[Spec.Name];
		}
		Ref<TextureCube> texture = CreateRef<TextureCube>(Spec);
		AddTextureCube(texture);
		return texture;
	}

	void TextureLibrary::InvalidateCube(const TextureCubeSpecification& spec)
	{
		ASSERT(HasCube(spec.Name), "Unable to Invalidate TextureCube with name '{}' - does not exist", spec.Name);
		const Ref<TextureCube> TextureCube = s_NameToTextureCubeLibrary[spec.Name];
		TextureCube->Invalidate(spec);
	}

	Ref<Texture2D> TextureLibrary::Get2DFromID(uint32_t ID)
	{
		ASSERT(s_IdToNameLibrary.find(ID) != s_IdToNameLibrary.end(), "Unable to find Texture2D with ID: {}", ID);
		const auto TextureName = s_IdToNameLibrary[ID];
		ASSERT(s_NameToTexture2DLibrary.find(TextureName) != s_NameToTexture2DLibrary.end(), "Unable to find Texture2D with Name: {}", TextureName);
		return s_NameToTexture2DLibrary[TextureName];
	}

	Ref<TextureCube> TextureLibrary::GetCubeFromID(uint32_t ID)
	{
		ASSERT(s_IdToNameLibrary.find(ID) != s_IdToNameLibrary.end(), "Unable to find TextureCube with ID: {}", ID);
		const auto TextureName = s_IdToNameLibrary[ID];
		ASSERT(s_NameToTexture2DLibrary.find(TextureName) != s_NameToTexture2DLibrary.end(), "Unable to find TextureCube with Name: {}", TextureName);
		return s_NameToTextureCubeLibrary[TextureName];
	}

	Ref<Texture2D> TextureLibrary::LoadTexture2D(const std::string& filePath)
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
		ASSERT(Has2D(name), "No Texture2D with name '{}' found in Texture Library.", name)
		return s_NameToTexture2DLibrary.at(name);
	}

	const Ref<TextureCube>& TextureLibrary::GetCube(const std::string& name)
	{
		ASSERT(HasCube(name), "No TextureCube with name '{}' found in Texture Library.", name)
		return s_NameToTextureCubeLibrary.at(name);
	}

	void TextureLibrary::BindTexture2DToSlot(const std::string& TwoDimensionTextureName, uint32_t Slot)
	{
		ASSERT(Has2D(TwoDimensionTextureName), "TextureLibrary: Unable to bind Texture2D with name '{}' to slot '{}'.  This texture has not been registered.", TwoDimensionTextureName, Slot);
		const auto& Texture2D = s_NameToTexture2DLibrary[TwoDimensionTextureName];
		glBindTextureUnit(Slot, Texture2D->GetID());
	}

	void TextureLibrary::BindTextureCubeToSlot(const std::string& CubeTextureName, uint32_t Slot)
	{
		ASSERT(HasCube(CubeTextureName), "TextureLibrary: Unable to bind TextureCube with name '{}' to slot '{}'.  This texture has not been registered.", CubeTextureName, Slot);
		const auto& TextureCube = s_NameToTextureCubeLibrary[CubeTextureName];
		glBindTextureUnit(Slot, TextureCube->GetID());
	}

	void TextureLibrary::BindTextureToSlot(uint32_t TexID, uint32_t Slot)
	{
		glBindTextureUnit(Slot, TexID);
	}

	std::string TextureLibrary::GetNameFromID(uint32_t TextureID)
	{
		ASSERT(s_IdToNameLibrary.find(TextureID) != s_IdToNameLibrary.end(), "TextureLibrary: Unable to find texture with ID '{}'.", TextureID);
		return s_IdToNameLibrary[TextureID];
	}

	std::unordered_map<std::string, Ref<Texture2D>> TextureLibrary::s_NameToTexture2DLibrary;
	std::unordered_map<std::string, Ref<TextureCube>> TextureLibrary::s_NameToTextureCubeLibrary;
	std::unordered_map<uint32_t, std::string> TextureLibrary::s_IdToNameLibrary;
}

