#pragma once

#include "Engine/Rendering/TextureUtils.h"
#include "Engine/Core/Memory.h"

#include <string>
#include <stdint.h>
#include <vector>
#include <string>
#include <glm/glm.hpp>

namespace Engine
{
	struct TextureSpecification
	{
		ImageUtils::Usage Usage = ImageUtils::Usage::Texture;
		ImageUtils::WrapMode SamplerWrap = ImageUtils::WrapMode::Repeat;
		ImageUtils::FilterMode Filter = ImageUtils::FilterMode::Linear;
		ImageUtils::ImageInternalFormat InternalFormat = ImageUtils::ImageInternalFormat::RGBA8;
		ImageUtils::ImageDataLayout DataLayout = ImageUtils::ImageDataLayout::RGBA;
		ImageUtils::ImageDataType DataType = ImageUtils::ImageDataType::UByte;
		uint32_t Width, Height;
	};

	struct Texture2DSpecification
	{
		ImageUtils::WrapMode WrapModeS;
		ImageUtils::WrapMode WrapModeT;
		ImageUtils::FilterMode MinFilterMode;
		ImageUtils::FilterMode MagFilterMode;
		ImageUtils::ImageInternalFormat InternalFormat;
		ImageUtils::ImageDataLayout PixelLayoutFormat;
		ImageUtils::ImageDataType DataType;
		uint32_t Width, Height;
	};

	class Texture2DArray
	{
	public:
		Texture2DArray(const std::vector<std::string>& filePaths, const Texture2DSpecification& specification);
		~Texture2DArray();

		uint32_t GetID() const { return m_ID; }
		const Texture2DSpecification& GetSpecification() const { return m_Specification; }

		std::pair<uint32_t, uint32_t> GetMipSize(uint32_t mip) const;
		uint32_t GetMipLevelCount() const;
		uint32_t GetWidth() const { return m_Specification.Width; }
		uint32_t GetHeight() const { return m_Specification.Height; }

		void BindToSamplerSlot(uint32_t slot);
		void Unbind() const;
		void BindToImageSlot(uint32_t unit, uint32_t level, ImageUtils::TextureAccessLevel access, ImageUtils::TextureShaderDataFormat shaderDataFormat);

	private:
		Texture2DSpecification m_Specification;
		uint32_t m_ID;
	};

	class Texture2D
	{
	public:
		Texture2D(const Texture2DSpecification& specification);
		Texture2D(const std::string& filePath, const Texture2DSpecification& specification);
		~Texture2D();

		void Invalidate();
		void Clear();
		void Resize(uint32_t width, uint32_t height);

		uint32_t GetID() const { return m_ID; }
		const Texture2DSpecification& GetSpecification() const { return m_Specification; }

		std::pair<uint32_t, uint32_t> GetMipSize(uint32_t mip) const;
		uint32_t GetMipLevelCount() const;
		uint32_t GetWidth() const { return m_Specification.Width; }
		uint32_t GetHeight() const { return m_Specification.Height; }

		static void BindTextureIDToSamplerSlot(uint32_t slot, uint32_t id);

		void BindToSamplerSlot(uint32_t slot);
		void Unbind() const;
		void BindToImageSlot(uint32_t unit, uint32_t level, ImageUtils::TextureAccessLevel access, ImageUtils::TextureShaderDataFormat shaderDataFormat);
		void SetData(void* data, uint32_t size);

		static Ref<Texture2D> CreateWhiteTexture();
		static Ref<Texture2D> CreateBlackTexture();

	private:
		Texture2DSpecification m_Specification;
		uint32_t m_ID;
	};

	class Texture2DImageView
	{
	public:
		Texture2DImageView(const Ref<Texture2D>& original, uint32_t baseMip, uint32_t mipCount, uint32_t baseLayer = 1, uint32_t layerCount = 0);
		~Texture2DImageView();


		void ChangeToMip(const Ref<Texture2D>& original, uint32_t baseMip, uint32_t mipCount, uint32_t baseLayer = 1, uint32_t layerCount = 0);
		void Bind() const;
		void Unbind() const;

		uint32_t GetBaseMip() const { return m_BaseMip; }
		uint32_t GetID() const { return m_ID; }

	private:
		uint32_t m_BaseMip = 0;
		uint32_t m_ID;
	};

	class Texture3D
	{
	public:
		Texture3D(const TextureSpecification& spec);
		~Texture3D();

		void BindToSamplerSlot(uint32_t slot = 0);
		void Unbind() const;
		void BindToImageSlot(uint32_t unit, uint32_t level, ImageUtils::TextureAccessLevel access, ImageUtils::TextureShaderDataFormat shaderDataFormat);

		uint32_t GetID() const { return m_ID; }
		uint32_t GetWidth() const { return m_Specification.Width; }
		uint32_t GetHeight() const { return m_Specification.Height; }

		int WriteToFile(const std::string& assetPath);

		const TextureSpecification& GetSpecification() const { return m_Specification; }

		std::pair<uint32_t, uint32_t> GetMipSize(uint32_t mip) const;
		uint32_t GetMipLevelCount() const;

	private:
		TextureSpecification m_Specification;
		uint32_t m_ID;
	};

	
	class TextureCube
	{
	public:
		TextureCube(const TextureSpecification& spec, const std::vector<std::string>& cubeFaceFiles);
		TextureCube(const TextureSpecification& spec, const void* data);
		~TextureCube();

		void BindToSamplerSlot(uint32_t slot = 0);
		void Unbind() const;
		void BindToImageSlot(uint32_t unit, uint32_t level, ImageUtils::TextureAccessLevel access, ImageUtils::TextureShaderDataFormat shaderDataFormat);

		uint32_t GetID() const { return m_ID; }
		uint32_t GetWidth() const { return m_Specification.Width; }
		uint32_t GetHeight() const { return m_Specification.Height; }

	private:
		TextureSpecification m_Specification;
		uint32_t m_ID;
	};
}

