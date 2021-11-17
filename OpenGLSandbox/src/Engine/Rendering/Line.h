#pragma once

#include "Engine/Core/Memory.h"
#include "Engine/Rendering/Shader.h"
#include "Engine/Rendering/VertexArray.h"
#include "Engine/Rendering/VertexBuffer.h"
#include "Engine/Rendering/IndexBuffer.h"
#include "Engine/Scene/SimpleECS/EntityTransform.h"

#include <vector>
#include <string>
#include <glm/glm.hpp>

namespace Engine
{
	struct LineVertex
	{
		glm::vec3 Position;
	};

	class Line
	{
	public:
		Line(const std::string& shaderName);
		Line(const std::vector<LineVertex>& vertices, const std::string& shaderName);
		Line(std::initializer_list<LineVertex>& vertices, const std::string& shaderName);
		virtual ~Line();

		void AddPoint(const LineVertex& point) { m_Vertices.push_back(point); }
		void Clear();
		void Draw(const glm::mat4& viewProjection);

		const Ref<EntityTransform>& GetEntityTransform() const { return m_Transform; }
		const Ref<Shader>& GetShader() const { return m_Shader; }

	private:
		void Initialize();

	private:
		std::vector<LineVertex> m_Vertices;
		Ref<Shader> m_Shader;
		Ref<VertexArray> m_VAO;
		Ref<VertexBuffer> m_VBO;
		Ref<EntityTransform> m_Transform;
	};

	class BezierCurve
	{
	public:
		BezierCurve(const glm::vec3& center = glm::vec3(0.0f));
		BezierCurve(const std::string& lineShaderName, const std::string& pointShaderName, const glm::vec3& center = glm::vec3(0.0f));
		~BezierCurve();

		void SetPointSize(float size);
		void AddSegment(const LineVertex& anchor);
		void MovePoint(uint32_t index, const glm::vec3& newPosition);
		void Draw(const glm::mat4& viewProjection);

		void ToggleDebug() { m_Debug = !m_Debug; }
		void SetDebug(bool enabled) { m_Debug = enabled; }
		void ToggleLooped();
		void SetPointColor(const glm::vec3& pointColor) { m_ControlPointColor = pointColor; }
		void SetControlLineColor(const glm::vec3& controlLineColor) { m_ControlLineColor = controlLineColor; }
		void SetCurveColor(const glm::vec3& curveColor) { m_CurveColor = curveColor; }
		void Clear() { m_Vertices.clear(); m_Looped = false; }

		bool GetIsLooped() const { return m_Looped; }
		bool GetIsDisplayingDebug() const { return m_Debug; }
		glm::vec3& GetCurveColor() { return m_CurveColor; }
		glm::vec3& GetPointColor() { return m_ControlPointColor; }
		const Ref<EntityTransform>& GetTransform() const { return m_Transform; }

		void SetVertices(const std::vector<LineVertex>& vertices);

		uint32_t GetPointCount() const { return m_Vertices.size(); }
		uint32_t GetSegmentCount() const { return (m_Vertices.size() - 4) / 3 + 1; }
		std::vector<LineVertex> GetPointsInSegment(uint32_t index);

		std::vector<LineVertex>& GetVertices() { return m_Vertices; }
		const std::vector<LineVertex>& GetVertices() const { return m_Vertices; }

		LineVertex operator[](uint32_t index) { return m_Vertices[index]; }
		const LineVertex& operator[](uint32_t index) const { return m_Vertices[index]; }

		std::vector<LineVertex>::iterator begin() { return m_Vertices.begin(); }
		std::vector<LineVertex>::iterator end() { return m_Vertices.end(); }
		const std::vector<LineVertex>::const_iterator begin() const { return m_Vertices.begin(); }
		const std::vector<LineVertex>::const_iterator end() const { return m_Vertices.end(); }

	private:
		void Initialize();

	private:
		uint32_t m_Resolution = 100;
		bool m_Debug = false;
		bool m_Looped = false;
		glm::vec3 m_ControlPointColor = glm::vec3(1.0, 0.0, 0.0);
		glm::vec3 m_ControlLineColor = glm::vec3(1.0, 1.0, 1.0);
		glm::vec3 m_CurveColor = glm::vec3(0.0, 1.0, 0.0);

	private:
		std::vector<LineVertex> m_Vertices;
		Ref<Shader> m_LineShader;
		Ref<Shader> m_PointShader;
		Ref<VertexArray> m_VAO;
		Ref<VertexBuffer> m_DebugVBO;
		Ref<VertexBuffer> m_CurveVBO;
		Ref<EntityTransform> m_Transform;
	};
}