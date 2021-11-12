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

	class Texture2D
	{
	public:
		Texture2D(const Texture2DSpecification& specification);
		Texture2D(const std::string& filePath, const Texture2DSpecification& specification);
		~Texture2D();

		void Invalidate();
		void Resize(uint32_t width, uint32_t height);

		const Texture2DSpecification& GetSpecification() const { return m_Specification; }

		glm::vec2 GetMipCount(uint32_t mip) const;
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
		uint32_t CalculateMipLevelCount(uint32_t width, uint32_t height) const;
	private:
		Texture2DSpecification m_Specification;
		uint32_t m_ID;
	};

	
	class Texture3D
	{
	public:
		Texture3D(const TextureSpecification& spec, const std::vector<std::string>& cubeFaceFiles);
		Texture3D(const TextureSpecification& spec, const void* data);
		~Texture3D();

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

