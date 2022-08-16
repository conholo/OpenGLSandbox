#include "WorleyWaterDemoLayer.h"
#include <iostream>
#include <glm/gtc/quaternion.hpp>
#include "imgui/imgui.h"


#define PI 3.14159265359

WorleyWaterDemoLayer::WorleyWaterDemoLayer()
	:m_Camera(45.0f, 1920.0f / 1080.0f, 0.1, 1000.0f)
{

}

WorleyWaterDemoLayer::~WorleyWaterDemoLayer()
{

}

void WorleyWaterDemoLayer::OnAttach()
{
	m_Camera.SetPosition({ 0.0f, 40.0f, 120.0f });
	m_WhiteTexture = Engine::Texture2D::CreateWhiteTexture();

	Engine::Texture2DSpecification boxSpec =
	{
		Engine::ImageUtils::WrapMode::Repeat,
		Engine::ImageUtils::WrapMode::Repeat,
		Engine::ImageUtils::FilterMode::Linear,
		Engine::ImageUtils::FilterMode::Linear,
		Engine::ImageUtils::ImageInternalFormat::FromImage,
		Engine::ImageUtils::ImageDataLayout::FromImage,
		Engine::ImageUtils::ImageDataType::UByte,
	};

	m_BoxTexture = Engine::CreateRef<Engine::Texture2D>("assets/textures/box.jpg", boxSpec);

	Engine::Ref<Engine::Mesh> tessellatedQuad = Engine::MeshFactory::TessellatedQuad(200);
	m_Entity = Engine::CreateRef<Engine::SimpleEntity>(tessellatedQuad, "WorleyWaterPlane");
	m_Entity->GetEntityTransform()->SetScale({ 100.0f, 100.0f, 100.0f });

	m_FloatingEntity = Engine::CreateRef<Engine::SimpleEntity>(Engine::PrimitiveType::Cube, "FlatColor");

	Engine::LightSpecification pointLightSpec =
	{
		Engine::LightType::Point,
		{1.0f, 1.0f, 1.0f},
		1.0f
	};

	m_PointLight = Engine::CreateRef<Engine::Light>(pointLightSpec);
	m_PointLight->GetLightTransform()->SetPosition({0.0f, 30.0f, 15.0f});

	BlinnPhongProperties properties =
	{
		{0.6f, 0.6f, 0.6f},
		{0.6f, 0.6f, 0.6f},
		0.7f,
		0.5f,
		0.1f,
		2.0f
	};

	for (uint32_t i = 0; i < m_FloatingBoxesCount; i++)
	{
		FloatingBox box;
		m_FloatingBoxes.push_back(box);
	}
}

static float Ripple(float x, float z, float t)
{
	float d = sqrt(x * x + z * z);
	float y = sin(5.0 * (PI * d - t));
	return y / (0.5 + 10.0 * d);
}

static float Wave(float x, float z, float t)
{
	return sin(PI * (x + z + t));
}

static float MultiWave(float x, float z, float t)
{
	float y = sin(PI * (x + 0.5 * t));
	y += 0.5 * sin(2.0 * PI * (z + t));
	y += sin(PI * (x + z + 0.25 * t));

	return y * (1.0 / 2.5);
}


void WorleyWaterDemoLayer::OnDetach()
{

}


