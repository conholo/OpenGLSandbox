#include "Engine/Rendering/Shader.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <array>
#include <glad/glad.h>
#include <glm/gtc/type_ptr.hpp>

namespace Engine
{
	Shader::Shader(const std::string& filePath)
	{
		size_t shaderLocationOffset = filePath.rfind("\/") + 1;
		size_t extensionOffset = filePath.find_first_of(".", shaderLocationOffset);

		m_Name = filePath.substr(shaderLocationOffset, extensionOffset - shaderLocationOffset);

		std::string source = ReadFile(filePath);
		auto shaderSources = PreProcess(source);
		Compile(shaderSources);
	}

	Shader::~Shader()
	{
		glDeleteShader(m_ID);
	}

	void Shader::Bind() const
	{
		glUseProgram(m_ID);
	}

	void Shader::Unbind() const
	{
		glUseProgram(0);
	}

	std::string Shader::ReadFile(const std::string& filePath)
	{
		std::string result;
		std::ifstream input(filePath, std::ios::binary | std::ios::in);

		if (input)
		{
			input.seekg(0, std::ios::end);
			size_t size = input.tellg();

			if (size != -1)
			{
				result.resize(size);
				input.seekg(0, std::ios::beg);
				input.read(&result[0], size);
			}
		}

		return result;
	}

	GLenum ShaderTypeFromString(const std::string& type)
	{
		if (type == "vertex")
			return GL_VERTEX_SHADER;
		else if (type == "fragment")
			return GL_FRAGMENT_SHADER;
		else if (type == "compute")
			return GL_COMPUTE_SHADER;

		return GL_FALSE;
	}

	std::unordered_map<GLenum, std::string> Shader::PreProcess(const std::string& source)
	{
		const char* typeToken = "#type";
		size_t typeTokenLength = strlen(typeToken);
		size_t position = source.find(typeToken, 0);

		std::unordered_map<GLenum, std::string> result;

		while (position != std::string::npos)
		{
			size_t endOfLine = source.find_first_of("\r\n", position);
			size_t beginShaderType = position + typeTokenLength + 1;
			std::string type = source.substr(beginShaderType, endOfLine - beginShaderType);

			size_t nextLinePosition = source.find_first_not_of("\r\n", endOfLine);
			position = source.find(typeToken, nextLinePosition);
			result[ShaderTypeFromString(type)] =
				(position == std::string::npos)
				? source.substr(nextLinePosition)
				: source.substr(nextLinePosition, position - nextLinePosition);
		}

		return result;
	}

	void Shader::Compile(const std::unordered_map<GLenum, std::string>& shaderSources)
	{
		GLuint program = glCreateProgram();
		std::vector<GLuint> shaderIDs;
		int glShaderIDIndex = 0;

		for (auto& kv : shaderSources)
		{
			GLenum type = kv.first;
			m_IsCompute = type == GL_COMPUTE_SHADER;
			const std::string& source = kv.second;

			GLuint shader = glCreateShader(type);

			const GLchar* sourceCStr = source.c_str();
			glShaderSource(shader, 1, &sourceCStr, 0);

			glCompileShader(shader);

			GLint isCompiled = 0;
			glGetShaderiv(shader, GL_COMPILE_STATUS, &isCompiled);
			if (isCompiled == GL_FALSE)
			{
				GLint maxLength = 0;
				glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &maxLength);

				std::vector<GLchar> infoLog(maxLength);
				glGetShaderInfoLog(shader, maxLength, &maxLength, &infoLog[0]);

				std::stringstream ss;

				std::string type;

				if (shader == GL_FRAGMENT_SHADER)
					type = "FRAGMENT COMPILATION ERROR";
				else if (shader == GL_VERTEX_SHADER)
					type = "VERTEX COMPILATION ERROR: ";
				else if (shader == GL_COMPUTE_SHADER)
					type = "COMPUTE COMPILATION ERROR: ";

				ss << m_Name << ":\n" << type << infoLog.data();

				std::cout << ss.str() << "\n";

				glDeleteShader(shader);

				break;
			}

			glAttachShader(program, shader);
			shaderIDs.push_back(shader);
		}

		m_ID = program;

		glLinkProgram(program);

