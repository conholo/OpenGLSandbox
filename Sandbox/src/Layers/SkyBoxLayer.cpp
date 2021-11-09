#include "SkyboxLayer.h"
#include <iostream>

#include <glad/glad.h>

SkyboxLayer::SkyboxLayer()
	:Engine::Layer("Test Cube Layer"), m_Camera(45.0f, 1920.0f / 1080.0f, 0.1f, 1000.0f)
{

}

SkyboxLayer::~SkyboxLayer()
{

}

void SkyboxLayer::OnAttach()
{
	m_Camera.SetPosition({ 0.0f, 5.0f, 20.0 });
	Engine::TextureSpecification skyBoxSpec =
	{
		Engine::ImageUtils::Usage::Storage,
		Engine::ImageUtils::WrapMode::ClampToEdge,
		Engine::ImageUtils::FilterMode::Linear,
		Engine::ImageUtils::ImageInternalFormat::RGBA32F,
		Engine::ImageUtils::ImageDataLayout::RGBA,
		Engine::ImageUtils::ImageDataType::Float,
		1024, 1024
	};

	Engine::Texture2DSpecification whiteTextureSpec =
	{
		Engine::ImageUtils::WrapMode::Repeat,
		Engine::ImageUtils::WrapMode::Repeat,
		Engine::ImageUtils::FilterMode::Linear,
		Engine::ImageUtils::FilterMode::Linear,
		Engine::ImageUtils::ImageInternalFormat::RGBA8,
		Engine::ImageUtils::ImageDataLayout::RGBA,
		Engine::ImageUtils::ImageDataType::UByte,
		1, 1
	};

	m_Cube = Engine::CreateRef<Engine::SimpleEntity>(Engine::PrimitiveType::Cube, "EnvironmentReflection");
	m_Cube->GetEntityTransform()->SetPosition({ 0.0f, 10.0f, 0.0f });
	m_Cube->GetEntityTransform()->SetScale({ 2.5f, 2.5f, 2.5f });

	m_WhiteTexture = Engine::CreateRef<Engine::Texture2D>(whiteTextureSpec);
	uint32_t whiteTextureData = 0xffffffff;
	m_WhiteTexture->SetData(&whiteTextureData, sizeof(uint32_t));

	// Create Texture Cube
	m_CubeTexture = Engine::CreateRef<Engine::Texture3D>(skyBoxSpec, nullptr);
	m_SkyBox = Engine::CreateRef<Engine::CubeMap>(m_CubeTexture, Engine::ShaderLibrary::Get("Skybox"));
	m_CubeTexture->BindToImageSlot(0, 0, Engine::ImageUtils::TextureAccessLevel::WriteOnly, Engine::ImageUtils::TextureShaderDataFormat::RGBA32F);

	m_EditorGrid = Engine::CreateRef<Engine::EditorGrid>();
}

void SkyboxLayer::OnDetach()
{

}


void SkyboxLayer::DrawSkybox(float deltaTime)
{
	if (m_AnimateInclination)
		m_TAI.z += deltaTime * 0.1f;

	// Write to Cube
	Engine::ShaderLibrary::Get("Preetham")->Bind();
	Engine::ShaderLibrary::Get("Preetham")->UploadUniformFloat3("u_TAI", m_TAI);
	Engine::ShaderLibrary::Get("Preetham")->DispatchCompute(m_SkyBox->GetTexture3D()->GetWidth() / 32, m_SkyBox->GetTexture3D()->GetHeight() / 32, 6);
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

	// Read from & Render Cubemap
	Engine::ShaderLibrary::Get("Skybox")->Bind();
	Engine::ShaderLibrary::Get("Skybox")->UploadUniformFloat("u_TextureLOD", 0);
	Engine::ShaderLibrary::Get("Skybox")->UploadUniformFloat("u_Intensity", 1.0f);
	glm::mat4 inverseViewProjection = glm::mat4(glm::inverse(m_Camera.GetViewProjection()));
	m_SkyBox->Submit(inverseViewProjection);
}