void WorleyWaterDemoLayer::OnUpdate(float deltaTime)
{
	Engine::RenderCommand::SetDrawMode(m_WireFrame ? Engine::DrawMode::WireFrame : Engine::DrawMode::Fill);
	Engine::RenderCommand::Clear(true, true);
	Engine::RenderCommand::ClearColor(m_ClearColor);

	m_Counter += deltaTime;

	if (!m_PauseFragment)
		m_FragmentCounter += deltaTime;
	if (!m_PauseVertex)
		m_VertexCounter += deltaTime;

	if (m_AnimateVertex)
		m_Entity->GetEntityTransform()->SetScale({ 100.0f, 10.0f, 100.0f });
	else
		m_Entity->GetEntityTransform()->SetScale({ 100.0f, 100.0f, 100.0f });


	if (m_MoveLight)
	{
		if (m_AnimateVertex)
			m_PointLight->GetLightTransform()->SetPosition(glm::vec3(cos(m_Counter) * 60.0f, 100.0f, sin(m_Counter) * 60.0f));
		else
			m_PointLight->GetLightTransform()->SetPosition(glm::vec3(0.0f, sin(m_Counter) * 50.0f, 15.0f));
	}
	else
	{
		if(m_AnimateVertex)
			m_PointLight->GetLightTransform()->SetPosition({ 0.0f, 100.0f, 15.0f });
		else
			m_PointLight->GetLightTransform()->SetPosition({ 0.0f, 30.0f, 15.0f });
	}


	m_WhiteTexture->BindToSamplerSlot(0);
	m_Entity->GetEntityRenderer()->GetShader()->Bind();

	m_Entity->GetEntityRenderer()->GetShader()->UploadUniformFloat3("u_CameraPosition", m_Camera.GetPosition());
	m_Entity->GetEntityRenderer()->GetShader()->UploadUniformMat4("u_ModelMatrix", m_Entity->GetEntityTransform()->Transform());
	m_Entity->GetEntityRenderer()->GetShader()->UploadUniformInt("u_AnimateVertex", m_AnimateVertex ? 1 : 0);
	m_Entity->GetEntityRenderer()->GetShader()->UploadUniformInt("u_SurfaceType", (int)m_SurfaceType);
	m_Entity->GetEntityRenderer()->GetShader()->UploadUniformFloat("u_VertexCounter", m_VertexCounter);

	m_Entity->GetEntityRenderer()->GetShader()->UploadUniformFloat3("u_Light.Position", m_PointLight->GetLightTransform()->GetPosition());
	m_Entity->GetEntityRenderer()->GetShader()->UploadUniformFloat3("u_Light.Color", m_PointLight->GetLightColor());
	m_Entity->GetEntityRenderer()->GetShader()->UploadUniformFloat("u_Light.Intensity", m_PointLight->GetLightIntensity());

	m_Entity->GetEntityRenderer()->GetShader()->UploadUniformInt("u_AnimateFragment", m_AnimateFragment ? 1 : 0);
	m_Entity->GetEntityRenderer()->GetShader()->UploadUniformFloat("u_FragmentCounter", m_FragmentCounter);
	m_Entity->GetEntityRenderer()->GetShader()->UploadUniformFloat3("u_AmbientColor", { 0.8, 0.8, 0.8 });
	m_Entity->GetEntityRenderer()->GetShader()->UploadUniformFloat3("u_DiffuseColor", { 0.0f, 0.7f, 0.9f });
	m_Entity->GetEntityRenderer()->GetShader()->UploadUniformFloat3("u_SpecularColor", { 1.0f, 1.0f, 1.0f });
	m_Entity->GetEntityRenderer()->GetShader()->UploadUniformFloat("u_AmbientStrength", 0.5f);
	m_Entity->GetEntityRenderer()->GetShader()->UploadUniformFloat("u_DiffuseStrength", 0.6f);
	m_Entity->GetEntityRenderer()->GetShader()->UploadUniformFloat("u_SpecularStrength", 1.0f);
	m_Entity->GetEntityRenderer()->GetShader()->UploadUniformFloat("u_Shininess", 32.0f);
	m_Entity->GetEntityRenderer()->GetShader()->UploadUniformInt("u_Texture", 0);

	m_Entity->DrawEntity(m_Camera.GetViewProjection());


	if (m_AnimateVertex && m_ShowBoxes)
	{
		m_BoxTexture->BindToSamplerSlot(0);
		Engine::ShaderLibrary::Get("BlinnPhongWS")->Bind();
		Engine::ShaderLibrary::Get("BlinnPhongWS")->UploadUniformInt("u_Texture", 0);
		Engine::ShaderLibrary::Get("BlinnPhongWS")->UploadUniformFloat3("u_Light.Position", m_PointLight->GetLightTransform()->GetPosition());
		Engine::ShaderLibrary::Get("BlinnPhongWS")->UploadUniformFloat3("u_Light.Color", m_PointLight->GetLightColor());
		Engine::ShaderLibrary::Get("BlinnPhongWS")->UploadUniformFloat("u_Light.Intensity", m_PointLight->GetLightIntensity());

		Engine::ShaderLibrary::Get("BlinnPhongWS")->UploadUniformFloat3("u_MaterialProperties.AmbientColor", m_BoxProperties.AmbientColor);
		Engine::ShaderLibrary::Get("BlinnPhongWS")->UploadUniformFloat("u_MaterialProperties.AmbientStrength", m_BoxProperties.AmbientStrength);
		Engine::ShaderLibrary::Get("BlinnPhongWS")->UploadUniformFloat3("u_MaterialProperties.DiffuseColor", m_BoxProperties.DiffuseColor);
		Engine::ShaderLibrary::Get("BlinnPhongWS")->UploadUniformFloat("u_MaterialProperties.DiffuseStrength", m_BoxProperties.DiffuseStrength);
		Engine::ShaderLibrary::Get("BlinnPhongWS")->UploadUniformFloat("u_MaterialProperties.SpecularStrength", m_BoxProperties.SpecularStrength);
		Engine::ShaderLibrary::Get("BlinnPhongWS")->UploadUniformFloat("u_MaterialProperties.Shininess", m_BoxProperties.Shininess);

		Engine::ShaderLibrary::Get("BlinnPhongWS")->UploadUniformFloat3("u_CameraPosition", m_Camera.GetPosition());


		for (uint32_t i = 0; i < m_FloatingBoxesCount; i++)
		{
			glm::vec3 worldPosition = m_FloatingBoxes[i].Position;
			float x = Engine::Remap(worldPosition.x, -100.0f, 100.0f, -1.0f, 1.0f);
			float z = Engine::Remap(worldPosition.z, -100.0f, 100.0f, -1.0f, 1.0f);
			glm::vec3 remappedPosition = glm::vec3(x, 0.0f, z);

			float y = 0;
			float worldY = 0;

			glm::vec3 directionOfWave;

			switch (m_SurfaceType)
			{
				case SurfaceType::Ripple:
				{
					y = Ripple(x, z, m_VertexCounter);
					worldY = Engine::Remap(y, -1.0f, 1.0f, -10.0f, 10.0f);
					directionOfWave = glm::normalize(glm::vec3(worldPosition.x, worldY, worldPosition.z));
					break;
				}
				case SurfaceType::Wave:
				{
					y = Wave(x, z, m_VertexCounter);
					worldY = Engine::Remap(y, -1.0f, 1.0f, -10.0f, 10.0f);
					directionOfWave = glm::normalize(glm::vec3(-100.0f, 0.0f, -100.0f) - glm::vec3(100.0f, 0.0f, 100.0f));
					break;
				}
				case SurfaceType::MultiWave:
				{
					y = MultiWave(x, z, m_VertexCounter);
					worldY = Engine::Remap(y, -1.0f, 1.0f, -10.0f, 10.0f);
					directionOfWave = glm::normalize(glm::vec3(-100.0f, 0.0f, -100.0f) - glm::vec3(100.0f, 0.0f, 100.0f));
					break;
				}
			}

			glm::vec3 normal{ x, y, z };
			glm::vec3 waveForceVector = glm::normalize(directionOfWave + normal);
			float pitch = glm::asin(waveForceVector.y);
			float yaw = std::atan2(waveForceVector.x, waveForceVector.z);

			glm::quat targetEuler = glm::quat({ pitch, yaw, 0.0 });
			glm::vec3 newEulers = glm::eulerAngles(targetEuler);

			m_FloatingEntity->GetEntityTransform()->SetRotation(glm::degrees(newEulers));
			m_FloatingEntity->GetEntityTransform()->SetPosition({ worldPosition.x, worldY, worldPosition.z });
			m_FloatingEntity->GetEntityTransform()->SetScale(m_FloatingBoxes[i].Scale);


			m_FloatingEntity->GetEntityRenderer()->GetShader()->Bind();
			m_FloatingEntity->GetEntityRenderer()->GetShader()->UploadUniformMat4("u_ModelMatrix", m_FloatingEntity->GetEntityTransform()->Transform());
			m_FloatingEntity->DrawEntity(m_Camera.GetViewProjection());
		}
	}

	m_PointLight->DrawDebug(m_Camera.GetViewProjection());

	m_Camera.Update(deltaTime);
}

