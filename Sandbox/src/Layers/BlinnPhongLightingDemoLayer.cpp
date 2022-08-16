#include "BlinnPhongLightingDemoLayer.h"

#include <sstream>
#include <iostream>

BlinnPhongLightingDemoLayer::BlinnPhongLightingDemoLayer()
	:m_Camera(45.0f, 1920.0f / 1080.0f, 0.1f, 1000.0f)
{
	m_Camera.SetPosition({ 0.0f, 2.0f, 15.0f });
}

BlinnPhongLightingDemoLayer::~BlinnPhongLightingDemoLayer()
{

}

static Engine::Ref<Engine::Light> CreatePointLight(const glm::vec3& position, const glm::vec3& attenuation)
{
	Engine::LightSpecification pointLightSpec =
	{
		Engine::LightType::Point,
		{Engine::Random::RandomRange(0.0f, 1.0f), Engine::Random::RandomRange(0.0f, 1.0f), Engine::Random::RandomRange(0.0f, 1.0f)},
		Engine::Random::RandomRange(0.0f, 1.0f),
		attenuation.x,
		attenuation.y,
		attenuation.z,
	};

	Engine::Ref<Engine::Light> pointLight = Engine::CreateRef<Engine::Light>(pointLightSpec);
	pointLight->GetLightTransform()->SetPosition(position);
	return pointLight;
}


static Engine::Ref<Engine::Light> CreateSpotLight(const glm::vec3& position, const glm::vec3& color, const glm::vec3& lookAt, float intensity, const glm::vec3& attenuation, float innerCutOff, float outerCutOff)
{
	Engine::LightSpecification spotLightSpec =
	{
		Engine::LightType::Spot,
		color,
		intensity,
		attenuation.x,
		attenuation.y,
		attenuation.z,
		{0.0f, 0.0f, 0.0f},
		glm::cos(glm::radians(innerCutOff)),
		glm::cos(glm::radians(outerCutOff)),
		Engine::PrimitiveType::Cube
	};

	Engine::Ref<Engine::Light> spotLight = Engine::CreateRef<Engine::Light>(spotLightSpec);
	spotLight->GetLightTransform()->SetPosition(position);
	spotLight->GetLightTransform()->LookAt(lookAt);
	spotLight->GetLightTransform()->SetScale({ 0.3f, 0.3f, 1.0f });
	spotLight->SetLightDirection(spotLight->GetLightTransform()->Forward());

	return spotLight;
}

static bool CatWalkSpin(const Engine::Ref<Engine::SimpleEntity>& entity, float percent)
{
	glm::vec3 currentRotation = entity->GetEntityTransform()->GetRotation();

	float x = Engine::Lerp(0, 360.0f, percent * percent);
	float y = Engine::Lerp(0, 360.0f, percent * percent);
	float z = Engine::Lerp(0, 360.0f, percent * percent);

	glm::vec3 newRotation{ x, y, z };
	entity->GetEntityTransform()->SetRotation(newRotation);

	return percent >= 1.0f;
}

static bool CatWalkScale(const Engine::Ref<Engine::SimpleEntity>& entity, float percent)
{
	glm::vec3 currentRotation = entity->GetEntityTransform()->GetRotation();

	float t = percent > 0.5 ? 1 - percent : percent;

	float x = Engine::Lerp(1.0f, 3.0f, t);
	float y = Engine::Lerp(1.0f, 3.0f, t);
	float z = Engine::Lerp(1.0f, 3.0f, t);

	glm::vec3 newScale{ x, y, z };
	entity->GetEntityTransform()->SetScale(newScale);

	return percent >= 1.0f;
}


static bool WalkTo(const Engine::Ref<Engine::SimpleEntity>& entity, const glm::vec3& start, const glm::vec3& target, float percent)
{
	float distance = glm::distance(start, target);

	float x = Engine::Lerp(start.x, target.x, percent);
	float y = Engine::Lerp(start.y, target.y, percent);
	float z = Engine::Lerp(start.z, target.z, percent);

	glm::vec3 newPosition{ x, y, z };
	entity->GetEntityTransform()->SetPosition(newPosition);

	return percent >= 1.0f;
}

