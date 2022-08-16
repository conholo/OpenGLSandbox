#include "Engine/Rendering/BezierSurface.h"
#include "Engine/Rendering/RenderCommand.h"
#include "Engine/Core/Input.h"
#include "Engine/Core/Time.h"
#include <iostream>
#define PI 3.14159265358979

namespace Engine
{
	BezierSurface::BezierSurface()
	{
		m_Transform = CreateRef<EntityTransform>();
		m_VAO = CreateRef<VertexArray>();
		m_PointShader = ShaderLibrary::Get("BlinnPhongSimple");
		m_PointVBO = CreateRef<VertexBuffer>(0);
		BufferLayout layout
		{
			{ "a_Position", ShaderAttributeType::Float3 },
			{ "a_TexCoord", ShaderAttributeType::Float2 },
			{ "a_Normal",  ShaderAttributeType::Float3}
		};

		Texture2DSpecification spec =
		{
			ImageUtils::WrapMode::Repeat,
			ImageUtils::WrapMode::Repeat,
			ImageUtils::FilterMode::Linear,
			ImageUtils::FilterMode::Linear,
			ImageUtils::ImageInternalFormat::FromImage,
			ImageUtils::ImageDataLayout::FromImage,
			ImageUtils::ImageDataType::UByte,
		};


		m_Texture = CreateRef<Texture2D>("assets/textures/japan.png", spec);

		m_PointVBO->SetLayout(layout);
		m_VAO->EnableVertexAttributes(m_PointVBO);

		m_TestMesh = MeshFactory::TessellatedQuad(4);

		for (uint32_t i = 0; i < m_TestMesh->GetVertices().size(); i++)
			m_Vertices.push_back(m_TestMesh->GetVertices()[i].Position);
		
		m_PatchCount = m_Vertices.size() / m_DivisionCount;

		m_PatchVertices.resize(m_PatchCount);
		m_PatchIndexBuffer.resize(m_PatchCount);

		for (uint32_t i = 0; i < m_PatchCount; i++)
		{
			m_PatchVertices[i].resize((m_DivisionCount + 1) * (m_DivisionCount + 1));
			m_PatchIndexBuffer[i].resize(m_DivisionCount * m_DivisionCount * 4);
		}

		// 2D Array to keep track of vertex indices for each patch.
		m_PatchIndices.resize(m_PatchCount);
		for (uint32_t i = 0; i < m_PatchIndices.size(); i++)
			m_PatchIndices[i].resize(m_DivisionCount);


		for (uint32_t i = 0; i < m_PatchCount; i++)
			for (uint32_t j = 0; j < m_DivisionCount; j++)
				m_PatchIndices[i][j] = i * m_DivisionCount + j + 1;

		std::vector<glm::vec3> controls;
		m_PatchCurves.resize(m_PatchCount);
		m_PatchCurvesDefaultPositions.resize(m_PatchCount);
		for (uint32_t patchIndex = 0; patchIndex < m_PatchCount; patchIndex++)
		{
			// Always 4 controllable curves per patch.
			m_PatchCurves[patchIndex].resize(4);
			m_PatchCurvesDefaultPositions[patchIndex].resize(4);

			// Always 16 controls per patch (4 per curve).
			controls.resize(16);

			// Set the vertices of the controls for this patch.
			for (uint32_t controlIndex = 0; controlIndex < 16; controlIndex++)
			{
				uint32_t vertexIndex = m_PatchIndices[patchIndex][controlIndex] - 1;
				controls[controlIndex] = m_Vertices[vertexIndex];
			}

			// Supply the curves for this patch with their controls and store the curves for this patch.
			for (uint32_t i = 0; i < 4; i++)
			{
				std::vector<LineVertex> curveControls;
				curveControls.resize(4);
				curveControls[0] = { controls[i * 4 + 0] };
				curveControls[1] = { controls[i * 4 + 1] };
				curveControls[2] = { controls[i * 4 + 2] };
				curveControls[3] = { controls[i * 4 + 3] };

				for(uint32_t curvePosition = 0; curvePosition < 4; curvePosition++)
					m_PatchCurvesDefaultPositions[patchIndex][i].push_back(curveControls[curvePosition].Position);

				Ref<BezierCurve> curve = CreateRef<BezierCurve>();

				curve->SetVertices(curveControls);
				curve->ToggleDebug();
				m_PatchCurves[patchIndex][i] = curve;
			}

			for (uint32_t x = 0, k = 0; x < m_DivisionCount; x++)
			{
				for (uint32_t y = 0; y < m_DivisionCount; y++, k++)
				{
					m_PatchIndexBuffer[patchIndex][k * 4] = (m_DivisionCount + 1) * x + y;
					m_PatchIndexBuffer[patchIndex][k * 4 + 1] = (m_DivisionCount + 1) * x + y + 1;
					m_PatchIndexBuffer[patchIndex][k * 4 + 2] = (m_DivisionCount + 1) * (x + 1) + y + 1;
					m_PatchIndexBuffer[patchIndex][k * 4 + 3] = (m_DivisionCount + 1) * (x + 1) + y;
				}
			}
		}
			
		uint32_t* indexPtr = &m_PatchIndexBuffer[0][0];
		m_PointEBO = CreateRef<IndexBuffer>(indexPtr, m_DivisionCount * m_DivisionCount * 4);
		m_VAO->SetIndexBuffer(m_PointEBO);
	}

