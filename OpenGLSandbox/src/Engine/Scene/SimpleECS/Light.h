#pragma once

#include "Engine/Core/Memory.h"
#include "Engine/Scene/SimpleECS/EntityTransform.h"
#include "Engine/Scene/SimpleECS/EntityRenderer.h"
#include "Engine/Rendering/Texture.h"
#include <glm/glm.hpp>

namespace Engine
{
	enum class LightType { None = 0, Directional, Point, Spot };

	struct LightSpecification
	{
		LightType Type				= LightType::None;
		glm::vec3 LightColor		= glm::vec3(1.0f);
		float Intensity				= 1.0f;

		float ConstantAttenuation	= 0.0f;
		float LinearAttenuation		= 0.0f;
		float QuadraticAttenuation	= 0.0f;
		
		glm::vec3 LightDirection	= glm::vec3(0.0f, 0.0f, -1.0f);
		float InnerCutOff			= 45.0f;
		float OuterCutOff			= 90.0f;
		PrimitiveType DebugShape	= PrimitiveType::Sphere;
	};

	class Light
	{
	public:
		Light(const LightSpecification& specification);

		const LightSpecification& GetSpecification() const { return m_Specification; }
		void SetSpecification(const LightSpecification& spec) { m_Specification = spec; }
		const glm::vec3& GetViewSpaceVector(const glm::mat4& viewMatrix) const;

		void SetLightColor(const glm::vec3& color) { m_Specification.LightColor = color; }
		void SetIntensity(float intensity) { m_Specification.Intensity = intensity; }
		void SetConstantAttenuation(float constantAttenuation) { m_Specification.ConstantAttenuation = constantAttenuation; }
		void SetLinearAttenuation(float linearAttenuation) { m_Specification.LinearAttenuation = linearAttenuation; }
		void SetQuadraticAttenuation(float quadraticAttenuation) { m_Specification.QuadraticAttenuation = quadraticAttenuation; }

		void SetLightDirection(const glm::vec3& direction) { m_Specification.LightDirection = direction; }
		void SetInnerCutOff(float innerCutoff) { m_Specification.InnerCutOff = innerCutoff; }
		void SetOuterCutOff(float outerCutoff) { m_Specification.OuterCutOff = outerCutoff; }

		const glm::vec3& GetLightColor() const { return m_Specification.LightColor; }
		float GetLightIntensity() const { return m_Specification.Intensity; }
		float GetConstantAttenuation() const { return m_Specification.ConstantAttenuation; }
		float GetLinearAttenuation() const { return m_Specification.LinearAttenuation; }
		float GetQuadraticAttenuation() const { return m_Specification.QuadraticAttenuation; }

		const glm::vec3& GetLightDirection() const { return m_Specification.LightDirection; }
		float GetInnerCutOff() const { return m_Specification.InnerCutOff; }
		float GetOuterCutOff() const { return m_Specification.OuterCutOff; }

		const Ref<EntityTransform> GetLightTransform() const { return m_Transform; }
		const Ref<EntityRenderer> GetDebugRenderer() const { return m_DebugRenderer; }

		void DrawDebug(const glm::mat4& viewProjection);

	private:
		LightSpecification m_Specification;
		Ref<EntityTransform> m_Transform;
		Ref<EntityRenderer> m_DebugRenderer;
		Ref<Texture2D> m_WhiteTexture;
	};
}