void BlinnPhongLightingDemoLayer::OnAttach()
{
	m_WhiteTexture = Engine::Texture2D::CreateWhiteTexture();

	Engine::Texture2DSpecification mapSpec =
	{
		Engine::ImageUtils::WrapMode::Repeat,
		Engine::ImageUtils::WrapMode::Repeat,
		Engine::ImageUtils::FilterMode::Linear,
		Engine::ImageUtils::FilterMode::Linear,
		Engine::ImageUtils::ImageInternalFormat::FromImage,
		Engine::ImageUtils::ImageDataLayout::FromImage,
		Engine::ImageUtils::ImageDataType::UByte,
	};

	m_MapTexture = Engine::CreateRef<Engine::Texture2D>("assets/textures/water-glass.jpg", mapSpec);
	m_GroundTexture = Engine::CreateRef<Engine::Texture2D>("assets/textures/ground-blue.jpg", mapSpec);
	m_WallTexture = Engine::CreateRef<Engine::Texture2D>("assets/textures/space.jpg", mapSpec);


	float maxZDistance = m_PointLightCount * 2.0f;

	for (uint32_t i = 0; i < m_SpotLightCount; i++)
	{
		glm::vec3 position = glm::vec3(0.0f, 10.0f, -maxZDistance / 2);
		position.x += i % 2 == 0 ? 8.0f : -8.0f;
		position.z += i * 2.0f;

		glm::vec3 lookAt = { 0.0f, 0.0f, position.z };

		glm::vec3 attenuation = { 0.5f, 0.0001f, 0.003f };
		glm::vec3 color = { Engine::Random::RandomRange(0.0f, 1.0f), Engine::Random::RandomRange(0.0f, 1.0f), Engine::Random::RandomRange(0.0f, 1.0f) };
		Engine::Ref<Engine::Light> spotLight = CreateSpotLight(position, color, lookAt, 1.0f, attenuation, 2.0f, 8.0f);
		m_SpotLights.push_back(spotLight);
		m_SpotLightFocalPoints.push_back(lookAt);
	}

	Engine::LightSpecification directionalLightSpec =
	{
		Engine::LightType::Directional,
		{0.5f, 0.5f, 0.5f},
		0.5f
	};

	m_DirectionalLight = Engine::CreateRef<Engine::Light>(directionalLightSpec);
	m_DirectionalLight->GetLightTransform()->SetPosition({ 20.0f, 500.0f, 5.0f });

	for (uint32_t i = 0; i < m_PointLightCount; i++)
	{
		Engine::LightSpecification pointLightSpec =
		{
			Engine::LightType::Point,
			{Engine::Random::RandomRange(0.0f, 1.0f), Engine::Random::RandomRange(0.0f, 1.0f), Engine::Random::RandomRange(0.0f, 1.0f)},
			1.0f,
			Engine::Random::RandomRange(1.0f, 10.0f),
			Engine::Random::RandomRange(0.0f, 0.00000001f),
			0.00000000001f,
		};

		Engine::Ref<Engine::Light> pointLight = Engine::CreateRef<Engine::Light>(pointLightSpec);
		glm::vec3 position{ Engine::Random::RandomRange(-20.0f, 20.0f), Engine::Random::RandomRange(1.0f, 6.0f), Engine::Random::RandomRange(-20.0f, 20.0f) };
		m_PointLightPositions.push_back(position);
		pointLight->GetLightTransform()->SetPosition(position);
		m_PointLights.push_back(pointLight);
	}

	m_DiscoBall = Engine::CreateRef<Engine::SimpleEntity>(Engine::PrimitiveType::Sphere, "BlinnPhongVS");
	m_Plane = Engine::CreateRef<Engine::SimpleEntity>(Engine::PrimitiveType::Plane, "BlinnPhongVS");
	m_Wall = Engine::CreateRef<Engine::SimpleEntity>(Engine::PrimitiveType::Quad, "BlinnPhongVS");
	m_Wall->GetEntityTransform()->SetPosition({ 0.0f, 12.0f, -22.5f });
	m_Wall->GetEntityTransform()->SetScale({ 50.0f, 25.0f, 50.0f });


	m_DisplayCube = Engine::CreateRef<Engine::SimpleEntity>(Engine::PrimitiveType::Cube, "BlinnPhongVS");
	m_DisplayCube->GetEntityTransform()->SetPosition({ -10.0f, 2.0f, 5.0f });
	m_DisplayCube->GetEntityTransform()->SetScale({ 1.0f, 1.0f, 1.0f });
	m_DisplayCube->GetEntityTransform()->SetRotation({ -45.0f, 25.0f, 2.0f });
	m_DisplaySphere = Engine::CreateRef<Engine::SimpleEntity>(Engine::PrimitiveType::Sphere, "BlinnPhongVS");
	m_DisplaySphere->GetEntityTransform()->SetPosition({ -8.0f, 2.0f, 5.0f });
	m_DisplaySphere->GetEntityTransform()->SetScale({ 0.75f, 0.75f, 0.75f });
	m_TexturedDisplaySphere = Engine::CreateRef<Engine::SimpleEntity>(Engine::PrimitiveType::Sphere, "BlinnPhongVS");
	m_TexturedDisplaySphere->GetEntityTransform()->SetPosition({ -12.0f, 2.0f, 5.0f });
	m_TexturedDisplaySphere->GetEntityTransform()->SetScale({ 0.75f, 0.75f, 0.75f });

	m_FlatShadedShinyProperties =
	{
		"BlinnPhongVS",
		{1.0f, 1.0f, 1.0f},
		{1.0f, 1.0f, 1.0f},
		0.2f,
		0.5f,
		1.0f,
		256.0f,
		0
	};

	m_SmoothProperties =
	{
		"BlinnPhongVS",
		{1.0f, 1.0f, 1.0f},
		{1.0f, 1.0f, 1.0f},
		0.2f,
		0.5f,
		1.0f,
		256.0f,
		1
	};

	m_RoughProperties =
	{
		"BlinnPhongVS",
		{0.0f, 0.8f, 1.0f},
		{0.8f, 0.8f, 0.8f},
		0.3f,
		0.7f,
		0.1f,
		2.0f,
		1
	};

	Engine::Ref<Engine::SimpleEntity> catWalkCube = Engine::CreateRef<Engine::SimpleEntity>(Engine::PrimitiveType::Cube, "BlinnPhongVS");
	Engine::Ref<Engine::SimpleEntity> catWalkTriangle = Engine::CreateRef<Engine::SimpleEntity>(Engine::PrimitiveType::Triangle, "BlinnPhongVS");
	Engine::Ref<Engine::SimpleEntity> catWalkQuad = Engine::CreateRef<Engine::SimpleEntity>(Engine::PrimitiveType::Quad, "BlinnPhongVS");
	Engine::Ref<Engine::SimpleEntity> catWalkSphere = Engine::CreateRef<Engine::SimpleEntity>(Engine::PrimitiveType::Sphere, "BlinnPhongVS");

	m_CatWalkEntities.push_back({ catWalkCube, m_SmoothProperties, CatWalkSpin, { 0.0f, 2.0f, -15.0f } });
	m_CatWalkEntities.push_back({ catWalkTriangle, m_SmoothProperties, CatWalkSpin, { 0.0f, 2.0f, -15.0f } });
	m_CatWalkEntities.push_back({ catWalkQuad, m_SmoothProperties, CatWalkSpin, { 0.0f, 2.0f, -15.0f } });
	m_CatWalkEntities.push_back({ catWalkSphere, m_SmoothProperties, CatWalkScale, { 0.0f, 2.0f, -15.0f } });

	Engine::Ref<Engine::SimpleEntity> sdfGeometryA = Engine::CreateRef<Engine::SimpleEntity>(Engine::PrimitiveType::Cube, "SDFBlinnPhong");
	Engine::Ref<Engine::SimpleEntity> sdfGeometryB = Engine::CreateRef<Engine::SimpleEntity>(Engine::PrimitiveType::Cube, "SDFBlinnPhong");
	sdfGeometryB->GetEntityTransform()->SetRotation({ 0.0f, 180.0f, 0.0f });
	m_SDFEntities.push_back({ sdfGeometryA, m_SmoothProperties, {-3.0f, 2.0f, -10.0f} });
	m_SDFEntities.push_back({ sdfGeometryB, m_SmoothProperties, {3.0f, 2.0f, -10.0f} });

	m_DiscoBall->GetEntityTransform()->SetPosition({ 0.0f, 8.0f, -5.0f });
	m_Plane->GetEntityTransform()->SetScale({ 50.0f, 1.0f, 50.0f });
}

