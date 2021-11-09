#include "ToonShadingLayer.h"

ToonShadingLayer::ToonShadingLayer()
	:m_Camera(45.0f, 1920.0f / 1080.0f, 0.1f, 1000.0f)
{

}

ToonShadingLayer::~ToonShadingLayer()
{

}

void ToonShadingLayer::OnAttach()
{
	m_WhiteTexture = Engine::Texture2D::CreateWhiteTexture();
	m_Entity = Engine::CreateRef<Engine::SimpleEntity>(Engine::PrimitiveType::Sphere, "ToonShader");
	m_Plane = Engine::CreateRef<Engine::SimpleEntity>(Engine::PrimitiveType::Plane, "FlatColor");
	m_Plane->GetEntityTransform()->SetPosition({ 0.0f, -2.0f, 0.0f });
	m_Plane->GetEntityTransform()->SetScale({ 20.0f, 1.0f, 20.0f });

	Engine::LightSpecification lightSpec =
	{
		Engine::LightType::Point,
		{0.4f, 0.4f, 0.4f},
		1.0f
	};

	m_PointLight = Engine::CreateRef<Engine::Light>(lightSpec);
	m_PointLight->GetLightTransform()->SetPosition({ 2.0f, 1.0f, 4.0f });
}

void ToonShadingLayer::OnDetach()
{

}

void ToonShadingLayer::OnUpdate(float deltaTime)
{
	Engine::RenderCommand::Clear(true, true);
	Engine::RenderCommand::ClearColor(m_ClearColor);

	m_Camera.Update(deltaTime);

	float x = cos(Engine::Time::Elapsed()) * 5.0f;
	float z = sin(Engine::Time::Elapsed()) * 5.0f;

	m_PointLight->GetLightTransform()->SetPosition({ x, m_LightOrigin.y, z });

	m_WhiteTexture->BindToSamplerSlot(0);
	m_Entity->GetEntityRenderer()->GetShader()->Bind();
	m_Entity->GetEntityRenderer()->GetShader()->UploadUniformFloat("u_Bias", 0.3f);
	m_Entity->GetEntityRenderer()->GetShader()->UploadUniformFloat("u_Scale", 1.0f);
	m_Entity->GetEntityRenderer()->GetShader()->UploadUniformFloat("u_Power", 2.0f);
	m_Entity->GetEntityRenderer()->GetShader()->UploadUniformMat4("u_ModelMatrix", m_Entity->GetEntityTransform()->Transform());
	m_Entity->GetEntityRenderer()->GetShader()->UploadUniformFloat3("u_CameraPosition", m_Camera.GetPosition());

	m_Entity->GetEntityRenderer()->GetShader()->UploadUniformFloat3("u_Light.Position", m_PointLight->GetLightTransform()->GetPosition());
	m_Entity->GetEntityRenderer()->GetShader()->UploadUniformFloat3("u_Light.Color", m_PointLight->GetLightColor());
	m_Entity->GetEntityRenderer()->GetShader()->UploadUniformFloat("u_Light.Intensity", m_PointLight->GetLightIntensity());


	m_Entity->GetEntityRenderer()->GetShader()->UploadUniformFloat3("u_DiffuseColor", { 0.0f, 0.8f, 0.9});
	m_Entity->GetEntityRenderer()->GetShader()->UploadUniformFloat3("u_FresnelColor", { 1.0f, 0.3f, 1.0f });
	m_Entity->GetEntityRenderer()->GetShader()->UploadUniformFloat3("u_SpecularColor", { 1.0f, 0.5f, 1.0f });
	m_Entity->GetEntityRenderer()->GetShader()->UploadUniformFloat3("u_AmbientColor", { 0.4, 0.4, 0.4});
	m_Entity->GetEntityRenderer()->GetShader()->UploadUniformFloat3("u_RimColor", { 1.0f, 1.0f, 1.0f });
	m_Entity->GetEntityRenderer()->GetShader()->UploadUniformFloat("u_RimAmount", 0.716f);
	m_Entity->GetEntityRenderer()->GetShader()->UploadUniformFloat("u_RimThreshold", 0.1f);
	m_Entity->GetEntityRenderer()->GetShader()->UploadUniformFloat("u_SpecularStrength", 1.0f);
	m_Entity->GetEntityRenderer()->GetShader()->UploadUniformFloat("u_Shininess", 32.0f);
	m_Entity->GetEntityRenderer()->GetShader()->UploadUniformInt("u_Texture", 0);

	m_Entity->DrawEntity(m_Camera.GetViewProjection());

	m_Plane->GetEntityRenderer()->GetShader()->Bind();
	m_Plane->GetEntityRenderer()->GetShader()->UploadUniformFloat3("u_Color", {1.0f, 1.0f, 1.0f});
	m_Plane->DrawEntity(m_Camera.GetViewProjection());

	m_PointLight->DrawDebug(m_Camera.GetViewProjection());
}

void ToonShadingLayer::OnEvent(Engine::Event& event)
{
	m_Camera.OnEvent(event);

	Engine::EventDispatcher dispatcher(event);

	dispatcher.Dispatch<Engine::KeyPressedEvent>(BIND_FN(ToonShadingLayer::OnKeyPressed));
}

bool ToonShadingLayer::OnKeyPressed(Engine::KeyPressedEvent& keyPressedEvent)
{
	return true;
}