	BezierSurface::~BezierSurface()
	{
	}



	SurfaceVertex BezierSurface::CalculateBezierSurface(const std::vector<Ref<BezierCurve>>& curves, float u, float v)
	{
		std::vector<LineVertex> vControls;
		vControls.resize(4);

		for (uint32_t i = 0; i < 4; i++)
		{
			Ref<BezierCurve> curve = curves[i];
			std::vector<LineVertex> uCurvePoints = curve->GetVertices();
			vControls[i] = { BezierCurve::CalculateCubicBezier(uCurvePoints[0].Position, uCurvePoints[1].Position, uCurvePoints[2].Position, uCurvePoints[3].Position, u) };
		}

		glm::vec3 position = BezierCurve::CalculateCubicBezier(vControls[0].Position, vControls[1].Position, vControls[2].Position, vControls[3].Position, v);
		glm::vec2 texCoord = { u, v };

		glm::vec3 du = DUBezier(curves, u, v);
		glm::vec3 dv = DVBezier(curves, u, v);
		glm::vec3 normal = glm::normalize(glm::cross(du, dv));

		return { position, texCoord, normal };
	}

	glm::vec3 BezierSurface::DUBezier(const std::vector<Ref<BezierCurve>>& curves, float u, float v)
	{
		std::vector<LineVertex> vCurve;
		vCurve.resize(4);

		std::vector<glm::vec3> points;
		points.resize(4);
		
		for (uint32_t i = 0; i < 4; i++)
		{
			Ref<BezierCurve> curve = curves[i];
			points[0] = curves[0]->GetVertices()[i].Position;
			points[1] = curves[1]->GetVertices()[i].Position;
			points[2] = curves[2]->GetVertices()[i].Position;
			points[3] = curves[3]->GetVertices()[i].Position;

			std::vector<LineVertex> uCurvePoints = curve->GetVertices();
			vCurve[i] = { BezierCurve::CalculateCubicBezier(points[0], points[1], points[2], points[3], v) };
		}

		return -3.0f * (1.0f - u) * (1.0f - u) * vCurve[0].Position +
			(3.0f * (1.0f - u) * (1.0f - u) - 6.0f * u * (1.0f - u)) * vCurve[1].Position +
			(6.0f * u * (1.0f - u) - 3.0f * u * u) * vCurve[2].Position +
			3.0f * u * u * vCurve[3].Position;
	}