void BlinnPhongLightingDemoLayer::OnDetach()
{

}


static void UploadPositionalData(const Engine::Ref<Engine::SimpleEntity> entity, const Engine::Camera& camera)
{
	entity->GetEntityRenderer()->GetShader()->UploadUniformMat4("u_ModelMatrix", entity->GetEntityTransform()->Transform());
	entity->GetEntityRenderer()->GetShader()->UploadUniformMat4("u_ViewMatrix", camera.GetView());
	entity->GetEntityRenderer()->GetShader()->UploadUniformMat4("u_ProjectionMatrix", camera.GetProjection());
	glm::mat4 modelView = camera.GetView() * entity->GetEntityTransform()->Transform();
	glm::mat4 normalMatrix = glm::transpose(glm::inverse(modelView));
	entity->GetEntityRenderer()->GetShader()->UploadUniformMat4("u_NormalMatrix", normalMatrix);
}

static void UploadMaterialProperties(const Engine::Ref<Engine::SimpleEntity> entity, const BlinnPhongMaterialProperties& properties)
{
	entity->GetEntityRenderer()->GetShader()->Bind();
	entity->GetEntityRenderer()->GetShader()->UploadUniformFloat3("u_MaterialProperties.AmbientColor", properties.AmbientColor);
	entity->GetEntityRenderer()->GetShader()->UploadUniformFloat3("u_MaterialProperties.DiffuseColor", properties.DiffuseColor);
	entity->GetEntityRenderer()->GetShader()->UploadUniformFloat("u_MaterialProperties.AmbientStrength", properties.AmbientStrength);
	entity->GetEntityRenderer()->GetShader()->UploadUniformFloat("u_MaterialProperties.DiffuseStrength", properties.DiffuseStrength);
	entity->GetEntityRenderer()->GetShader()->UploadUniformFloat("u_MaterialProperties.SpecularStrength", properties.SpecularStrength);
	entity->GetEntityRenderer()->GetShader()->UploadUniformFloat("u_MaterialProperties.Shininess", properties.Shininess);
	entity->GetEntityRenderer()->GetShader()->UploadUniformInt("u_IsSmooth", properties.IsSmooth);
}

