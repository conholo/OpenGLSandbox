#include "Material.h"
#include "Engine/Rendering/RenderCommand.h"
#include "Engine/Rendering/Texture.h"

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <utility>

namespace Engine
{
	Material::Material(std::string name, const Ref<Shader>& shader)
		: m_Shader(shader), m_Name(std::move(name))
	{
		AllocateStorageBuffer();
		InitializeStorageBufferWithUniformDefaults();

		SetFlag(RenderFlag::DepthTest);
		SetFlag(RenderFlag::Blend);
	}

	Ref<Material> Material::Clone(const std::string& cloneName)
	{
		Ref<Material> copy = CreateRef<Material>(cloneName, m_Shader);
		return copy;
	}

	void Material::BindSamplerTexturesToRenderContext()
	{
		const std::vector<ShaderUniform> Sampler2DUniforms = m_Shader->GetBaseBlockUniformsOfType(ShaderAttributeType::Sampler2D);
		for(const ShaderUniform& Uniform : Sampler2DUniforms)
		{
			const auto* TexUniform = Get<TextureUniform>(Uniform.GetName());
			TextureLibrary::BindTextureToSlot(TexUniform->RendererID, TexUniform->TextureUnit);
		}

		const std::vector<ShaderUniform> SamplerCubeUniforms = m_Shader->GetBaseBlockUniformsOfType(ShaderAttributeType::SamplerCube);
		for(const ShaderUniform& Uniform : SamplerCubeUniforms)
		{
			const auto* TexUniform = Get<TextureUniform>(Uniform.GetName());
			TextureLibrary::BindTextureToSlot(TexUniform->RendererID, TexUniform->TextureUnit);
		}
	}

	void Material::LogActiveTextures()
	{
		const std::vector<ShaderUniform> Sample2DUniforms = m_Shader->GetBaseBlockUniformsOfType(ShaderAttributeType::Sampler2D);
		for(const ShaderUniform& Uniform : Sample2DUniforms)
		{
			const auto* TexUniform = Get<TextureUniform>(Uniform.GetName());
			std::cout << Uniform.GetName() << " -> " << TextureLibrary::GetNameFromID(TexUniform->RendererID) << "\n";
		}

		const std::vector<ShaderUniform> SampleCubeUniforms = m_Shader->GetBaseBlockUniformsOfType(ShaderAttributeType::SamplerCube);
		for(const ShaderUniform& Uniform : SampleCubeUniforms)
		{
			const auto* TexUniform = Get<TextureUniform>(Uniform.GetName());
			std::cout << Uniform.GetName() << " -> " << TextureLibrary::GetNameFromID(TexUniform->RendererID) << "\n";
		}
	}
	
	MaterialUniformData Material::GetMaterialUniformData()
	{
		MaterialUniformData data;

		for (auto& [name, uniform] : m_Shader->GetUniforms())
		{
			std::string uniformName = uniform.GetName();

			switch (uniform.GetType())
			{
			case ShaderAttributeType::Float:
				{
					float* value = Get<float>(uniformName);
					data.FloatUniforms[uniformName] = *value;
					break;
				}
			case ShaderAttributeType::Float2:
				{
					glm::vec2* value = Get<glm::vec2>(uniformName);
					data.Vec2Uniforms[uniformName] = *value;
					break;
				}
			case ShaderAttributeType::Float3:
				{
					glm::vec3* value = Get<glm::vec3>(uniformName);
					data.Vec3Uniforms[uniformName] = *value;
					break;
				}
			case ShaderAttributeType::Float4:
				{
					glm::vec4* value = Get<glm::vec4>(uniformName);
					data.Vec4Uniforms[uniformName] = *value;
					break;
				}
			case ShaderAttributeType::Sampler2D:
				{
					TextureUniform* value = Get<TextureUniform>(uniformName);
					data.TextureUniforms[uniformName] = *value;
					break;
				}
			case ShaderAttributeType::SamplerCube:
				{
					TextureUniform* value = Get<TextureUniform>(uniformName);
					data.TextureUniforms[uniformName] = *value;
					break;
				}
			case ShaderAttributeType::Int:
				{
					int* value = Get<int>(uniformName);
					data.IntUniforms[uniformName] = *value;
					break;
				}
			case ShaderAttributeType::Mat3:
				{
					glm::mat3* value = Get<glm::mat3>(uniformName);
					break;
				}
			case ShaderAttributeType::Mat4:
				{
					glm::mat4* value = Get<glm::mat4>(uniformName);
					break;
				}
			}
		}

		return data;
	}

	void Material::AllocateStorageBuffer()
	{
		const auto& uniforms = m_Shader->GetUniforms();

		if (uniforms.size() <= 0) return;

		uint32_t bufferSize = 0;

		for (auto& [name, uniform] : uniforms)
			bufferSize += uniform.GetSize();

		m_UniformStorageBuffer.Allocate(bufferSize);
		m_UniformStorageBuffer.ZeroInitialize();
	}