void SkyboxLayer::DrawReflectionSpheres()
{
	m_SkyBox->GetTexture3D()->BindToSamplerSlot(0);
	Engine::ShaderLibrary::Get("EnvironmentReflection")->Bind();
	Engine::ShaderLibrary::Get("EnvironmentReflection")->UploadUniformFloat3("u_CameraPosition", m_Camera.GetPosition());

	m_Cube->GetEntityTransform()->SetRotation({ sin(m_Counter * 0.1f) * 360.0f, cos(m_Counter * 0.1f) * 360.0f, sin(m_Counter * 0.1f) * 360.0f });
	glm::mat4 modelMatrix = m_Cube->GetEntityTransform()->Transform();
	glm::mat3 normalMatrix = glm::mat3(glm::transpose(glm::inverse(modelMatrix)));
	Engine::ShaderLibrary::Get("EnvironmentReflection")->UploadUniformMat3("u_NormalMatrix", normalMatrix);
	Engine::ShaderLibrary::Get("EnvironmentReflection")->UploadUniformMat4("u_ModelMatrix", modelMatrix);
	m_Cube->DrawEntity(m_Camera.GetViewProjection());

	for (float x = -10.0f; x < 10.0f; x += 2.0f)
	{
		Engine::ShaderLibrary::Get("EnvironmentReflection")->Bind();
		Engine::Ref<Engine::SimpleEntity> sphere = Engine::CreateRef<Engine::SimpleEntity>(Engine::PrimitiveType::Sphere, "EnvironmentReflection");
		sphere->GetEntityTransform()->SetPosition({ x, 5.0f, 0.0f });
		glm::mat4 modelMatrix = sphere->GetEntityTransform()->Transform();
		glm::mat3 normalMatrix = glm::mat3(glm::transpose(glm::inverse(modelMatrix)));
		Engine::ShaderLibrary::Get("EnvironmentReflection")->UploadUniformMat3("u_NormalMatrix", normalMatrix);
		Engine::ShaderLibrary::Get("EnvironmentReflection")->UploadUniformMat4("u_ModelMatrix", modelMatrix);
		sphere->DrawEntity(m_Camera.GetViewProjection());
	}
}

void SkyboxLayer::OnUpdate(float deltaTime)
{
	m_Camera.Update(deltaTime);

	m_Counter += deltaTime;

	Engine::RenderCommand::Clear(true, true);
	Engine::RenderCommand::ClearColor(m_ClearColor);

	DrawReflectionSpheres();
	DrawSkybox(deltaTime);
	m_EditorGrid->Draw(m_Camera);
}

void SkyboxLayer::OnEvent(Engine::Event& event)
{
	Engine::EventDispatcher dispatcher(event);

	dispatcher.Dispatch<Engine::KeyPressedEvent>(BIND_FN(SkyboxLayer::OnKeyPressed));

	m_Camera.OnEvent(event);
}

bool SkyboxLayer::OnKeyPressed(Engine::KeyPressedEvent& keyPressedEvent)
{
	if (keyPressedEvent.GetKeyCode() == Engine::Key::T)
		m_TAI.x += m_TAIStep;
	else if (keyPressedEvent.GetKeyCode() == Engine::Key::Y)
		m_TAI.x -= m_TAIStep;


	if (keyPressedEvent.GetKeyCode() == Engine::Key::F)
		m_TAI.y += m_TAIStep;
	else if (keyPressedEvent.GetKeyCode() == Engine::Key::G)
		m_TAI.y -= m_TAIStep;


	if (keyPressedEvent.GetKeyCode() == Engine::Key::I)
		m_TAI.z += m_TAIStep / 100.0f;
	else if (keyPressedEvent.GetKeyCode() == Engine::Key::O)
		m_TAI.z -= m_TAIStep;

	if (keyPressedEvent.GetKeyCode() == Engine::Key::Space)
		std::cout << "TAI: " << m_TAI.x << "," << m_TAI.y << "," << m_TAI.z << "\n";

	if (keyPressedEvent.GetKeyCode() == Engine::Key::Tab)
		m_AnimateInclination = !m_AnimateInclination;

	return true;
}