static void UploadDirectionalLightData(const Engine::Ref<Engine::Light>& directionalLight, const Engine::Camera& camera, const Engine::Ref<Engine::Shader>& shader)
{
	shader->Bind();
	glm::vec3 lightViewSpacePosition = directionalLight->GetViewSpaceVector(camera.GetView());
	shader->UploadUniformFloat3("u_DirectionalLight.LightPosition", lightViewSpacePosition);
	shader->UploadUniformFloat3("u_DirectionalLight.LightColor", directionalLight->GetLightColor());
	shader->UploadUniformFloat("u_DirectionalLight.Intensity", directionalLight->GetLightIntensity());
}

static void UploadSpotLightData(uint32_t index, const Engine::Ref<Engine::Light>& spotLight, const glm::vec3& lightPosition, const glm::vec3& direction, const Engine::Ref<Engine::Shader>& shader)
{
	std::stringstream uniformNameStream;
	uniformNameStream << "u_SpotLights[" << index << "].";

	shader->Bind();
	shader->UploadUniformFloat3(uniformNameStream.str() + std::string("LightPosition"), lightPosition);
	shader->UploadUniformFloat3(uniformNameStream.str() + std::string("LightColor"), spotLight->GetLightColor());
	shader->UploadUniformFloat(uniformNameStream.str() + std::string("Intensity"), spotLight->GetLightIntensity());
	shader->UploadUniformFloat(uniformNameStream.str() + std::string("ConstantAttenuation"), spotLight->GetConstantAttenuation());
	shader->UploadUniformFloat(uniformNameStream.str() + std::string("LinearAttenuation"), spotLight->GetLinearAttenuation());
	shader->UploadUniformFloat(uniformNameStream.str() + std::string("QuadraticAttenuation"), spotLight->GetQuadraticAttenuation());
	shader->UploadUniformFloat3(uniformNameStream.str() + std::string("LightDirection"), direction);
	shader->UploadUniformFloat(uniformNameStream.str() + std::string("InnerCutOff"), spotLight->GetInnerCutOff());
	shader->UploadUniformFloat(uniformNameStream.str() + std::string("OuterCutOff"), spotLight->GetOuterCutOff());
}

static void UploadPointLight(uint32_t index, const Engine::Ref<Engine::Light>& pointLight, const glm::vec3& lightPosition, const Engine::Ref<Engine::Shader>& shader)
{
	std::stringstream uniformNameStream;
	uniformNameStream << "u_PointLights[" << index << "].";
	shader->Bind();
	shader->UploadUniformFloat3(uniformNameStream.str() + std::string("LightPosition"), lightPosition);
	shader->UploadUniformFloat3(uniformNameStream.str() + std::string("LightColor"), pointLight->GetLightColor());
	shader->UploadUniformFloat(uniformNameStream.str() + std::string("Intensity"), pointLight->GetLightIntensity());
	shader->UploadUniformFloat(uniformNameStream.str() + std::string("ConstantAttenuation"), pointLight->GetConstantAttenuation());
	shader->UploadUniformFloat(uniformNameStream.str() + std::string("LinearAttenuation"), pointLight->GetLinearAttenuation());
	shader->UploadUniformFloat(uniformNameStream.str() + std::string("QuadraticAttenuation"), pointLight->GetQuadraticAttenuation());
}