	void Material::InitializeStorageBufferWithUniformDefaults()
	{
		const auto& uniforms = m_Shader->GetUniforms();

		if (uniforms.size() <= 0) return;

		for (auto& [name, uniform] : uniforms)
		{
			switch (uniform.GetType())
			{
			case ShaderAttributeType::Float:
			case ShaderAttributeType::Float2:
			case ShaderAttributeType::Float3:
			case ShaderAttributeType::Float4:
				{
					GLfloat* data = (GLfloat*)m_Shader->GetUniformData(uniform.GetType(), uniform.GetLocation());
					m_UniformStorageBuffer.Write<float>(data, uniform.GetSize(), uniform.GetBufferOffset());
					break;
				}
			case ShaderAttributeType::Int:
				{
					GLint* data = (GLint*)m_Shader->GetUniformData(uniform.GetType(), uniform.GetLocation());
					m_UniformStorageBuffer.Write<int>(data, uniform.GetSize(), uniform.GetBufferOffset());
					break;
				}
			case ShaderAttributeType::Mat3:
			case ShaderAttributeType::Mat4:
				{
					void* data = malloc(ShaderDataTypeSize(uniform.GetType()));
					m_UniformStorageBuffer.Write<glm::mat4>(data, ShaderDataTypeSize(uniform.GetType()), uniform.GetBufferOffset());
					break;
				}
			case ShaderAttributeType::SamplerCube:
				{
					TextureUniform* data = (TextureUniform*)m_Shader->GetUniformData(uniform.GetType(), uniform.GetLocation());
					// Change this to default black cube.
					data->RendererID = TextureLibrary::Get2D("White Texture")->GetID();
					m_UniformStorageBuffer.Write<TextureUniform>(data, uniform.GetSize(), uniform.GetBufferOffset());
					break;
				}
			case ShaderAttributeType::Sampler2D:
				{
					TextureUniform* data = (TextureUniform*)m_Shader->GetUniformData(uniform.GetType(), uniform.GetLocation());
					data->RendererID = TextureLibrary::Get2D("White Texture")->GetID();
					m_UniformStorageBuffer.Write<TextureUniform>(data, uniform.GetSize(), uniform.GetBufferOffset());
					break;
				}
			default:
			case ShaderAttributeType::None: break;
			}
		}
	}


	const ShaderUniform* Material::FindShaderUniform(const std::string& name)
	{
		const auto& uniforms = m_Shader->GetUniforms();

		if (uniforms.size() <= 0) return nullptr;

		if (uniforms.find(name) == uniforms.end())
			return nullptr;

		return &uniforms.at(name);
	}

	void Material::UploadStagedUniforms()
	{
		BindSamplerTexturesToRenderContext();
		m_Shader->Bind();
		for (auto& [name, uniform] : m_Shader->GetUniforms())
		{
			std::string uniformName = uniform.GetName();

			switch (uniform.GetType())
			{
			case ShaderAttributeType::Float:
				{
					float* value = Get<float>(uniformName);
					m_Shader->UploadUniformFloat(uniformName, *value);
					break;
				}
			case ShaderAttributeType::Float2:
				{
					glm::vec2* value = Get<glm::vec2>(uniformName);
					m_Shader->UploadUniformFloat2(uniformName, *value);
					break;
				}
			case ShaderAttributeType::Float3:
				{
					glm::vec3* value = Get<glm::vec3>(uniformName);
					m_Shader->UploadUniformFloat3(uniformName, *value);
					break;
				}
			case ShaderAttributeType::Float4:
				{
					glm::vec4* value = Get<glm::vec4>(uniformName);
					m_Shader->UploadUniformFloat4(uniformName, *value);
					break;
				}
			case ShaderAttributeType::SamplerCube:
			case ShaderAttributeType::Sampler2D:
				{
					TextureUniform* value = Get<TextureUniform>(uniformName);
					m_Shader->UploadUniformInt(uniformName, (*value).TextureUnit);
					break;
				}
			case ShaderAttributeType::Int:
				{
					int* value = Get<int>(uniformName);
					m_Shader->UploadUniformInt(uniformName, *value);
					break;
				}
			case ShaderAttributeType::Mat3:
				{
					glm::mat3* value = Get<glm::mat3>(uniformName);
					m_Shader->UploadUniformMat3(uniformName, *value);
					break;
				}
			case ShaderAttributeType::Mat4:
				{
					glm::mat4* value = Get<glm::mat4>(uniformName);
					m_Shader->UploadUniformMat4(uniformName, *value);
					break;
				}
			}
		}
	}
}