void WorleyWaterDemoLayer::OnEvent(Engine::Event& event)
{
	m_Camera.OnEvent(event);
	Engine::EventDispatcher dispatcher(event);

	dispatcher.Dispatch<Engine::KeyPressedEvent>(BIND_FN(WorleyWaterDemoLayer::OnKeyPressed));
}

void WorleyWaterDemoLayer::OnImGuiRender()
{
	ImGui::Begin("Test");

	ImGui::Checkbox("Animate Light", &m_MoveLight);
	ImGui::Checkbox("Animate Vertex", &m_AnimateVertex);
	ImGui::Checkbox("Animate Fragment", &m_AnimateFragment);
	if (m_AnimateVertex)
	{
		ImGui::Checkbox("Show Boxes", &m_ShowBoxes);
	}

	ImGui::End();
}

bool WorleyWaterDemoLayer::OnKeyPressed(Engine::KeyPressedEvent& keyPressedEvent)
{
	if (keyPressedEvent.GetKeyCode() == Engine::Key::LeftShift)
	{
		m_AnimateVertex = !m_AnimateVertex;

		if (m_AnimateVertex)
			m_Entity->GetEntityTransform()->SetScale({ 100.0f, 10.0f, 100.0f });
		else
			m_Entity->GetEntityTransform()->SetScale({ 100.0f, 100.0f, 100.0f });

		return true;
	}

	if (keyPressedEvent.GetKeyCode() == Engine::Key::P)
	{
		std::cout << m_Camera.GetPosition().x << "," << 0.0f << "," << m_Camera.GetPosition().z << "\n";
	}

	if (keyPressedEvent.GetKeyCode() == Engine::Key::RightShift)
		m_AnimateFragment = !m_AnimateFragment;

	if (keyPressedEvent.GetKeyCode() == Engine::Key::Space)
		m_MoveLight = !m_MoveLight;

	if (keyPressedEvent.GetKeyCode() == Engine::Key::M)
		m_WireFrame = !m_WireFrame;

	if (m_AnimateFragment)
	{
		if (keyPressedEvent.GetKeyCode() == Engine::Key::F)
			m_PauseFragment = !m_PauseFragment;
	}

	if (m_AnimateVertex)
	{
		if (keyPressedEvent.GetKeyCode() == Engine::Key::V)
			m_PauseVertex = !m_PauseVertex;

		if (keyPressedEvent.GetKeyCode() == Engine::Key::LeftControl)
			m_ShowBoxes = !m_ShowBoxes;

		if (keyPressedEvent.GetKeyCode() == Engine::Key::D1)
			m_SurfaceType = SurfaceType::Ripple;
		else if (keyPressedEvent.GetKeyCode() == Engine::Key::D2)
			m_SurfaceType = SurfaceType::Wave;
		else if (keyPressedEvent.GetKeyCode() == Engine::Key::D3)
			m_SurfaceType = SurfaceType::MultiWave;

		if (keyPressedEvent.GetKeyCode() == Engine::Key::Tab)
		{
			for (uint32_t i = 0; i < m_FloatingBoxesCount; i++)
				m_FloatingBoxes[i].Randomize();
		}
	}

	return false;
}
