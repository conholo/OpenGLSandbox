#pragma once

#include "Engine/Event/Event.h"
#include "Engine/Core/Time.h"
#include "Engine/Event/MouseEvent.h"

#include <glm/glm.hpp>

namespace Engine
{
	class Camera
	{
	public:

		Camera(float fov, float aspectRatio, float nearClip, float farClip);

		void Update(float deltaTime);
		void OnEvent(Event& event);

		void LockLookAtAndPosition(const glm::vec3& position, const glm::vec3& lookAt);
		void ToggleIsLocked(bool isLocked);

		void SetViewportSize(float width, float height);

		float GetZDistance() const { return m_DistanceFromFocalPoint; }
		void SetZDistance(float zDistance) { m_DistanceFromFocalPoint = zDistance; }

		void Orbit(const glm::vec3& eyePosition, const glm::vec3& target, const glm::vec3& angleAxisDegrees);


		void SetPosition(const glm::vec3& position) { m_Position = position; RecalculateView(); }
		void SetRotation(const glm::vec2& rotation);

		glm::mat4 GetViewProjection() const { return m_ProjectionMatrix * m_ViewMatrix; }
		glm::mat4 GetView() const { return m_ViewMatrix; }
		glm::mat4 GetProjection() const { return m_ProjectionMatrix; }

		glm::vec3 Forward() const;
		glm::vec3 Up() const;
		glm::vec3 Right() const;
		glm::vec3 GetPosition() const { return m_Position; }

	private:
		glm::quat CalculateOrientation() const;
		glm::vec3 CalculatePosition() const;
		void RecalculateView();
		void RecalculateProjection();

		bool OnScroll(MouseScrolledEvent& event);

	private:

		bool m_IsLocked = false;

		float m_PanSpeed = 3.0f;
		float m_RotationSpeed = 50.0f;

		float m_FOV = 45.0f;
		float m_AspectRatio;

		float m_NearClip = 0.1f;
		float m_FarClip = 1000.0f;

		float m_Pitch = 0.0f, m_Yaw = 0.0f;
		float m_PitchDelta = 0.0f, m_YawDelta = 0.0f;

		float m_DistanceFromFocalPoint = 10.0f;
		float m_Rotation;

		glm::vec3 m_WorldRotationEulers{ 0.0f };
		glm::vec3 m_Position{ 0.0f };

		glm::vec3 m_PositionDelta{ 0.0f };
		glm::vec3 m_FocalPoint{ 0.0f };

		glm::vec2 m_CurrentMousePosition{ 0.0f };

		glm::mat4 m_ViewMatrix{ 1.0f };
		glm::mat4 m_ProjectionMatrix{ 1.0f };
	};
}