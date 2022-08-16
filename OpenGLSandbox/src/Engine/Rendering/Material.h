#pragma once
#include <iostream>
#include "Shader.h"
#include "Engine/Core/Buffer.h"
#include "Engine/Core/Memory.h"

namespace Engine
{
	class Texture2D;
	enum class RenderFlag;

	const std::string EngineShaderName = "EnginePBR";
	struct TextureUniform
	{
		uint32_t RendererID = 0;
		int32_t TextureUnit = -1;
	};

	struct MaterialUniformData
	{
		std::unordered_map<std::string, float> FloatUniforms;
		std::unordered_map<std::string, int> IntUniforms;
		std::unordered_map<std::string, TextureUniform> TextureUniforms;
		std::unordered_map<std::string, glm::vec2> Vec2Uniforms;
		std::unordered_map<std::string, glm::vec3> Vec3Uniforms;
		std::unordered_map<std::string, glm::vec4> Vec4Uniforms;
	};

	class Material
	{
	public:
		Material(std::string name, const Ref<Shader>& shader);
		Ref<Material> Clone(const std::string& cloneName);

		Ref<Shader> GetShader() const { return m_Shader; }
		const std::string& GetName() const { return m_Name; }

		void UploadStagedUniforms();
		void BindSamplerTexturesToRenderContext();
		void LogActiveTextures();

		template<typename T>
		void Set(const std::string& name, const T& data)
		{
			const auto* uniform = FindShaderUniform(name);

			if (uniform == nullptr)
			{
				std::cout << "Unable to Set Uniform with name: " << name << " for material: " << m_Name << "\n";				
				return;
			}

			m_UniformStorageBuffer.Write<T>((uint8_t*)&data, uniform->GetSize(), uniform->GetBufferOffset());
		}

		template<typename T>
		T* Get(const std::string& name)
		{
			const auto* uniform = FindShaderUniform(name);

			if (uniform == nullptr)
				return nullptr;
		
			return m_UniformStorageBuffer.Read<T>(uniform->GetBufferOffset());
		}

		bool Has(const std::string& name)
		{
			const auto* uniform = FindShaderUniform(name);

			return uniform != nullptr;
		}

		const ShaderUniform* FindShaderUniform(const std::string& name);

		uint32_t GetFlags() const { return m_RenderFlags; }
		bool GetFlag(RenderFlag flag) const { return (uint32_t)flag & m_RenderFlags; }
		void SetFlag(RenderFlag flag, bool value = true)
		{
			m_RenderFlags = value ? m_RenderFlags | (uint32_t)flag : m_RenderFlags & ~(uint32_t)flag;
		}

		MaterialUniformData GetMaterialUniformData();
		void Bind() const { m_Shader->Bind(); }
		void Unbind() const { m_Shader->Unbind(); }

	private:
		void AllocateStorageBuffer();
		void InitializeStorageBufferWithUniformDefaults();

		Ref<Shader> m_Shader;
		Buffer m_UniformStorageBuffer;
		uint32_t m_RenderFlags;
		std::string m_Name;

		friend class SimpleEntity;
	};
}

