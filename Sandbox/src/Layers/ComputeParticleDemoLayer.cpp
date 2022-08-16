#include "Layers/ComputeParticleDemoLayer.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>

#include <glad/glad.h>

ComputeParticleDemoLayer::ComputeParticleDemoLayer()
	:Engine::Layer("Compute Test Layer"), m_Camera(45.0f, 1920.0f/1080.0f, 0.1f, 1000.0f)
{
	m_Camera.SetPosition({ 0.0f, -5.0f, 60.0f });
}

ComputeParticleDemoLayer::~ComputeParticleDemoLayer()
{
}



void ComputeParticleDemoLayer::OnAttach()
{
	Engine::Texture2DSpecification textureSpec
	{
		Engine::ImageUtils::WrapMode::Repeat,
		Engine::ImageUtils::WrapMode::Repeat,
		Engine::ImageUtils::FilterMode::Linear,
		Engine::ImageUtils::FilterMode::Linear,
		Engine::ImageUtils::ImageInternalFormat::FromImage,
		Engine::ImageUtils::ImageDataLayout::FromImage,
		Engine::ImageUtils::ImageDataType::UByte,
		1, 1
	};

	m_WhiteTexture = Engine::Texture2D::CreateWhiteTexture();
	m_BrickTexture = Engine::CreateRef<Engine::Texture2D>("assets/textures/brick-wall.jpg", textureSpec);
	m_GrassTexture = Engine::CreateRef<Engine::Texture2D>("assets/textures/grass.jpg", textureSpec);

	m_GroundPlane = Engine::CreateRef<Engine::SimpleEntity>(Engine::PrimitiveType::Plane, "FlatColor");
	m_GroundPlane->GetEntityTransform()->SetPosition({ 0.0f, -2.0f, 0.0f });
	m_GroundPlane->GetEntityTransform()->SetScale({ 20.0f, 0.0f, 20.0f });

	m_Particles = new Particle[m_ParticleCount];
	m_Velocities = new glm::vec4[m_ParticleCount];

	for (size_t i = 0; i < m_ParticleCount; i++)
	{
		m_Particles[i].Position = glm::vec4(Engine::Random::RandomRange(0.0f, 0.0f), Engine::Random::RandomRange(20.0f, 25.0f), Engine::Random::RandomRange(0.0f, 0.0f), 1.0f);
		m_Velocities[i] = glm::vec4(Engine::Random::RandomRange(-0.1f, 0.1f), Engine::Random::RandomRange(-5.0, -15.0f), Engine::Random::RandomRange(-0.1f, 0.1f), 0.0f);
		m_Particles[i].Color = glm::vec4(Engine::Random::RandomRange(0.0f, 1.0f), Engine::Random::RandomRange(0.0f, 1.0f), Engine::Random::RandomRange(0.0f, 1.0f), 1.0f);
	}

	m_ParticleBuffer = Engine::CreateRef<Engine::ShaderStorageBuffer>(m_Particles, m_ParticleCount * sizeof(Particle));
	m_VelocityBuffer = Engine::CreateRef<Engine::ShaderStorageBuffer>(m_Velocities, m_ParticleCount * sizeof(glm::vec4));

	m_ParticleBuffer->BindToComputeShader(4, Engine::ShaderLibrary::Get("ParticleCompute")->GetID());
	m_VelocityBuffer->BindToComputeShader(5, Engine::ShaderLibrary::Get("ParticleCompute")->GetID());
}

void ComputeParticleDemoLayer::OnDetach()
{

}

void ComputeParticleDemoLayer::OnUpdate(float deltaTime)
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


	Engine::ShaderLibrary::Get("ParticleCompute")->DispatchCompute(m_ParticleCount / 128, 1, 1);
	Engine::ShaderLibrary::Get("ParticleCompute")->UploadUniformFloat("u_DeltaTime", deltaTime);
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

	Engine::ShaderLibrary::Get("VertexColorSimple")->Bind();
	Engine::ShaderLibrary::Get("VertexColorSimple")->UploadUniformMat4("u_ViewProjectionMatrix", m_Camera.GetViewProjection());
	glBindBuffer(GL_ARRAY_BUFFER, m_ParticleBuffer->GetID());
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 8 * sizeof(float), 0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(4 * sizeof(float)));
	glEnableClientState(GL_VERTEX_ARRAY);
	glDrawArrays(GL_POINTS, 0, m_ParticleCount);
	glDisableClientState(GL_VERTEX_ARRAY);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void ComputeParticleDemoLayer::OnEvent(Engine::Event& event)
{
	Engine::EventDispatcher dispatcher(event);

	dispatcher.Dispatch<Engine::KeyPressedEvent>(BIND_FN(ComputeParticleDemoLayer::OnKeyPressed));

	m_Camera.OnEvent(event);
}

bool ComputeParticleDemoLayer::OnKeyPressed(Engine::KeyPressedEvent& event)
{
	if (event.GetKeyCode() == Engine::Key::Space)
	{
		m_CameraOrbitModeActive = !m_CameraOrbitModeActive;
		m_Camera.ToggleIsLocked(m_CameraOrbitModeActive);
	}
	else if (event.GetKeyCode() == Engine::Key::LeftShift)
	{
		Reset();
	}

	return true;
}

void ComputeParticleDemoLayer::Reset()
{
	OnAttach();
}