	glm::vec3 BezierSurface::DVBezier(const std::vector<Ref<BezierCurve>>& curves, float u, float v)
	{
		std::vector<LineVertex> uCurve;
		uCurve.resize(4);

		for (uint32_t i = 0; i < 4; i++)
		{
			Ref<BezierCurve> curve = curves[i];
			std::vector<LineVertex> uCurvePoints = curve->GetVertices();
			uCurve[i] = { BezierCurve::CalculateCubicBezier(uCurvePoints[0].Position, uCurvePoints[1].Position, uCurvePoints[2].Position, uCurvePoints[3].Position, u) };
		}

		return -3.0f * (1.0f - v) * (1.0f - v) * uCurve[0].Position +
			(3.0f * (1.0f - v) * (1.0f - v) - 6.0f * v * (1.0f - v)) * uCurve[1].Position +
			(6.0f * v * (1.0f - v) - 3.0f * v * v) * uCurve[2].Position +
			3.0f * v * v * uCurve[3].Position;
	}

	void BezierSurface::UpdateSurfaceCurves()
	{
		for (uint32_t patchIndex = 0; patchIndex < m_PatchCount; patchIndex++)
		{
			for (uint32_t y = 0, k = 0; y <= m_DivisionCount; y++)
			{
				for (uint32_t x = 0; x <= m_DivisionCount; x++, k++)
					m_PatchVertices[patchIndex][k] = CalculateBezierSurface(m_PatchCurves[patchIndex], x / (float)m_DivisionCount, y / (float)m_DivisionCount);
			}
		}
	}

	void BezierSurface::Draw(const glm::mat4& viewProjection)
	{
		UpdateSurfaceCurves();

		m_VAO->Bind();
		m_PointShader->Bind();
		m_VAO->EnableVertexAttributes(m_PointVBO);
		m_PointShader->UploadUniformMat4("u_MVP", viewProjection * m_Transform->Transform());

		float* vertexDataPtr = &m_PatchVertices[0][0].Position.x;
		uint32_t vertexCount = m_PatchVertices.size() * (m_DivisionCount + 1) * (m_DivisionCount + 1);
		uint32_t sizeOfData = vertexCount * sizeof(SurfaceVertex);
		m_Texture->BindToSamplerSlot(0);
		m_PointShader->UploadUniformInt("u_Texture", 0);
		m_PointShader->UploadUniformMat4("u_ModelMatrix", m_Transform->Transform());
		m_PointVBO->ResizeAndSetData(vertexDataPtr, sizeOfData);
		if (m_WireFrame)
			RenderCommand::SetDrawMode(DrawMode::WireFrame);
		RenderCommand::DrawIndexed(m_VAO, m_PointEBO->GetIndexCount(), RenderTopology::Quads);
		RenderCommand::SetDrawMode(DrawMode::Fill);
		m_PointShader->Unbind();
		m_VAO->Unbind();

		if (m_DrawCurves)
		{
			for(auto patchCurves: m_PatchCurves)
				for (auto curve : patchCurves)
					curve->Draw(viewProjection);
		}
	}

	void BezierSurface::StartPicking(MouseCode mouseCode, Camera& camera)
	{
		if (!m_IsDragging && mouseCode == Engine::Mouse::ButtonLeft)
		{
			for (auto patchCurves : m_PatchCurves)
				for (auto curve : patchCurves)
				{
					glm::vec2 mousePosition = Engine::Input::GetMousePosition();
					glm::vec3 worldCoordinate = camera.ScreenToWorldPoint({ mousePosition.x, mousePosition.y, 0.0f });

					for (uint32_t i = 0; i < curve->GetPointCount(); i++)
					{
						Engine::LineVertex vertex = (*curve)[i];
						glm::vec3 transformOffset = curve->GetTransform()->GetPosition();
						transformOffset.z = 0.0f;

						glm::vec3 toWorld = glm::vec3(curve->GetTransform()->Transform() * glm::vec4(vertex.Position, 1.0));

						glm::vec3 offset = toWorld - worldCoordinate;

						if (glm::dot(offset, offset) < glm::dot(0.04f, 0.04f))
						{
							m_DragCurve = curve;
							m_IsDragging = true;
							m_DragID = i;
						}
					}
				}
		}
	}

