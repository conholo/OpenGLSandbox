#pragma once

#include "Engine/Rendering/Mesh.h"
#include <string>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>

namespace Engine
{
	struct TagComponent
	{
		std::string Tag;

		TagComponent() = default;
		TagComponent(const TagComponent&) = default;
		TagComponent(const std::string& tag)
			: Tag(tag)
		{
		}
	};

	struct TransformComponent
	{
		TransformComponent() = default;
		TransformComponent(const TransformComponent&) = default;
		TransformComponent(const glm::vec3& translation, const glm::vec3& rotation, const glm::vec3& scale)
			:Translation(translation), Rotation(rotation), Scale(scale) { }


		glm::mat4 Transform() const
		{
			return 
				glm::translate(glm::mat4(1.0), Translation) * 
				glm::toMat4(glm::quat(glm::radians(Rotation))) * 
				glm::scale(glm::mat4(1.0), Scale);
		}

		glm::quat Orientation()
		{
			return glm::quat(glm::radians(glm::vec3(Rotation.x, Rotation.y, 0.0f)));
		}

		glm::vec3 Up()
		{
			return glm::rotate(Orientation(), glm::vec3(0.0f, 1.0f, 0.0f));
		}

		glm::vec3 Right()
		{
			return glm::rotate(Orientation(), glm::vec3(1.0f, 0.0f, 0.0f));
		}

		glm::vec3 Forward()
		{
			return glm::rotate(Orientation(), glm::vec3(0.0f, 0.0f, -1.0f));
		}

		glm::vec3 Translation = glm::vec3(0.0f);
		glm::vec3 Rotation = glm::vec3(0.0f);
		glm::vec3 Scale = glm::vec3(1.0f);
	};

	struct RendererMaterialProperties
	{
		glm::vec3 AmbientColor = glm::vec3(1.0f, 1.0f, 1.0f);
		glm::vec3 DiffuseColor = glm::vec3(1.0f, 1.0f, 1.0f);
		float AmbientStrength = 1.0f;
		float DiffuseStrength = 1.0f;
		float SpecularStrength = 1.0f;
		float Shininess = 1.0f;
	};

	struct MeshRendererComponent
	{
		MeshRendererComponent() = default;
		MeshRendererComponent(const MeshRendererComponent&) = default;
		MeshRendererComponent(PrimitiveType type, const RendererMaterialProperties& properties)
			:Type(type), Properties(properties) { }

		PrimitiveType Type;
		RendererMaterialProperties Properties;
	};

	struct CameraComponent
	{
		glm::mat4 View;
		glm::mat4 Projection;

		CameraComponent() = default;
		CameraComponent(const CameraComponent&) = default;
		CameraComponent(const glm::mat4& view, const glm::mat4& projection)
			:View(view), Projection(projection)
		{
		}
	};

	enum class EngineLightType { Sun, Directional, Point, Spot };

	struct LightComponent
	{
		EngineLightType Type;
		glm::vec4 Color;
		float Intensity = 1.0f;
		bool DebugLight = true;

		LightComponent() = default;
		LightComponent(const LightComponent&) = default;
		LightComponent(EngineLightType type, const glm::vec4& color = glm::vec4(1.0f), float intensity = 1.0, bool debug = false)
			:Type(type), Color(color), DebugLight(debug), Intensity(intensity)
		{
		}
	};
}