static void UploadPointLights(const std::vector<Engine::Ref<Engine::Light>>& pointLights, const Engine::Camera& camera, const Engine::Ref<Engine::Shader>& shader)
{
	for (uint32_t i = 0; i < pointLights.size(); i++)
	{
		std::stringstream uniformNameStream;
		uniformNameStream << "u_PointLights[" << i << "].";

		shader->Bind();
		glm::vec3 lightViewSpacePosition = pointLights[i]->GetViewSpaceVector(camera.GetView());
		shader->UploadUniformFloat3(uniformNameStream.str() + std::string("LightPosition"), lightViewSpacePosition);
		shader->UploadUniformFloat3(uniformNameStream.str() + std::string("LightColor"), pointLights[i]->GetLightColor());
		shader->UploadUniformFloat(uniformNameStream.str() + std::string("Intensity"), pointLights[i]->GetLightIntensity());
		shader->UploadUniformFloat(uniformNameStream.str() + std::string("ConstantAttenuation"), pointLights[i]->GetConstantAttenuation());
		shader->UploadUniformFloat(uniformNameStream.str() + std::string("LinearAttenuation"), pointLights[i]->GetLinearAttenuation());
		shader->UploadUniformFloat(uniformNameStream.str() + std::string("QuadraticAttenuation"), pointLights[i]->GetQuadraticAttenuation());
	}
}

void BlinnPhongLightingDemoLayer::OnUpdate(float deltaTime)
{
	m_Camera.Update(deltaTime);

	m_Counter += deltaTime;
	m_CatWalkCounter += deltaTime;

	Engine::RenderCommand::Clear(true, true);
	Engine::RenderCommand::ClearColor({ 0.1f, 0.1f, 0.1f, 1.0f });

	float x = cos(m_Counter) * 500.0f;
	float z = sin(m_Counter) * 500.0f;

	PerformCatwalkRoutine();
	UploadLights(x, z);
	DrawEntities();
	DrawSDFEntities();
	DrawDebugLights();
}

void BlinnPhongLightingDemoLayer::OnEvent(Engine::Event& event)
{
	m_Camera.OnEvent(event);

	Engine::EventDispatcher dispatcher(event);

	dispatcher.Dispatch<Engine::KeyPressedEvent>(BIND_FN(BlinnPhongLightingDemoLayer::OnKeyPressed));
}

void BlinnPhongLightingDemoLayer::PerformCatwalkRoutine()
{
	float percent = Engine::Clamp(m_CatWalkCounter / 5.0f, 0, 1);
	CatWalkData currentData = m_CatWalkEntities[m_CatWalkPointer];

	if (ActionComplete)
	{
		if (WalkTo(currentData.CatWalkEntity, { 0.0f, 2.0f, 0.0f }, currentData.StartingPosition, percent))
		{
			m_CatWalkPointer = m_CatWalkPointer >= m_CatWalkEntities.size() - 1 ? 0 : m_CatWalkPointer + 1;
			m_CatWalkCounter = 0.0f;
			WalkUpComplete = false;
			ActionComplete = false;
		}
	}
	else
	{
		if (WalkUpComplete)
		{
			if (currentData.Execute(percent))
			{
				m_CatWalkCounter = 0.0f;
				ActionComplete = true;
			}
		}
		else
		{
			if (WalkTo(currentData.CatWalkEntity, currentData.StartingPosition, { 0.0f, 2.0f, 0.0f }, percent))
			{
				m_CatWalkCounter = 0.0f;
				WalkUpComplete = true;
			}
		}
	}
}

