#include "Engine/Rendering/Line.h"
#include "Engine/Rendering/RenderCommand.h"

#include <iostream>

namespace Engine
{
	Line::Line(const std::string& shaderName)
		:m_Shader(ShaderLibrary::Get(shaderName))
	{
		Initialize();
	}

	Line::Line(const std::vector<LineVertex>& vertices, const std::string& shaderName)
		: m_Vertices(vertices), m_Shader(ShaderLibrary::Get(shaderName))
	{
		Initialize();
	}

	Line::Line(std::initializer_list<LineVertex>& vertices, const std::string& shaderName)
		: m_Vertices(vertices), m_Shader(ShaderLibrary::Get(shaderName))
	{
		Initialize();
	}

	void Line::Initialize()
	{
		m_Transform = CreateRef<EntityTransform>();
		m_VAO = CreateRef<VertexArray>();

		m_VBO = CreateRef<VertexBuffer>(m_Vertices.size() * sizeof(LineVertex));

		BufferLayout layout
		{
			{ "a_Position", ShaderAttributeType::Float3 }
		};

		m_VBO->SetLayout(layout);

		m_VAO->EnableVertexAttributes(m_VBO);
	}

	Line::~Line()
	{

	}

	void Line::Draw(const glm::mat4& viewProjection)
	{
		m_VAO->Bind();
		float* baseVertexPtr = &m_Vertices.data()->Position.x;
		m_VBO->ResizeAndSetData(baseVertexPtr, m_Vertices.size() * sizeof(LineVertex));
		m_Shader->Bind();
		m_Shader->UploadUniformMat4("u_MVP", viewProjection * m_Transform->Transform());
		RenderCommand::DrawLine(LineTopology::LineStrip, m_Vertices.size());
		m_Shader->Unbind();
		m_VAO->Unbind();
	}

	void Line::Clear()
	{
		m_Vertices.clear();
	}



	BezierCurve::BezierCurve(const std::string& lineShaderName, const std::string& pointShaderName, const glm::vec3& center)
		:m_LineShader(ShaderLibrary::Get(lineShaderName)), m_PointShader(ShaderLibrary::Get(pointShaderName))
	{
		AddSegment({ center });
		Initialize();
	}

	BezierCurve::BezierCurve(const glm::vec3& center)
		:m_LineShader(ShaderLibrary::Get("LineShader")), m_PointShader(ShaderLibrary::Get("LinePointShader"))
	{
		AddSegment({ center });
		Initialize();
	}

	BezierCurve::~BezierCurve()
	{

	}

	/*							(4)
	    (1)                    /	\
	   /					  /		 \
	  /						 /		  \
	(0)-------(x)----------(3)--------(5)
						   /			\
						  /				 \
						 /				  \
						(2)			      (6)
	*/

	void BezierCurve::AddSegment(const LineVertex& anchor)
	{
		if (m_Vertices.size() <= 0)
		{
			// Left Anchor
			m_Vertices.push_back({ anchor.Position + glm::vec3(-1.0f, 0.0f, 0.0f) });
			// Left Control
			m_Vertices.push_back({ anchor.Position + glm::vec3(-1.0f, 1.0f, 0.0f) * 0.5f });
			// Right Control
			m_Vertices.push_back({ anchor.Position + glm::vec3(1.0f, -1.0f, 0.0f) * 0.5f });
			// Right Anchor
			m_Vertices.push_back({ anchor.Position + glm::vec3(1.0f, 0.0f, 0.0f) });
		}
		else
		{
			if (m_Looped) return;

			glm::vec3 controlOne = m_Vertices[m_Vertices.size() - 1].Position * 2.0f - m_Vertices[m_Vertices.size() - 2].Position;
			m_Vertices.push_back({ controlOne });
			glm::vec3 controlTwo = (m_Vertices[m_Vertices.size() - 1].Position + anchor.Position) * 0.5f;
			m_Vertices.push_back({ controlTwo });
			m_Vertices.push_back({ anchor });
		}
	}

	void BezierCurve::ToggleLooped()
	{
		if (GetSegmentCount() <= 1) return;

		m_Looped = !m_Looped;

		if (!m_Looped)
		{
			glm::vec3 halfWayFromAnchorToLastControl = (m_Vertices[0].Position + m_Vertices[m_Vertices.size() - 2].Position) / 2.0f;
			m_Vertices[m_Vertices.size() - 1].Position = halfWayFromAnchorToLastControl;
			return;
		}

		m_Vertices[m_Vertices.size() - 1] = m_Vertices[0];
		MovePoint(1, m_Vertices[1].Position);
	}

