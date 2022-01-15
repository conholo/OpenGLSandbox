#include "Layers/Testing/TestSSBOReadWriteLayer.h"
#include <imgui/imgui.h>

#include <glm/gtx/string_cast.hpp>

TestSSBOReadWriteLayer::TestSSBOReadWriteLayer()
{

}

TestSSBOReadWriteLayer::~TestSSBOReadWriteLayer()
{

}

void TestSSBOReadWriteLayer::OnAttach()
{
	Engine::ShaderLibrary::Load("assets/shaders/Testing/TestSSBORW.shader");
	Engine::Application::GetApplication().GetWindow().ToggleMaximize(true);
	m_Camera.SetOrthographic();

	m_QuadDisplay = Engine::CreateRef<Engine::SimpleEntity>(Engine::PrimitiveType::Quad, "FlatColor");
	m_QuadDisplay->GetEntityTransform()->SetScale({ 1.5f, 1.5f, 1.5f });

	Engine::Texture2DSpecification spec =
	{
		Engine::ImageUtils::WrapMode::Repeat,
		Engine::ImageUtils::WrapMode::Repeat,
		Engine::ImageUtils::FilterMode::Linear,
		Engine::ImageUtils::FilterMode::Linear,
		Engine::ImageUtils::ImageInternalFormat::RGBA32F,
		Engine::ImageUtils::ImageDataLayout::RGBA,
		Engine::ImageUtils::ImageDataType::Float,
		m_Dimensions.x, m_Dimensions.y
	};

	m_RWTexture = Engine::CreateRef<Engine::Texture2D>(spec);

	m_Values.resize(m_Dimensions.x * m_Dimensions.y);
	m_TestSSBO = Engine::CreateRef<Engine::ShaderStorageBuffer>(m_Values.data(), sizeof(glm::vec4) * m_Values.size());
}

void TestSSBOReadWriteLayer::OnDetach()
{

}

void TestSSBOReadWriteLayer::OnUpdate(float deltaTime)
{
	m_Camera.Update(deltaTime);
	Engine::RenderCommand::Clear(true, true);
	Engine::RenderCommand::ClearColor({0.0f, 0.0f, 0.0f, 0.0f});
}

void TestSSBOReadWriteLayer::OnEvent(Engine::Event& e)
{
	m_Camera.OnEvent(e);
}

void TestSSBOReadWriteLayer::OnImGuiRender()
{
	ImGui::Begin("Controls");
	if (ImGui::Button("Refresh"))
	{
		uint32_t threadGroups = glm::ceil(m_Dimensions.x / (float)m_LocalGroupSize);

		Engine::ShaderLibrary::Get("TestSSBORW")->Bind();
		m_TestSSBO->BindToComputeShader(0, Engine::ShaderLibrary::Get("TestSSBORW")->GetID());
		Engine::ShaderLibrary::Get("TestSSBORW")->UploadUniformFloat2("u_Dimensions", m_Dimensions);
		Engine::ShaderLibrary::Get("TestSSBORW")->DispatchCompute(threadGroups, threadGroups, 1);
		Engine::ShaderLibrary::Get("TestSSBORW")->EnableBufferUpdateBarrierBit();

		glm::vec2* values = (glm::vec2*)m_TestSSBO->GetData();
		memcpy(m_Values.data(), values, sizeof(glm::vec4) * m_Values.size());
	}
	
	for (int i = 0; i < m_Values.size(); i++)
	{
		std::string value = std::to_string(i) + ": " + glm::to_string(glm::vec2(m_Values[i]));
		ImGui::Text(value.c_str());
	}
	

	ImGui::End();
}