		GLint isLinked = 0;
		glGetProgramiv(program, GL_LINK_STATUS, (int*)&isLinked);
		if (isLinked == GL_FALSE)
		{
			GLint maxLength = 0;
			glGetProgramiv(program, GL_INFO_LOG_LENGTH, &maxLength);

			std::vector<GLchar> infoLog(maxLength);
			glGetProgramInfoLog(program, maxLength, &maxLength, &infoLog[0]);

			std::stringstream ss;

			std::string messageHeader = m_Name + ": LINKING ERROR: ";

			ss << messageHeader << infoLog.data();

			std::cout << ss.str() << "\n";

			glDeleteProgram(program);

			for (auto id : shaderIDs)
				glDeleteShader(id);

			return;
		}

		for (auto id : shaderIDs)
		{
			glDetachShader(program, id);
			glDeleteShader(id);
		}
	}

	GLint Shader::UploadUniformFloat(const std::string& name, float value)
	{
		GLint location = glGetUniformLocation(m_ID, name.c_str());
		glUniform1f(location, value);
		return location;
	}

	GLint Shader::UploadUniformFloat2(const std::string& name, const glm::vec2& value)
	{
		GLint location = glGetUniformLocation(m_ID, name.c_str());
		glUniform2f(location, value.x, value.y);
		return location;
	}

	GLint Shader::UploadUniformFloat3(const std::string& name, const glm::vec3& value)
	{
		GLint location = glGetUniformLocation(m_ID, name.c_str());
		glUniform3f(location, value.x, value.y, value.z);
		return location;
	}

	GLint Shader::UploadUniformFloat4(const std::string& name, const glm::vec4& value)
	{
		GLint location = glGetUniformLocation(m_ID, name.c_str());
		glUniform4f(location, value.x, value.y, value.z, value.w);
		return location;
	}

	GLint Shader::UploadUniformInt(const std::string& name, int value)
	{
		GLint location = glGetUniformLocation(m_ID, name.c_str());
		glUniform1i(location, value);
		return location;
	}

	GLint Shader::UploadUniformIntArray(const std::string& name, uint32_t count, int* basePtr)
	{
		GLint location = glGetUniformLocation(m_ID, name.c_str());
		glUniform1iv(location, count, basePtr);
		return location;
	}

	GLint Shader::UploadUniformMat3(const std::string& name, const glm::mat3& matrix)
	{
		GLint location = glGetUniformLocation(m_ID, name.c_str());
		glUniformMatrix3fv(location, 1, GL_FALSE, glm::value_ptr(matrix));
		return location;
	}

	GLint Shader::UploadUniformMat4(const std::string& name, const glm::mat4& matrix)
	{
		GLint location = glGetUniformLocation(m_ID, name.c_str());
		glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(matrix));
		return location;
	}

	GLint Shader::UploadUniformMat4Array(const std::string& name, uint32_t count, float* start)
	{
		GLint location = glGetUniformLocation(m_ID, name.c_str());
		glUniformMatrix4fv(location, count, GL_FALSE, start);
		return location;
	}

	void Shader::EnableShaderImageAccessBarrierBit()
	{
		glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT | GL_TEXTURE_FETCH_BARRIER_BIT);
	}

	void Shader::EnableTextureFetchBarrierBit()
	{
		glMemoryBarrier(GL_TEXTURE_FETCH_BARRIER_BIT);
	}

	void Shader::DispatchCompute(uint32_t groupX, uint32_t groupY, uint32_t groupZ)
	{
		if (!m_IsCompute)
		{
			std::cout << m_Name << " is not of type Compute Shader.  Cannot dispatch." << std::endl;
			return;
		}

		glUseProgram(m_ID);
		glDispatchCompute(groupX, groupY, groupZ);
	}

	std::unordered_map<std::string, Ref<Shader>> ShaderLibrary::s_ShaderLibrary;

	void ShaderLibrary::Add(const Ref<Shader>& shader)
	{
		if (s_ShaderLibrary.find(shader->GetName()) == s_ShaderLibrary.end())
		{
			s_ShaderLibrary[shader->GetName()] = shader;
		}
		else
		{
			std::cout << "Shader already contained in Shader Library.  Attempted to add (" << shader->GetName() << ") Shader to Library." << "\n";
		}
	}

	void ShaderLibrary::Load(const std::string& filePath)
	{
		Ref<Shader> shader = CreateRef<Shader>(filePath);
		Add(shader);
	}

	const Ref<Shader>& ShaderLibrary::Get(const std::string& name)
	{
		if (s_ShaderLibrary.find(name) == s_ShaderLibrary.end())
		{
			std::cout << "No shader with name:( " << name << ") found in Shader Library." << "\n";
			return nullptr;
		}

		return s_ShaderLibrary.at(name);
	}
}

