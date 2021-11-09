#include "ShadowsLayer.h"

#include <imgui/imgui.h>
#include <glad/glad.h>

ShadowsLayer::ShadowsLayer()
{

}

ShadowsLayer::~ShadowsLayer()
{

}

void ShadowsLayer::OnAttach()
{
	m_Scene = Engine::CreateRef<Engine::Scene>("Test Scene");
	
	m_Cube = m_Scene->Create("Cube");
	m_Cube.AddComponent<Engine::MeshRendererComponent>(Engine::PrimitiveType::Cube, m_CubeProperties);
	m_Cube.GetComponent<Engine::TransformComponent>().Translation = { 3.0f, 0.0f, 0.0f };

	m_Sphere = m_Scene->Create("Entity A");
	m_Sphere.AddComponent<Engine::MeshRendererComponent>(Engine::PrimitiveType::Sphere, m_SphereProperties);

	m_Plane = m_Scene->Create("Entity B");
	m_Plane.AddComponent<Engine::MeshRendererComponent>(Engine::PrimitiveType::Plane, m_PlaneProperties);
	m_Plane.GetComponent<Engine::TransformComponent>().Translation = { 0.0f, -2.0f, 0.0f };
	m_Plane.GetComponent<Engine::TransformComponent>().Scale = { 15.0f, 1.0f, 15.0f };
	
	m_Light = m_Scene->Create("Directional Light");
	m_Light.AddComponent<Engine::LightComponent>(Engine::EngineLightType::Point);
	m_Light.GetComponent<Engine::TransformComponent>().Translation = { 2.0f, 3.0f, 2.0f };
	m_Light.GetComponent<Engine::LightComponent>().Intensity = 0.7f;
	m_Scene->RegisterDirectionalLight(m_Light);

	Engine::SceneRenderer::LoadScene(m_Scene);
	Engine::SceneRenderer::InitializePipeline();
}

void ShadowsLayer::OnDetach()
{
	Engine::SceneRenderer::UnloadScene();
}

void ShadowsLayer::OnUpdate(float deltaTime)
{
	Engine::SceneRenderer::UpdateCamera(deltaTime);
	Engine::SceneRenderer::SubmitPipeline();
}