void BlinnPhongLightingDemoLayer::UploadLights(float x, float z)
{
	{
		for (uint32_t i = 0; i < m_SpotLightCount; i++)
		{
			glm::vec3 focalPoint = m_SpotLightFocalPoints[i];

			float x = i % 2 == 0 ? focalPoint.x + cos(m_Counter * 2.0f) * -3.0f : focalPoint.x + cos(-m_Counter * 2.0f) * 3.0f;
			float z = i % 2 == 0 ? focalPoint.z + sin(m_Counter * 2.0f) * 3.0f : focalPoint.z + sin(-m_Counter * 2.0f) * 3.0f;

			glm::vec3 newPoint{ x, focalPoint.y, z };
			m_SpotLights[i]->GetLightTransform()->LookAt(newPoint);
			m_SpotLights[i]->SetLightDirection(m_SpotLights[i]->GetLightTransform()->Forward());
		}

		glm::vec3 SunLightSpin = { x, 500.0f, z };
		m_DirectionalLight->GetLightTransform()->SetPosition(SunLightSpin);

		Engine::ShaderLibrary::Get("BlinnPhongVS")->Bind();
		Engine::ShaderLibrary::Get("BlinnPhongVS")->UploadUniformInt("u_UseDirectionalLight", m_UseDirectional ? 1 : 0);
		Engine::ShaderLibrary::Get("BlinnPhongVS")->UploadUniformInt("u_UsePointLights", m_UsePointLights ? 1 : 0);
		Engine::ShaderLibrary::Get("BlinnPhongVS")->UploadUniformInt("u_UseSpotLights", m_UseSpotLights ? 1 : 0);


		UploadDirectionalLightData(m_DirectionalLight, m_Camera, Engine::ShaderLibrary::Get("BlinnPhongVS"));

		if (m_MovePointLights)
		{
			for (uint32_t i = 0; i < m_PointLightCount; i++)
			{
				glm::vec3 centerPoint = m_PointLightPositions[i];

				float x = i % 2 == 0 ? centerPoint.x + cos(m_Counter * 2.0f) * -3.0f : centerPoint.x + cos(-m_Counter * 2.0f) * 3.0f;
				float z = i % 2 == 0 ? centerPoint.z + sin(m_Counter * 2.0f) * 3.0f : centerPoint.z + sin(-m_Counter * 2.0f) * 3.0f;

				glm::vec3 newPoint{ x, centerPoint.y, z };
				m_PointLights[i]->GetLightTransform()->SetPosition(newPoint);
			}
		}

		for (uint32_t i = 0; i < m_PointLightCount; i++)
		{
			glm::vec3 lightViewSpacePosition = m_PointLights[i]->GetViewSpaceVector(m_Camera.GetView());
			UploadPointLight(i, m_PointLights[i], lightViewSpacePosition, Engine::ShaderLibrary::Get("BlinnPhongVS"));
		}

		for (uint32_t i = 0; i < m_SpotLightCount; i++)
		{
			glm::vec3 lightSpacePosition = m_SpotLights[i]->GetViewSpaceVector(m_Camera.GetView());
			glm::vec3 lightViewSpaceDirection = glm::vec3(m_Camera.GetView() * glm::vec4(m_SpotLights[i]->GetLightDirection(), 0.0f));
			UploadSpotLightData(i, m_SpotLights[i], lightSpacePosition, lightViewSpaceDirection, Engine::ShaderLibrary::Get("BlinnPhongVS"));
		}
	}
}

void BlinnPhongLightingDemoLayer::DrawEntities()
{
	{
		m_DiscoBall->GetEntityTransform()->SetRotation({ 0.0f, m_Counter * 80.0f, 0.0f });

		for (uint32_t i = 0; i < m_CatWalkEntities.size(); i++)
		{
			auto entity = m_CatWalkEntities[i].CatWalkEntity;
			entity->GetEntityRenderer()->GetShader()->Bind();
			m_WhiteTexture->BindToSamplerSlot(0);
			UploadPositionalData(entity, m_Camera);
			UploadMaterialProperties(entity, m_CatWalkEntities[i].MaterialProperties);
			entity->DrawEntity(m_Camera.GetViewProjection());
		}

		m_DiscoBall->GetEntityRenderer()->GetShader()->Bind();
		m_WhiteTexture->BindToSamplerSlot(0);
		UploadPositionalData(m_DiscoBall, m_Camera);
		UploadMaterialProperties(m_DiscoBall, m_FlatShadedShinyProperties);
		m_DiscoBall->GetEntityRenderer()->GetShader()->Bind();
		m_DiscoBall->GetEntityRenderer()->GetShader()->UploadUniformInt("u_Texture", 0);
		m_DiscoBall->DrawEntity(m_Camera.GetViewProjection());

		m_Plane->GetEntityRenderer()->GetShader()->Bind();

		if (m_GroundPlaneHasTexture)
			m_GroundTexture->BindToSamplerSlot(0);
		else
			m_WhiteTexture->BindToSamplerSlot(0);
		UploadPositionalData(m_Plane, m_Camera);
		UploadMaterialProperties(m_Plane, m_SmoothProperties);
		m_Plane->GetEntityRenderer()->GetShader()->Bind();
		m_Plane->GetEntityRenderer()->GetShader()->UploadUniformInt("u_Texture", 0);
		m_Plane->DrawEntity(m_Camera.GetViewProjection());
		m_WhiteTexture->BindToSamplerSlot(0);

		m_Wall->GetEntityRenderer()->GetShader()->Bind();
		m_WallTexture->BindToSamplerSlot(0);
		UploadPositionalData(m_Wall, m_Camera);
		UploadMaterialProperties(m_Wall, m_SmoothProperties);
		m_Wall->GetEntityRenderer()->GetShader()->Bind();
		m_Wall->GetEntityRenderer()->GetShader()->UploadUniformInt("u_Texture", 0);
		m_Wall->DrawEntity(m_Camera.GetViewProjection());
		m_WhiteTexture->BindToSamplerSlot(0);

		m_DisplayCube->GetEntityRenderer()->GetShader()->Bind();
		m_WhiteTexture->BindToSamplerSlot(0);
		UploadPositionalData(m_DisplayCube, m_Camera);
		UploadMaterialProperties(m_DisplayCube, m_SmoothProperties);
		m_DisplayCube->GetEntityRenderer()->GetShader()->Bind();
		m_DisplayCube->GetEntityRenderer()->GetShader()->UploadUniformInt("u_Texture", 0);
		m_DisplayCube->DrawEntity(m_Camera.GetViewProjection());
		m_WhiteTexture->BindToSamplerSlot(0);

		m_DisplaySphere->GetEntityRenderer()->GetShader()->Bind();
		m_WhiteTexture->BindToSamplerSlot(0);
		UploadPositionalData(m_DisplaySphere, m_Camera);
		UploadMaterialProperties(m_DisplaySphere, m_RoughProperties);
		m_DisplaySphere->GetEntityRenderer()->GetShader()->Bind();
		m_DisplaySphere->GetEntityRenderer()->GetShader()->UploadUniformInt("u_Texture", 0);
		m_DisplaySphere->DrawEntity(m_Camera.GetViewProjection());
		m_WhiteTexture->BindToSamplerSlot(0);

		m_TexturedDisplaySphere->GetEntityRenderer()->GetShader()->Bind();
		m_MapTexture->BindToSamplerSlot(0);
		UploadPositionalData(m_TexturedDisplaySphere, m_Camera);
		UploadMaterialProperties(m_TexturedDisplaySphere, m_SmoothProperties);
		m_TexturedDisplaySphere->GetEntityRenderer()->GetShader()->Bind();
		m_TexturedDisplaySphere->GetEntityRenderer()->GetShader()->UploadUniformInt("u_Texture", 0);
		m_TexturedDisplaySphere->DrawEntity(m_Camera.GetViewProjection());
		m_WhiteTexture->BindToSamplerSlot(0);
	}
}

