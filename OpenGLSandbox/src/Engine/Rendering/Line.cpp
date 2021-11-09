#include "Engine/Rendering/Line.h"
#include "Engine/Rendering/RenderCommand.h"

namespace Engine
{
	Line::Line(const std::vector<LineVertex>& vertices, const std::string& shaderName)
		:m_Vertices(vertices), m_Shader(ShaderLibrary::Get(shaderName))
	{
		Initialize();
	}

	Line::Line(std::initializer_list<LineVertex>& vertices, const std::string& shaderName)
		:m_Vertices(vertices), m_Shader(ShaderLibrary::Get(shaderName))
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
}