	void BezierCurve::MovePoint(uint32_t index, const glm::vec3& newPosition)
	{
		glm::vec3 movementDirection = glm::normalize(newPosition - m_Vertices[index].Position);
		float movementDistance = glm::length(newPosition - m_Vertices[index].Position);

		if (movementDistance == 0) return;

		if (m_Looped && (index == 0 || index == m_Vertices.size() - 1))
		{
			m_Vertices[0].Position = newPosition;
			m_Vertices[m_Vertices.size() - 1].Position = newPosition;
		}

		m_Vertices[index].Position = newPosition;

		if (index % 3 == 0)
		{
			if ((m_Looped && index == 0) || (m_Looped && index == m_Vertices.size() - 1))
			{
				glm::vec3 firstControlAfterAnchor = m_Vertices[1].Position;
				glm::vec3 lastControlBeforeAnchor = m_Vertices[m_Vertices.size() - 2].Position;

				m_Vertices[1].Position = firstControlAfterAnchor + movementDirection * movementDistance;
				m_Vertices[m_Vertices.size() - 2].Position = lastControlBeforeAnchor + movementDirection * movementDistance;
			}
			else
			{
				if (index == 0 || index == m_Vertices.size() - 1) return;

				glm::vec3 firstControlAfterAnchor = m_Vertices[index + 1].Position;
				glm::vec3 lastControlBeforeAnchor = m_Vertices[index - 1].Position;

				m_Vertices[index + 1].Position = firstControlAfterAnchor + movementDirection * movementDistance;
				m_Vertices[index - 1].Position = lastControlBeforeAnchor + movementDirection * movementDistance;
			}
		}
		else
		{
			if (index % 3 == 1)
			{
				if (m_Looped && index == 1)
				{
					glm::vec3 thisControl = m_Vertices[index].Position;
					glm::vec3 anchor = m_Vertices[index - 1].Position;

					glm::vec3 loopedCtrl = m_Vertices[m_Vertices.size() - 2].Position;
					float loopedCtrlToAnchorLength = glm::length(anchor - loopedCtrl);
					glm::vec3 directionToAnchor = glm::normalize(anchor - thisControl);
					glm::vec3 updatedLoopedCtrlPosition = anchor + directionToAnchor * loopedCtrlToAnchorLength;

					m_Vertices[m_Vertices.size() - 2].Position = updatedLoopedCtrlPosition;
				}
				else if ((int)index - 2 >= 0)
				{
					glm::vec3 thisControl = m_Vertices[index].Position;
					glm::vec3 anchor = m_Vertices[index - 1].Position;

					glm::vec3 previousSegmentCtrl = m_Vertices[index - 2].Position;
					float prevCtrlToAnchorLength = glm::length(anchor - previousSegmentCtrl);
					glm::vec3 directionToAnchor = glm::normalize(anchor - thisControl);
					glm::vec3 updatedPrevSegPoint = anchor + directionToAnchor * prevCtrlToAnchorLength;

					m_Vertices[index - 2].Position = updatedPrevSegPoint;
				}
			}
			else if (index % 3 == 2)
			{
				if (m_Looped && index == m_Vertices.size() - 2)
				{
					glm::vec3 thisControl = m_Vertices[index].Position;
					glm::vec3 anchor = m_Vertices[m_Vertices.size() - 1].Position;

					glm::vec3 loopedCtrl = m_Vertices[1].Position;
					float loopedCtrlToAnchorLength = glm::length(anchor - loopedCtrl);
					glm::vec3 directionToAnchor = glm::normalize(anchor - thisControl);
					glm::vec3 updatedLoopedCtrlPosition = anchor + directionToAnchor * loopedCtrlToAnchorLength;

					m_Vertices[1].Position = updatedLoopedCtrlPosition;
				}
				else if (index + 2 <= m_Vertices.size() - 1)
				{
					glm::vec3 thisControl = m_Vertices[index].Position;
					glm::vec3 anchor = m_Vertices[index + 1].Position;

					glm::vec3 nextSegmentCtrl = m_Vertices[index + 2].Position;

					float nextCtrlToAnchorLength = glm::length(anchor - nextSegmentCtrl);
					glm::vec3 directionToAnchor = glm::normalize(anchor - thisControl);
					glm::vec3 updatedNextSegPoint = anchor + directionToAnchor * nextCtrlToAnchorLength;

					m_Vertices[index + 2].Position = updatedNextSegPoint;
				}
			}
		}
	}

