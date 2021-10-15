#include "Engine/Rendering/Camera.h"
#include "Engine/Event/WindowEvent.h"
#include "Engine/Core/Input.h"
#include "Engine/Core/Utility.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

#define PI 3.14159265359

namespace Engine
{
	Camera::Camera(float fov, float aspectRatio, float nearClip, float farClip)
		:m_FOV(fov), m_AspectRatio(aspectRatio), m_NearClip(nearClip), m_FarClip(farClip)
	{
		m_Position = CalculatePosition();

		glm::quat orientation = CalculateOrientation();
		m_WorldRotationEulers = glm::degrees(glm::eulerAngles(orientation));

		m_ViewMatrix = glm::translate(glm::mat4(1.0f), m_Position) * glm::toMat4(orientation);
		m_ViewMatrix = glm::inverse(m_ViewMatrix);

		RecalculateProjection();
	}

	glm::vec3 Camera::CalculatePosition() const
	{
		return m_FocalPoint - Forward() * m_DistanceFromFocalPoint + m_PositionDelta;
	}

	glm::quat Camera::CalculateOrientation() const
	{
		return glm::quat(glm::vec3(-m_Pitch, -m_Yaw, 0.0f));
	}

	void Camera::RecalculateView()
	{
		const float yawSign = Up().y < 0.0f ? -1.0f : 1.0f;

		const glm::vec3 lookAt = m_Position + Forward();
		m_WorldRotationEulers = glm::normalize(m_FocalPoint - m_Position);
		m_FocalPoint = m_Position + Forward() * m_DistanceFromFocalPoint;
		m_DistanceFromFocalPoint = glm::distance(m_Position, m_FocalPoint);
		m_ViewMatrix = glm::lookAt(m_Position, lookAt, glm::vec3{ 0.0f, yawSign, 0.0f });

		m_YawDelta *= 0.6f;
		m_PitchDelta *= 0.6f;
		m_PositionDelta *= 0.8f;
	}

	void Camera::RecalculateProjection()
	{
		m_ProjectionMatrix = glm::perspective(glm::radians(m_FOV), m_AspectRatio, m_NearClip, m_FarClip);
	}

	void Camera::Update(float deltaTime)
	{
		if (m_IsLocked) return;

		const glm::vec2 mousePosition = Input::GetMousePosition();
		const glm::vec2 mouseDelta = (mousePosition - m_CurrentMousePosition) * 0.002f;

		if (Input::IsMouseButtonPressed(2) || Input::IsMouseButtonPressed(1))
		{
			if (Input::IsKeyPressed(Key::LeftAlt))
			{
				// Reset Rotation
				m_Yaw = 0.0f;
				m_Pitch = 0.0f;
			}
			else
			{
				// Fly forward/back
				if (Input::IsKeyPressed(Key::W))
					m_PositionDelta += Forward() * m_PanSpeed * deltaTime;
				if (Input::IsKeyPressed(Key::S))
					m_PositionDelta -= Forward() * m_PanSpeed * deltaTime;

				// Pitch/Yaw adjustment from mouse pan
				const float yawSign = Up().y < 0.0f ? -1.0f : 1.0f;
				m_YawDelta += yawSign * mouseDelta.x * m_RotationSpeed * deltaTime;
				m_PitchDelta += mouseDelta.y * m_RotationSpeed * deltaTime;
			}
		}
		else
		{
			// Fly up/down
			if (Input::IsKeyPressed(Key::W))
				m_PositionDelta += Up() * m_PanSpeed * deltaTime;
			if (Input::IsKeyPressed(Key::S))
				m_PositionDelta -= Up() * m_PanSpeed * deltaTime;
		}

		// Fly left/right
		if (Input::IsKeyPressed(Key::A))
			m_PositionDelta -= Right() * m_PanSpeed * deltaTime;
		if (Input::IsKeyPressed(Key::D))
			m_PositionDelta += Right() * m_PanSpeed * deltaTime;


		m_Position += m_PositionDelta;
		m_Yaw += m_YawDelta;
		m_Pitch += m_PitchDelta;

		m_CurrentMousePosition = mousePosition;
		RecalculateView();
	}

	void Camera::LockLookAtAndPosition(const glm::vec3& position, const glm::vec3& lookAt)
	{
		m_IsLocked = true;
		m_Position = position;

		m_PositionDelta = glm::vec3(0.0f);
		m_YawDelta = 0.0f;
		m_PitchDelta = 0.0f;
		m_Yaw = 0.0f;
		m_Pitch = 0.0f;

		float yawSign = Up().y < 0.0 ? -1.0f : 1.0f;

		m_ViewMatrix = glm::lookAt(position, lookAt, { 0.0f, yawSign, 0.0f });
	}

	void Camera::ToggleIsLocked(bool isLocked)
	{
		m_IsLocked = isLocked;
	}

	void Camera::OnEvent(Event& event)
	{
		EventDispatcher dispatcher(event);

		if (m_IsLocked) return;

		dispatcher.Dispatch<MouseScrolledEvent>(BIND_FN(Camera::OnScroll));
	}

	bool Camera::OnScroll(MouseScrolledEvent& event)
	{
		if (m_IsLocked) return false;

		m_DistanceFromFocalPoint -= event.GetYOffset();
		m_Position = m_FocalPoint - Forward() * m_DistanceFromFocalPoint;

		if (m_DistanceFromFocalPoint > 1.0f)
		{
			m_FocalPoint += Forward();
			m_DistanceFromFocalPoint = 1.0f;
		}

		m_PositionDelta += event.GetYOffset() * Forward();

		RecalculateView();
		return true;
	}

	void Camera::SetViewportSize(float width, float height)
	{
		m_AspectRatio = width / height;
		RecalculateProjection();
	}

	void Camera::Orbit(const glm::vec3& eyePosition, const glm::vec3& target, const glm::vec3& angleAxisDegrees)
	{
		m_IsLocked = true;

		glm::quat quaternion = glm::quat(glm::radians(angleAxisDegrees));

		glm::vec3 worldPosition = quaternion * (eyePosition - target) + target;

		float yawSign = angleAxisDegrees.y < 0.0 ? -1.0f : 1.0f;

		m_ViewMatrix = glm::lookAt(worldPosition, target, glm::vec3{ 0.0f, yawSign, 0.0f });
	}

	void Camera::SetRotation(const glm::vec2& rotation)
	{
		m_Pitch = glm::radians(rotation.x);
		m_Yaw = glm::radians(rotation.y);
		RecalculateView();
	}

	glm::vec3 Camera::Forward() const
	{
		return glm::rotate(CalculateOrientation(), glm::vec3(0.0f, 0.0f, -1.0f));
	}

	glm::vec3 Camera::Up() const
	{
		return glm::rotate(CalculateOrientation(), glm::vec3(0.0f, 1.0f, 0.0f));
	}

	glm::vec3 Camera::Right() const
	{
		return glm::rotate(CalculateOrientation(), glm::vec3(1.0f, 0.0f, 0.0f));
	}
}