void BlinnPhongLightingDemoLayer::DrawSDFEntities()
{
	m_SphereOffset = { 0.0f, sin(m_Counter * 0.9f) * 0.4f, 0.0f };


	for (uint32_t i = 0; i < m_SDFEntities.size(); i++)
	{
		auto entity = m_SDFEntities[i].SceneSDFEntity;
		glm::mat4 worldToModel = glm::inverse(entity->GetEntityTransform()->Transform());

		UploadDirectionalLightData(m_DirectionalLight, m_Camera, Engine::ShaderLibrary::Get("SDFBlinnPhong"));

		glm::vec3 directionalModelSpacePosition = glm::vec3(worldToModel * glm::vec4(m_DirectionalLight->GetLightTransform()->GetPosition(), 1.0f));
		Engine::ShaderLibrary::Get("SDFBlinnPhong")->UploadUniformFloat3("u_DirectionalLight.LightPosition", directionalModelSpacePosition);

		for (uint32_t i = 0; i < m_PointLightCount; i++)
		{
			glm::vec3 lightModelSpacePosition = glm::vec3(worldToModel * glm::vec4(m_PointLights[i]->GetLightTransform()->GetPosition(), 1.0f));
			UploadPointLight(i, m_PointLights[i], lightModelSpacePosition, Engine::ShaderLibrary::Get("SDFBlinnPhong"));
		}

		for (uint32_t i = 0; i < m_SpotLightCount; i++)
		{
			glm::vec3 spotLightModelSpacePosition = glm::vec3(worldToModel * glm::vec4(m_SpotLights[i]->GetLightTransform()->GetPosition(), 1.0f));
			glm::vec3 lightViewSpaceDirection = glm::vec3(m_Camera.GetView() * glm::vec4(m_SpotLights[i]->GetLightDirection(), 0.0f));
			UploadSpotLightData(i, m_SpotLights[i], spotLightModelSpacePosition, m_SpotLights[i]->GetLightDirection(), Engine::ShaderLibrary::Get("SDFBlinnPhong"));
		}

		entity->GetEntityRenderer()->GetShader()->Bind();
		m_WhiteTexture->BindToSamplerSlot(0);
		glm::vec3 camModelSpace = glm::vec3(worldToModel * glm::vec4(m_Camera.GetPosition(), 1.0f));
		entity->GetEntityRenderer()->GetShader()->UploadUniformInt("u_UseDirectionalLight", m_UseDirectional ? 1 : 0);
		entity->GetEntityRenderer()->GetShader()->UploadUniformInt("u_UsePointLights", m_UsePointLights ? 1 : 0);
		entity->GetEntityRenderer()->GetShader()->UploadUniformInt("u_UseSpotLights", m_UseSpotLights ? 1 : 0);
		entity->GetEntityRenderer()->GetShader()->UploadUniformMat4("u_ModelMatrix", entity->GetEntityTransform()->Transform());
		entity->GetEntityRenderer()->GetShader()->UploadUniformFloat3("u_CameraPosition", camModelSpace);
		entity->GetEntityRenderer()->GetShader()->UploadUniformFloat3("u_CameraPositionWorld", m_Camera.GetPosition());
		entity->GetEntityRenderer()->GetShader()->UploadUniformFloat3("u_SphereOffset", m_SphereOffset);
		entity->GetEntityRenderer()->GetShader()->UploadUniformFloat("u_SmoothUnionCoefficient", 0.5f);
		UploadMaterialProperties(entity, m_SDFEntities[i].MaterialProperties);
		entity->DrawEntity(m_Camera.GetViewProjection());
	}
}