	void BezierCurve::SetVertices(const std::vector<LineVertex>& vertices)
	{
		m_Vertices.clear();
		m_Vertices = vertices;
	}

	std::vector<LineVertex> BezierCurve::GetPointsInSegment(uint32_t index)
	{
		return std::vector<LineVertex>
		{
				m_Vertices[index * 3],
				m_Vertices[index * 3 + 1],
				m_Vertices[index * 3 + 2],
				m_Vertices[index * 3 + 3],
		};
	}

	void BezierCurve::Initialize()
	{
		m_Transform = CreateRef<EntityTransform>();
		m_VAO = CreateRef<VertexArray>();

		m_DebugVBO = CreateRef<VertexBuffer>(m_Vertices.size() * sizeof(LineVertex));
		m_CurveVBO = CreateRef<VertexBuffer>(0);

		BufferLayout layout
		{
			{ "a_Position", ShaderAttributeType::Float3 }
		};


		m_DebugVBO->SetLayout(layout);
		m_CurveVBO->SetLayout(layout);

		m_VAO->EnableVertexAttributes(m_DebugVBO);
	}

	void BezierCurve::SetPointSize(float size)
	{
		RenderCommand::SetPointSize(size);
	}

	static glm::vec3 CalculateCubicBezier(const glm::vec3& p0, const glm::vec3& p1, const glm::vec3& p2, const glm::vec3& p3, float t)
	{
		float inv = 1.0 - t;
		float invSquared = inv * inv;
		float invCubed = invSquared * inv;
		float tSquared = t * t;
		float tCubed = tSquared * t;

		glm::vec3 p0_ = invCubed * p0;
		glm::vec3 p1_ = 3.0f * invSquared * t * p1;
		glm::vec3 p2_ = 3.0f * inv * tSquared * p2;
		glm::vec3 p3_ = tCubed * p3;

		return p0_ + p1_ + p2_ + p3_;
	}


	void BezierCurve::Draw(const glm::mat4& viewProjection)
	{
		if (m_Vertices.size() <= 0) return;

		m_VAO->Bind();
		float* baseVertexPtr = &m_Vertices.data()->Position.x;
		m_DebugVBO->ResizeAndSetData(baseVertexPtr, m_Vertices.size() * sizeof(LineVertex));

		{
			m_LineShader->Bind();
			m_LineShader->UploadUniformMat4("u_MVP", viewProjection * m_Transform->Transform());

			// Every two points, draw a line, skipping the line from point 2 to point 3 per segment (that will be the curve).
			for (uint32_t i = 0; i < GetSegmentCount(); i++)
			{
				if (m_Debug)
				{
					m_VAO->EnableVertexAttributes(m_DebugVBO);
					m_LineShader->UploadUniformFloat3("u_Color", m_ControlLineColor);
					RenderCommand::DrawLine(LineTopology::Lines, 2, i * 3 + 0);
					RenderCommand::DrawLine(LineTopology::Lines, 2, i * 3 + 2);
				}

				m_LineShader->UploadUniformFloat3("u_Color", m_CurveColor);
				{
					std::vector<LineVertex> segmentVertices = GetPointsInSegment(i);
					std::vector<LineVertex> curveVertices;
					curveVertices.resize(m_Resolution + 1);

					for (uint32_t s = 0; s <= m_Resolution; s++)
					{
						float t = (float)s / (float)m_Resolution;
						glm::vec3 curvePosition = CalculateCubicBezier(segmentVertices[0].Position, segmentVertices[1].Position, segmentVertices[2].Position, segmentVertices[3].Position, t);
						curveVertices[s] = { curvePosition };
					}

					float* curveBaseVertexPtr = &curveVertices.data()->Position.x;
					m_CurveVBO->ResizeAndSetData(curveBaseVertexPtr, sizeof(LineVertex) * (m_Resolution + 1));
					m_VAO->EnableVertexAttributes(m_CurveVBO);
					RenderCommand::DrawLine(LineTopology::LineStrip, m_Resolution + 1);
				}
			}

			m_LineShader->Unbind();
		}

		{
			if (m_Debug)
			{
				m_VAO->EnableVertexAttributes(m_DebugVBO);
				m_PointShader->Bind();
				m_LineShader->UploadUniformFloat3("u_Color", m_ControlPointColor);
				m_PointShader->UploadUniformMat4("u_MVP", viewProjection * m_Transform->Transform());
				RenderCommand::DrawPoints(m_Vertices.size());
				m_PointShader->Unbind();
			}
		}

		m_VAO->Unbind();
	}
}