	void BezierSurface::StopPicking(MouseCode mouseCode)
	{
		if (m_IsDragging && mouseCode == Engine::Mouse::ButtonLeft)
		{
			m_IsDragging = false;
			m_DragID = -1;
			m_DragCurve = nullptr;
		}
	}

	void BezierSurface::UpdateDragCurve(Camera& camera)
	{
		if (m_DragCurve == nullptr) return;

		glm::vec2 mousePosition = Engine::Input::GetMousePosition();
		glm::vec3 worldCoordinate = camera.ScreenToWorldPoint({ mousePosition.x, mousePosition.y, 0.0f });

		if (m_IsDragging && m_DragID != -1)
		{
			Engine::LineVertex dragPoint = (*m_DragCurve)[m_DragID];
			worldCoordinate.z = 0.0;

			glm::vec2 dragPointWorld = glm::vec2(m_DragCurve->GetTransform()->Transform() * glm::vec4(dragPoint.Position, 1.0));

			if (glm::vec2(worldCoordinate) != glm::vec2(dragPointWorld))
			{
				glm::vec3 curveSpace = glm::vec3(glm::inverse(m_DragCurve->GetTransform()->Transform()) * glm::vec4(worldCoordinate, 1.0f));
				curveSpace.z = 0.0f;
				m_DragCurve->MovePoint(m_DragID, curveSpace);
			}
		}
	}

	void BezierSurface::SetPosition(const glm::vec3& position)
	{
		m_Transform->SetPosition(position);
		for(auto patchCurves: m_PatchCurves)
			for (auto curve : patchCurves)
			{
				curve->GetTransform()->SetPosition(position);
			}
	}

	void BezierSurface::SetScale(const glm::vec3& scale)
	{
		m_Transform->SetScale(scale);
		for (auto patchCurves : m_PatchCurves)
			for (auto curve : patchCurves)
			{
				curve->GetTransform()->SetScale(scale);
			}
	}

	void BezierSurface::SetRotation(const glm::vec3& rotation)
	{
		m_Transform->SetRotation(rotation);
		for (auto patchCurves : m_PatchCurves)
			for (auto curve : patchCurves)
			{
				curve->GetTransform()->SetRotation(rotation);
			}
	}

	static float Wave(float x, float y, float t)
	{
		return sin(PI * (x + y + t));
	}

	static float MultiWave(float x, float y, float t)
	{
		float z = sin(PI * (x + 0.2 * t));
		z += 0.2 * sin(2.0 * PI * (y + t));
		z += sin(PI * (x + y + 0.2 * t));

		return z * (1.0 / 10.0f);
	}

	void BezierSurface::AnimateControls()
	{
		if (!m_Animate) return;

		for (auto patchCurves : m_PatchCurves)
			for (auto curve : patchCurves)
			{
				std::vector<LineVertex>& vertices = curve->GetVertices();

				for (uint32_t i = 0; i < vertices.size(); i++)
				{
					if (i % 3 == 0) continue;

					LineVertex vertex = vertices[i];
					float z = MultiWave(vertex.Position.x, vertex.Position.y, Time::Elapsed() * 0.5f);
					curve->MovePoint(i, { vertex.Position.x, vertex.Position.y, z });
				}
			}
	}

	void BezierSurface::ToggleDrawCurves()
	{
		m_DrawCurves = !m_DrawCurves;
	}

	void BezierSurface::ToggleWireFrame()
	{
		m_WireFrame = !m_WireFrame;
	}

	void BezierSurface::ToggleAnimation()
	{
		m_Animate = !m_Animate;

		if (!m_Animate)
			SetPosition(m_DefaultPosition);
	}

	void BezierSurface::ResetCurves()
	{
		for (uint32_t i = 0; i < m_PatchCount; i++)
		{
			for (uint32_t j = 0; j < m_PatchCurves[i].size(); j++)
			{
				Ref<BezierCurve> curve = m_PatchCurves[i][j];
				std::vector<glm::vec3> defaultPositions = m_PatchCurvesDefaultPositions[i][j];

				curve->SetVertices(defaultPositions);
			}
		}
	}

}