void BlinnPhongLightingDemoLayer::DrawDebugLights()
{
	{
		if (!m_RenderDebugLights) return;
		for (uint32_t i = 0; i < m_PointLights.size(); i++)
		{
			m_PointLights[i]->GetDebugRenderer()->GetShader()->Bind();
			m_PointLights[i]->GetDebugRenderer()->GetShader()->UploadUniformFloat3("u_Color", m_PointLights[i]->GetLightColor());
			m_PointLights[i]->DrawDebug(m_Camera.GetViewProjection());
		}

		m_DirectionalLight->GetDebugRenderer()->GetShader()->Bind();
		m_DirectionalLight->GetDebugRenderer()->GetShader()->UploadUniformFloat3("u_Color", m_DirectionalLight->GetLightColor());
		m_DirectionalLight->DrawDebug(m_Camera.GetViewProjection());

		for (uint32_t i = 0; i < m_SpotLights.size(); i++)
		{
			m_SpotLights[i]->GetDebugRenderer()->GetShader()->Bind();
			m_SpotLights[i]->GetDebugRenderer()->GetShader()->UploadUniformFloat3("u_Color", m_SpotLights[i]->GetLightColor());
			m_SpotLights[i]->DrawDebug(m_Camera.GetViewProjection());
		}
	}
}

bool BlinnPhongLightingDemoLayer::OnKeyPressed(Engine::KeyPressedEvent& event)
{
	if (event.GetKeyCode() == Engine::Key::Space)
	{
		for (uint32_t i = 0; i < m_PointLightCount; i++)
		{
			Engine::LightSpecification pointLightSpec =
			{
				Engine::LightType::Point,
				{Engine::Random::RandomRange(0.0f, 1.0f), Engine::Random::RandomRange(0.0f, 1.0f), Engine::Random::RandomRange(0.0f, 1.0f)},
				1.0f,
				Engine::Random::RandomRange(1.0f, 10.0f),
				Engine::Random::RandomRange(0.0f, 0.00000001f),
				0.00000000001f,
			};

			glm::vec3 position{ Engine::Random::RandomRange(-20.0f, 20.0f), Engine::Random::RandomRange(1.0f, 6.0f), Engine::Random::RandomRange(-20.0f, 20.0f) };
			m_PointLightPositions[i] = position;
			m_PointLights[i]->SetSpecification(pointLightSpec);
			m_PointLights[i]->GetLightTransform()->SetPosition(position);
		}

		for (uint32_t i = 0; i < m_SpotLights.size(); i++)
		{
			m_SpotLights[i]->SetLightColor({ Engine::Random::RandomRange(0.0f, 1.0f), Engine::Random::RandomRange(0.0f, 1.0f), Engine::Random::RandomRange(0.0f, 1.0f) });
		}

	}

	if (event.GetKeyCode() == Engine::Key::LeftShift)
		m_RenderDebugLights = !m_RenderDebugLights;

	if (event.GetKeyCode() == Engine::Key::RightShift)
		m_MovePointLights = !m_MovePointLights;

	if (event.GetKeyCode() == Engine::Key::Tab)
		m_GroundPlaneHasTexture = !m_GroundPlaneHasTexture;

	if (event.GetKeyCode() == Engine::Key::D1)
		m_UseDirectional = !m_UseDirectional;
	if (event.GetKeyCode() == Engine::Key::D2)
		m_UsePointLights = !m_UsePointLights;
	if (event.GetKeyCode() == Engine::Key::D3)
		m_UseSpotLights = !m_UseSpotLights;

	return false;
}