void ShadowsLayer::OnImGuiRender()
{
	ImGui::Begin("Scene Properties");

	Engine::Application::GetApplication().GetImGuiLayer()->BlockEvents(ImGui::IsWindowFocused() && ImGui::IsWindowHovered());

	if (ImGui::TreeNode("Sphere"))
	{
		Engine::MeshRendererComponent& sphereMeshRenderer = m_Sphere.GetComponent<Engine::MeshRendererComponent>();
		Engine::TransformComponent& sphereTransform = m_Sphere.GetComponent<Engine::TransformComponent>();

		ImGui::DragFloat3("Sphere Position", &sphereTransform.Translation.x);

		ImGui::PushItemWidth(150.0f);
		ImGui::ColorPicker3("Sphere Ambient Color", &sphereMeshRenderer.Properties.AmbientColor.x);
		ImGui::PopItemWidth();
		ImGui::DragFloat("Sphere Ambient Strength", &sphereMeshRenderer.Properties.AmbientStrength, 0.05f, 0.0f, 1.0f);

		ImGui::PushItemWidth(150.0f);
		ImGui::ColorPicker3("Sphere Diffuse Color", &sphereMeshRenderer.Properties.DiffuseColor.x);
		ImGui::PopItemWidth();
		ImGui::DragFloat("Sphere Diffuse Strength", &sphereMeshRenderer.Properties.DiffuseStrength, 0.05f, 0.0f, 1.0f);


		ImGui::DragFloat("Sphere Specular Strength", &sphereMeshRenderer.Properties.SpecularStrength, 0.05f, 0.0f, 1.0f);
		ImGui::DragFloat("Sphere Shininess", &sphereMeshRenderer.Properties.Shininess, 0.05f, 2.0f, 256.0f);
		ImGui::TreePop();
		ImGui::Separator();
	}

	if (ImGui::TreeNode("Plane"))
	{
		Engine::TransformComponent& planeTransform = m_Plane.GetComponent<Engine::TransformComponent>();

		ImGui::DragFloat3("Plane Position", &planeTransform.Translation.x);

		Engine::MeshRendererComponent& planeMeshRenderer = m_Plane.GetComponent<Engine::MeshRendererComponent>();
		ImGui::PushItemWidth(150.0f);
		ImGui::ColorPicker3("Plane Ambient Color", &planeMeshRenderer.Properties.AmbientColor.x);
		ImGui::PopItemWidth();
		ImGui::DragFloat("Plane Ambient Strength", &planeMeshRenderer.Properties.AmbientStrength, 0.05f, 0.0f, 1.0f);

		ImGui::PushItemWidth(150.0f);
		ImGui::ColorPicker3("Plane Diffuse Color", &planeMeshRenderer.Properties.DiffuseColor.x);
		ImGui::PopItemWidth();
		ImGui::DragFloat("Plane Diffuse Strength", &planeMeshRenderer.Properties.DiffuseStrength, 0.05f, 0.0f, 1.0f);

		ImGui::DragFloat("Plane Specular Strength", &planeMeshRenderer.Properties.SpecularStrength, 0.05f, 0.0f, 1.0f);
		ImGui::DragFloat("Plane Shininess", &planeMeshRenderer.Properties.Shininess, 0.05f, 2.0f, 256.0f);

		ImGui::TreePop();
		ImGui::Separator();
	}

	if (ImGui::TreeNode("Cube"))
	{
		Engine::TransformComponent& cubeTransform = m_Plane.GetComponent<Engine::TransformComponent>();

		ImGui::DragFloat3("Cube Position", &cubeTransform.Translation.x);

		Engine::MeshRendererComponent& cubeMeshRenderer = m_Cube.GetComponent<Engine::MeshRendererComponent>();
		ImGui::PushItemWidth(150.0f);
		ImGui::ColorPicker3("Cube Ambient Color", &cubeMeshRenderer.Properties.AmbientColor.x);
		ImGui::PopItemWidth();
		ImGui::DragFloat("Cube Ambient Strength", &cubeMeshRenderer.Properties.AmbientStrength, 0.05f, 0.0f, 1.0f);

		ImGui::PushItemWidth(150.0f);
		ImGui::ColorPicker3("Cube Diffuse Color", &cubeMeshRenderer.Properties.DiffuseColor.x);
		ImGui::PopItemWidth();
		ImGui::DragFloat("Cube Diffuse Strength", &cubeMeshRenderer.Properties.DiffuseStrength, 0.05f, 0.0f, 1.0f);

		ImGui::DragFloat("Cube Specular Strength", &cubeMeshRenderer.Properties.SpecularStrength, 0.05f, 0.0f, 1.0f);
		ImGui::DragFloat("Cube Shininess", &cubeMeshRenderer.Properties.Shininess, 0.05f, 2.0f, 256.0f);

		ImGui::TreePop();
		ImGui::Separator();
	}

	if (ImGui::TreeNode("Light"))
	{
		Engine::TransformComponent& lightTransform = m_Light.GetComponent<Engine::TransformComponent>();
		Engine::LightComponent& light = m_Light.GetComponent<Engine::LightComponent>();

		ImGui::DragFloat3("Light Position", &lightTransform.Translation.x);
		ImGui::PushItemWidth(150.0f);
		ImGui::ColorPicker3("Light Color", &light.Color.x);
		ImGui::PopItemWidth();
		ImGui::TreePop();
		ImGui::Separator();
	}


	ImGui::End();
}

void ShadowsLayer::OnEvent(Engine::Event& event)
{
	Engine::EventDispatcher dispatcher(event);

	Engine::SceneRenderer::OnEvent(event);

	dispatcher.Dispatch<Engine::KeyPressedEvent>(BIND_FN(ShadowsLayer::OnKeyPressed));
	dispatcher.Dispatch<Engine::WindowResizedEvent>(BIND_FN(ShadowsLayer::OnResize));
}

bool ShadowsLayer::OnKeyPressed(Engine::KeyPressedEvent& keyPressedEvent)
{
	return false;
}

bool ShadowsLayer::OnResize(Engine::WindowResizedEvent& windowResizeEvent)
{
	return false;
}

