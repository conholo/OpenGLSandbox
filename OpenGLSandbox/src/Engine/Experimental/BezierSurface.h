#pragma once

#include "Engine/Rendering/Line.h"
#include "Engine/Rendering/VertexArray.h"
#include "Engine/Rendering/VertexBuffer.h"
#include "Engine/Rendering/Mesh.h"
#include "Engine/Core/MouseCodes.h"
#include "Engine/Rendering/Camera.h"
#include "Engine/Rendering/Texture.h"

#include <glm/glm.hpp>

namespace Engine
{
	struct SurfaceVertex
	{
		glm::vec3 Position;
		glm::vec2 TexCoord;
		glm::vec3 Normal;
	};

	class BezierSurface
	{
	public:
		BezierSurface();
		~BezierSurface();

		SurfaceVertex CalculateBezierSurface(const std::vector<Ref<BezierCurve>>& curves, float u, float v);
		glm::vec3 DUBezier(const std::vector<Ref<BezierCurve>>& curves, float u, float v);
		glm::vec3 DVBezier(const std::vector<Ref<BezierCurve>>& curves, float u, float v);

		const Ref<Shader>& GetShader() const { return m_PointShader; }

		void Draw(const glm::mat4& viewProjection);
		void StartPicking(MouseCode mouseCode, Camera& camera);
		void StopPicking(MouseCode mouseCode);
		void UpdateDragCurve(Camera& camera);

		glm::vec3 GetPosition() const { return m_Transform->GetPosition(); }
		glm::vec3 GetScale() const { return m_Transform->GetScale(); }
		glm::vec3 GetRotation() const { return m_Transform->GetRotation(); }

		void SetPosition(const glm::vec3& position);
		void SetScale(const glm::vec3& scale);
		void SetRotation(const glm::vec3& rotation);

		bool IsAnimating() const { return m_Animate; }
		bool IsDrawCurves() const { return m_DrawCurves; }
		bool IsWireFrame() const { return m_WireFrame; }

		void AnimateControls();
		void ToggleDrawCurves();
		void ToggleWireFrame();
		void ToggleAnimation();

		void ResetCurves();

	private:
		void UpdateSurfaceCurves();
	private:
		bool m_Animate = false;
		bool m_DrawCurves = false;
		bool m_WireFrame = false;
		Ref<BezierCurve> m_DragCurve;
		bool m_IsDragging = false;
		uint32_t m_DragID = 0;
		glm::vec3 m_DefaultPosition{ 7.0f, 10.0f, 0.0f };

	private:
		uint32_t m_DivisionCount = 16;
		uint32_t m_PatchCount = 4;
		std::vector<std::vector<uint32_t>> m_PatchIndices;
		std::vector<std::vector<SurfaceVertex>> m_PatchVertices;
		std::vector<std::vector<uint32_t>> m_PatchIndexBuffer;

		Ref<Texture2D> m_Texture;

	private:
		std::vector<glm::vec3> m_SurfacePoints;
		std::vector<glm::vec3> m_Vertices;

		std::vector<std::vector<Ref<BezierCurve>>> m_PatchCurves;
		std::vector<std::vector<std::vector<glm::vec3>>> m_PatchCurvesDefaultPositions;

		std::vector<glm::vec3> m_FinalVertices;
		Ref<VertexArray> m_VAO;
		Ref<Shader> m_PointShader;
		Ref<VertexBuffer> m_PointVBO;
		Ref<IndexBuffer> m_PointEBO;
		Ref<EntityTransform> m_Transform;
		Ref<Mesh> m_TestMesh;
	};
}