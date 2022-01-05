#include "Layers/CubeMapLayer.h"

#include <vector>
#include <string>
#include <iostream>
#include <glm/gtc/matrix_transform.hpp>

CubeMapLayer::CubeMapLayer()
	:Engine::Layer("Cube Map Layer"), m_Camera(45.0f, 1920.0f / 1080.0f, 0.1f, 1000.0f)
{

}

CubeMapLayer::~CubeMapLayer()
{

}

void CubeMapLayer::OnAttach()
{
	std::vector<std::string> faceFiles =
	{
		"assets/textures/skybox/right.jpg",
		"assets/textures/skybox/left.jpg",
		"assets/textures/skybox/top.jpg",
		"assets/textures/skybox/bottom.jpg",
		"assets/textures/skybox/front.jpg",
		"assets/textures/skybox/back.jpg",
	};

	Engine::TextureSpecification spec =
	{
		Engine::ImageUtils::Usage::Texture,
		Engine::ImageUtils::WrapMode::ClampToEdge,
		Engine::ImageUtils::FilterMode::Linear,
		Engine::ImageUtils::ImageInternalFormat::FromImage,
		Engine::ImageUtils::ImageDataLayout::FromImage,
		Engine::ImageUtils::ImageDataType::UByte,
	};

	m_CubeMap = Engine::CreateRef<Engine::CubeMap>(Engine::CreateRef<Engine::TextureCube>(spec, faceFiles), Engine::ShaderLibrary::Get("SkyboxTest"));
}

void CubeMapLayer::OnDetach()
{

}

void CubeMapLayer::OnUpdate(float deltaTime)
{
	Engine::RenderCommand::Clear(true, true);
	Engine::RenderCommand::ClearColor({ 0.1f, 0.1f, 0.1f, 0.1f });

	if (m_CameraOrbitModeActive)
	{
		m_CameraYawRotate += deltaTime * 30.0f;
		m_Camera.Orbit({ 0.0f, 50.0f, 80.0f }, glm::vec3(0.0f), { 0.0f, m_CameraYawRotate, 0.0f });
	}
	else
	{
		m_Camera.Update(deltaTime);
	}

	DrawReflectionScene();
}

void CubeMapLayer::OnEvent(Engine::Event& event)
{
	Engine::EventDispatcher dispatcher(event);

	dispatcher.Dispatch<Engine::KeyPressedEvent>(BIND_FN(CubeMapLayer::OnKeyPressed));

	m_Camera.OnEvent(event);
}

bool CubeMapLayer::OnKeyPressed(Engine::KeyPressedEvent& event)
{
	if (event.GetKeyCode() == Engine::Key::Space)
	{
		m_CameraOrbitModeActive = !m_CameraOrbitModeActive;
		m_Camera.ToggleIsLocked(m_CameraOrbitModeActive);
	}

	return true;
}

void CubeMapLayer::DrawReflectionScene()
{
	m_CubeMap->GetTexture3D()->BindToSamplerSlot(0);
	Engine::ShaderLibrary::Get("EnvironmentReflection")->Bind();
	Engine::ShaderLibrary::Get("EnvironmentReflection")->UploadUniformFloat3("u_CameraPosition", m_Camera.GetPosition());
	for (float x = -10.0f; x < 10.0f; x += 2.0f)
	{
		Engine::ShaderLibrary::Get("EnvironmentReflection")->Bind();
		Engine::Ref<Engine::SimpleEntity> sphere = Engine::CreateRef<Engine::SimpleEntity>(Engine::PrimitiveType::Sphere, "EnvironmentReflection");
		sphere->GetEntityTransform()->SetPosition({ x, 0.0f, 0.0f });
		glm::mat4 modelMatrix = sphere->GetEntityTransform()->Transform();
		glm::mat3 normalMatrix = glm::mat3(glm::transpose(glm::inverse(modelMatrix)));
		Engine::ShaderLibrary::Get("EnvironmentReflection")->UploadUniformMat3("u_NormalMatrix", normalMatrix);
		Engine::ShaderLibrary::Get("EnvironmentReflection")->UploadUniformMat4("u_ModelMatrix", modelMatrix);
		sphere->DrawEntity(m_Camera.GetViewProjection());
	}
	m_CubeMap->GetTexture3D()->BindToSamplerSlot(0);

	glm::mat4 view3x3 = glm::mat4(glm::mat3(m_Camera.GetView()));
	m_CubeMap->Submit(m_Camera.GetProjection() * view3x3);
}
