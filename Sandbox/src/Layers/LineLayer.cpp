#include "Layers/LineLayer.h"
#include <vector>
#include <iostream>
#include <imgui/imgui.h>
#define PI 3.14159265358979

#include <glad/glad.h>

LineLayer::LineLayer()
{

}

LineLayer::~LineLayer()
{

}

void LineLayer::OnAttach()
{
	Engine::Application::GetApplication().GetWindow().ToggleMaximize(true);
	//m_Camera.SetOrthographic();

	m_WhiteTexture = Engine::Texture2D::CreateWhiteTexture();
	m_BloomDirtTexture = Engine::Texture2D::CreateBlackTexture();
	m_ViewportWidth = Engine::Application::GetApplication().GetWindow().GetWidth();
	m_ViewportHeight = Engine::Application::GetApplication().GetWindow().GetHeight();

	Engine::FramebufferSpecification fboSpec
	{
		m_ViewportWidth, m_ViewportHeight,
		{ Engine::FramebufferTextureFormat::RGBA32F, Engine::FramebufferTextureFormat::Depth }
	};
	m_FBO = Engine::CreateRef<Engine::Framebuffer>(fboSpec);

	m_BloomComputeTextures.resize(3);

	Engine::Texture2DSpecification bloomSpec =
	{
		Engine::ImageUtils::WrapMode::ClampToEdge,
		Engine::ImageUtils::WrapMode::ClampToEdge,
		Engine::ImageUtils::FilterMode::Linear,
		Engine::ImageUtils::FilterMode::Linear,
		Engine::ImageUtils::ImageInternalFormat::RGBA32F,
		Engine::ImageUtils::ImageDataLayout::RGBA,
		Engine::ImageUtils::ImageDataType::Float,
		1, 1
	};

	m_BloomComputeTextures[0] = Engine::CreateRef<Engine::Texture2D>(bloomSpec);
	m_BloomComputeTextures[1] = Engine::CreateRef<Engine::Texture2D>(bloomSpec);
	m_BloomComputeTextures[2] = Engine::CreateRef<Engine::Texture2D>(bloomSpec);

	std::vector<Engine::LineVertex> vertices;
	m_Curve = Engine::CreateRef<Engine::BezierCurve>(glm::vec3(0.0f, 0.0f, 0.0f));
	m_Curve->SetPointSize(10.0f);
	m_Curve->SetControlLineColor({0.0f, 1.0f, 0.0f});

	for (auto lineVertex : *m_Curve)
		m_BaseCurveVertices.push_back(lineVertex);

	m_FullScreenQuad = Engine::CreateRef<Engine::SimpleEntity>(Engine::PrimitiveType::FullScreenQuad, "PostProcessing");
	m_Cube = Engine::CreateRef<Engine::SimpleEntity>(Engine::PrimitiveType::Cube, "BlinnPhongWS");


	Engine::LightSpecification sunSpec =
	{
		Engine::LightType::Directional,
		{1.0f, 1.0f, 1.0f},
		1.0f
	};

	m_Light = Engine::CreateRef<Engine::Light>(sunSpec);
	m_Light->GetLightTransform()->SetPosition({ 10.0f, 50.0f, 10.0f });


	OnResize();
}

void LineLayer::OnDetach()
{

}

static float Fract(float f)
{
	return f - (long)f;
}
static float Rand(float n) { return Fract(sin(n) * 43758.5453); }


void LineLayer::OnUpdate(float deltaTime)
{
	uint32_t width = Engine::Application::GetApplication().GetWindow().GetWidth();
	uint32_t height = Engine::Application::GetApplication().GetWindow().GetHeight();
	if (m_ViewportWidth != width || m_ViewportHeight != height)
	{
		m_ViewportWidth = width;
		m_ViewportHeight = height;

		OnResize();
	}

	m_Camera.Update(deltaTime);
	glm::vec2 mousePosition = Engine::Input::GetMousePosition();
	glm::vec3 worldCoordinate = m_Camera.ScreenToWorldPoint({ mousePosition.x, mousePosition.y, 0.0f });

	if (!m_Animate &&  m_IsDragging && m_DragID != -1)
	{
		Engine::LineVertex dragPoint = (*m_Curve)[m_DragID];
		worldCoordinate.z = 0.0;
		
		if (glm::vec2(worldCoordinate) != glm::vec2(dragPoint.Position))
			m_Curve->MovePoint(m_DragID, worldCoordinate);

		m_BaseCurveVertices.clear();
		for (auto lineVertex : *m_Curve)
			m_BaseCurveVertices.push_back(lineVertex);
	}

	if (m_Animate)
	{
		float elapsed = Engine::Time::Elapsed();
		uint32_t i = 0;
		for (auto curveVertex : *m_Curve)
		{
			if (i % 3 == 1 || i % 3 == 2)
			{
				glm::vec2 randomOffset = { Rand(m_BaseCurveVertices[i].Position.x), Rand(m_BaseCurveVertices[i].Position.y) };
				glm::vec3 current = m_BaseCurveVertices[i].Position;
				glm::vec3 newPosition = current + glm::vec3(cos(elapsed * randomOffset.x), sin(elapsed * randomOffset.y), 0.0f);
				m_Curve->MovePoint(i, newPosition);
			}

			i++;
		}
	}


	m_FBO->Bind();
	Engine::RenderCommand::ClearColor(m_ClearColor);
	Engine::RenderCommand::Clear(true, true);
	m_Curve->SetControlLineColor(m_LineColor);
	m_Curve->Draw(m_Camera.GetViewProjection());

	m_Cube->GetEntityRenderer()->GetShader()->Bind();
	m_WhiteTexture->BindToSamplerSlot(0);
	m_Cube->GetEntityRenderer()->GetShader()->UploadUniformMat4("u_ModelMatrix", m_Cube->GetEntityTransform()->Transform());
	m_Cube->GetEntityRenderer()->GetShader()->UploadUniformInt("u_Texture", 0);
	m_Cube->GetEntityRenderer()->GetShader()->UploadUniformFloat3("u_MaterialProperties.AmbientColor", m_SphereProperties.AmbientColor);
	m_Cube->GetEntityRenderer()->GetShader()->UploadUniformFloat3("u_MaterialProperties.DiffuseColor", m_SphereProperties.DiffuseColor);
	m_Cube->GetEntityRenderer()->GetShader()->UploadUniformFloat("u_MaterialProperties.AmbientStrength", m_SphereProperties.AmbientStrength);
	m_Cube->GetEntityRenderer()->GetShader()->UploadUniformFloat("u_MaterialProperties.DiffuseStrength", m_SphereProperties.DiffuseStrength);
	m_Cube->GetEntityRenderer()->GetShader()->UploadUniformFloat("u_MaterialProperties.SpecularStrength", m_SphereProperties.SpecularStrength);
	m_Cube->GetEntityRenderer()->GetShader()->UploadUniformFloat("u_MaterialProperties.Shininess", m_SphereProperties.Shininess);
	m_Cube->GetEntityRenderer()->GetShader()->UploadUniformFloat3("u_Light.Position", m_Light->GetLightTransform()->GetPosition());
	m_Cube->GetEntityRenderer()->GetShader()->UploadUniformFloat("u_Light.Intensity", m_Light->GetLightIntensity());
	m_Cube->GetEntityRenderer()->GetShader()->UploadUniformFloat3("u_Light.Color", m_Light->GetLightColor());
	m_Cube->GetEntityRenderer()->GetShader()->UploadUniformFloat3("u_CameraPosition", m_Camera.GetPosition());
	m_Cube->DrawEntity(m_Camera.GetViewProjection());
	m_FBO->Unbind();

	BloomComputePass();

	Engine::RenderCommand::Clear(true, true);
	m_FullScreenQuad->GetEntityRenderer()->GetShader()->Bind();
	m_FullScreenQuad->GetEntityRenderer()->GetShader()->UploadUniformFloat("u_Exposure", m_Exposure);
	m_FullScreenQuad->GetEntityRenderer()->GetShader()->UploadUniformFloat("u_BloomIntensity", m_BloomEnabled ? m_BloomIntensity : 0.0f);
	m_FullScreenQuad->GetEntityRenderer()->GetShader()->UploadUniformFloat("u_BloomDirtIntensity", m_BloomEnabled ? m_BloomDirtIntensity : 0.0f);

	m_FBO->BindColorAttachment(0, 0);
	m_BloomComputeTextures[2]->BindToSamplerSlot(1);
	m_BloomDirtTexture->BindToSamplerSlot(2);
	m_FullScreenQuad->GetEntityRenderer()->GetShader()->UploadUniformInt("u_Texture", 0);
	m_FullScreenQuad->GetEntityRenderer()->GetShader()->UploadUniformInt("u_BloomTexture", 1);
	m_FullScreenQuad->GetEntityRenderer()->GetShader()->UploadUniformInt("u_BloomDirtTexture", 2);
	m_FullScreenQuad->DrawEntity(m_Camera.GetViewProjection());
	m_BloomComputeTextures[2]->Unbind();
	m_FBO->UnbindColorAttachment(0, 0);
}

void LineLayer::BloomComputePass()
{
	Engine::ShaderLibrary::Get("Bloom2")->Bind();

	struct BloomConstants
	{
		glm::vec4 Params;
		float LOD = 0.0f;
		int Mode = 0;
	} bloomConstants;

	bloomConstants.Params = { m_BloomThreshold, m_BloomThreshold - m_BloomKnee, m_BloomKnee * 2.0f, 0.25f / m_BloomKnee };

	//------------------ MODE_PREFILTER -----------------//
	uint32_t workGroupsX = m_BloomComputeTextures[0]->GetWidth() / m_BloomWorkGroupSize;
	uint32_t workGroupsY = m_BloomComputeTextures[0]->GetHeight() / m_BloomWorkGroupSize;

	{
		bloomConstants.Mode = 0;
		Engine::ShaderLibrary::Get("Bloom2")->UploadUniformFloat4("u_Params", bloomConstants.Params);
		Engine::ShaderLibrary::Get("Bloom2")->UploadUniformFloat("u_LOD", bloomConstants.LOD);
		Engine::ShaderLibrary::Get("Bloom2")->UploadUniformInt("u_Mode", bloomConstants.Mode);
		m_FBO->BindColorAttachment(0);
		Engine::ShaderLibrary::Get("Bloom2")->UploadUniformInt("u_Texture", 0);
		m_BloomComputeTextures[0]->BindToImageSlot(0, 0, Engine::ImageUtils::TextureAccessLevel::WriteOnly, Engine::ImageUtils::TextureShaderDataFormat::RGBA32F);

		Engine::ShaderLibrary::Get("Bloom2")->DispatchCompute(workGroupsX, workGroupsY, 1);
		Engine::ShaderLibrary::Get("Bloom2")->EnableShaderImageAccessBarrierBit();

		m_FBO->UnbindColorAttachment(0, 0);
	}
	//------------------ MODE_PREFILTER -----------------//


	//------------------ MODE_DOWNSAMPLE -----------------//
	bloomConstants.Mode = 1;
	uint32_t mips = m_BloomComputeTextures[0]->GetMipLevelCount() - 2;

	for (uint32_t mip = 1; mip < mips; mip++)
	{
		{
			auto [mipWidth, mipHeight] = m_BloomComputeTextures[0]->GetMipSize(mip);
			workGroupsX = (uint32_t)glm::ceil((float)mipWidth / (float)m_BloomWorkGroupSize);
			workGroupsY = (uint32_t)glm::ceil((float)mipHeight / (float)m_BloomWorkGroupSize);

			bloomConstants.LOD = mip - 1.0f;
			// Write to 1
			m_BloomComputeTextures[1]->BindToImageSlot(0, mip, Engine::ImageUtils::TextureAccessLevel::WriteOnly, Engine::ImageUtils::TextureShaderDataFormat::RGBA32F);
			// Read from 0 (starts pre-filtered)
			m_BloomComputeTextures[0]->BindToSamplerSlot(0);
			Engine::ShaderLibrary::Get("Bloom2")->UploadUniformInt("u_Texture", 0);
			Engine::ShaderLibrary::Get("Bloom2")->UploadUniformInt("u_Mode", bloomConstants.Mode);
			Engine::ShaderLibrary::Get("Bloom2")->UploadUniformFloat("u_LOD", bloomConstants.LOD);
			Engine::ShaderLibrary::Get("Bloom2")->DispatchCompute(workGroupsX, workGroupsY, 1);
			Engine::ShaderLibrary::Get("Bloom2")->EnableShaderImageAccessBarrierBit();
		}

		{
			bloomConstants.LOD = mip;
			// Write to 0
			m_BloomComputeTextures[0]->BindToImageSlot(0, mip, Engine::ImageUtils::TextureAccessLevel::WriteOnly, Engine::ImageUtils::TextureShaderDataFormat::RGBA32F);
			// Read from 1
			m_BloomComputeTextures[1]->BindToSamplerSlot(0);

			Engine::ShaderLibrary::Get("Bloom2")->UploadUniformInt("u_Texture", 0);
			Engine::ShaderLibrary::Get("Bloom2")->UploadUniformInt("u_Mode", bloomConstants.Mode);
			Engine::ShaderLibrary::Get("Bloom2")->UploadUniformFloat("u_LOD", bloomConstants.LOD);
			Engine::ShaderLibrary::Get("Bloom2")->DispatchCompute(workGroupsX, workGroupsY, 1);
			Engine::ShaderLibrary::Get("Bloom2")->EnableShaderImageAccessBarrierBit();
		}
	}
	//------------------ MODE_DOWNSAMPLE -----------------//

	//------------------ MODE_UPSAMPLE_FIRST -----------------//
	{
		bloomConstants.Mode = 2;
		bloomConstants.LOD--;
		// Write to 2 at smallest image in up-sampling mip chain
		m_BloomComputeTextures[2]->BindToImageSlot(0, mips - 2, Engine::ImageUtils::TextureAccessLevel::WriteOnly, Engine::ImageUtils::TextureShaderDataFormat::RGBA32F);
		// Read from 0 (fully down-sampled)
		m_BloomComputeTextures[0]->BindToSamplerSlot(0);

		Engine::ShaderLibrary::Get("Bloom2")->UploadUniformInt("u_Texture", 0);
		Engine::ShaderLibrary::Get("Bloom2")->UploadUniformInt("u_Mode", bloomConstants.Mode);
		Engine::ShaderLibrary::Get("Bloom2")->UploadUniformFloat("u_LOD", bloomConstants.LOD);

		auto [mipWidth, mipHeight] = m_BloomComputeTextures[2]->GetMipSize(mips - 2);
		workGroupsX = (uint32_t)glm::ceil((float)mipWidth / (float)m_BloomWorkGroupSize);
		workGroupsY = (uint32_t)glm::ceil((float)mipHeight / (float)m_BloomWorkGroupSize);
		Engine::ShaderLibrary::Get("Bloom2")->DispatchCompute(workGroupsX, workGroupsY, 1);
		Engine::ShaderLibrary::Get("Bloom2")->EnableShaderImageAccessBarrierBit();
	}
	//------------------ MODE_UPSAMPLE_FIRST -----------------//


	//------------------ UPSAMPLE -----------------//
	{
		bloomConstants.Mode = 3;
		for (int32_t mip = mips - 3; mip >= 0; mip--)
		{
			auto [mipWidth, mipHeight] = m_BloomComputeTextures[2]->GetMipSize(mip);
			workGroupsX = (uint32_t)glm::ceil((float)mipWidth / (float)m_BloomWorkGroupSize);
			workGroupsY = (uint32_t)glm::ceil((float)mipHeight / (float)m_BloomWorkGroupSize);

			bloomConstants.LOD = mip;

			// Write to 2
			Engine::ShaderLibrary::Get("Bloom2")->EnableShaderImageAccessBarrierBit();
			m_BloomComputeTextures[2]->BindToImageSlot(0, mip, Engine::ImageUtils::TextureAccessLevel::WriteOnly, Engine::ImageUtils::TextureShaderDataFormat::RGBA32F);
			// Read from 0	
			m_BloomComputeTextures[0]->BindToSamplerSlot(0);
			Engine::ShaderLibrary::Get("Bloom2")->UploadUniformInt("u_Texture", 0);
			m_BloomComputeTextures[2]->BindToSamplerSlot(1);
			Engine::ShaderLibrary::Get("Bloom2")->UploadUniformInt("u_BloomTexture", 1);
			Engine::ShaderLibrary::Get("Bloom2")->UploadUniformInt("u_Mode", bloomConstants.Mode);
			Engine::ShaderLibrary::Get("Bloom2")->UploadUniformFloat("u_LOD", bloomConstants.LOD);
			Engine::ShaderLibrary::Get("Bloom2")->DispatchCompute(workGroupsX, workGroupsY, 1);
			Engine::ShaderLibrary::Get("Bloom2")->EnableShaderImageAccessBarrierBit();
		}
	}
	//------------------ UPSAMPLE -----------------//

	for (auto tex : m_BloomComputeTextures)
		tex->Unbind();
	Engine::ShaderLibrary::Get("Bloom2")->Unbind();
}


void LineLayer::OnEvent(Engine::Event& event)
{
	m_Camera.OnEvent(event);

	Engine::EventDispatcher dispatcher(event);

	dispatcher.Dispatch<Engine::KeyPressedEvent>(BIND_FN(LineLayer::OnKeyPressed));
	dispatcher.Dispatch<Engine::MouseButtonReleasedEvent>(BIND_FN(LineLayer::OnMouseButtonReleased));
	dispatcher.Dispatch<Engine::MouseButtonPressedEvent>(BIND_FN(LineLayer::OnMouseButtonPressed));
}

void LineLayer::OnImGuiRender()
{
	ImGui::Begin("Scene Parameters");

	if (ImGui::TreeNodeEx("Scene Settings"))
	{
		ImGui::DragFloat3("Sphere Ambient Color", &m_SphereProperties.AmbientColor.x, 0.01);
		ImGui::DragFloat3("Sphere Diffuse Color", &m_SphereProperties.DiffuseColor.x, 0.01);
		ImGui::DragFloat("Sphere Ambient Strength", &m_SphereProperties.AmbientStrength, 0.01, 0.0f, 1.0f);
		ImGui::DragFloat("Sphere Diffuse Strength", &m_SphereProperties.DiffuseStrength, 0.01, 0.0f, 1.0f);
		ImGui::DragFloat("Sphere Shininess", &m_SphereProperties.Shininess, 0.01, 2.0f, 256.0f);
		ImGui::DragFloat3("Cube Position", &m_Cube->GetEntityTransform()->GetPosition().x, 0.01);
		ImGui::DragFloat3("Cube Scale", &m_Cube->GetEntityTransform()->GetScale().x, 0.01);
		ImGui::Separator();

		ImGui::DragFloat3("Light Position", &m_Light->GetLightTransform()->GetPosition().x);
		ImGui::DragFloat3("Light Color", &m_Light->GetLightColor().x);
		float intensity = m_Light->GetLightIntensity();
		ImGui::DragFloat("Light Intensity", &intensity);
		if (intensity != m_Light->GetLightIntensity())
			m_Light->SetIntensity(intensity);

		ImGui::TreePop();
	}

	if (ImGui::TreeNodeEx("Curve Settings"))
	{
		ImGui::DragFloat3("Line Color", &m_LineColor.x, 0.01);
		bool loop = m_Looped;
		ImGui::Checkbox("Loop", &loop);
		if (loop != m_Looped)
		{
			m_Curve->ToggleLooped();
			m_Looped = loop;
		}
		ImGui::Checkbox("Animate", &m_Animate);
		bool debug = m_Debug;
		ImGui::Checkbox("Debug Control Points", &debug);
		if (debug != m_Debug)
		{
			m_Curve->ToggleDebug();
			m_Debug = debug;
		}

		ImGui::TreePop();
	}

	if (ImGui::TreeNodeEx("Bloom"))
	{
		ImGui::Checkbox("Enabled", &m_BloomEnabled);
		ImGui::DragFloat("Bloom Dirt Intensity", &m_BloomDirtIntensity, 0.01f);
		ImGui::DragFloat("Bloom Intensity", &m_BloomIntensity, 0.01f);
		ImGui::DragFloat("Bloom Threshold", &m_BloomThreshold, 0.01f);
		ImGui::DragFloat("Bloom Knee", &m_BloomKnee, 0.001f);
		ImGui::DragFloat("Exposure", &m_Exposure, 0.01f);


		float aspect = (float)m_ViewportWidth / (float)m_ViewportHeight;
		ImGui::Image((ImTextureID)(m_BloomComputeTextures[0]->GetID()), { 300 * aspect, 300 }, { 0, 1 }, { 1, 0 });
		ImGui::Image((ImTextureID)(m_BloomComputeTextures[2]->GetID()), { 300 * aspect, 300 }, { 0, 1 }, { 1, 0 });

		ImGui::TreePop();
	}

	if (ImGui::TreeNodeEx("Camera Settings"))
	{
		bool isOrthographic = m_Orthographic;
		ImGui::Checkbox("Orthographic", &isOrthographic);

		if (m_Orthographic != isOrthographic)
		{
			if (m_Orthographic)
				m_Camera.SetPerspective();
			else
				m_Camera.SetOrthographic();

			m_Orthographic = isOrthographic;
		}

		ImGui::TreePop();
	}

	ImGui::End();
}


bool LineLayer::OnKeyPressed(Engine::KeyPressedEvent& keyPressedEvent)
{
	if (keyPressedEvent.GetKeyCode() == Engine::Key::Space)
		m_Curve->Clear();

	return true;
}

bool LineLayer::OnMouseButtonReleased(Engine::MouseButtonReleasedEvent& releasedEvent)
{
	if (m_IsDragging && releasedEvent.GetButton() == Engine::Mouse::ButtonLeft)
	{
		m_IsDragging = false;
		m_DragID = -1;
	}

	return true;
}

bool LineLayer::OnMouseButtonPressed(Engine::MouseButtonPressedEvent& pressedEvent)
{
	if (!m_IsDragging && pressedEvent.GetButton() == Engine::Mouse::ButtonLeft)
	{
		glm::vec2 mousePosition = Engine::Input::GetMousePosition();
		glm::vec3 worldCoordinate = m_Camera.ScreenToWorldPoint({ mousePosition.x, mousePosition.y, 0.0f });

		for (uint32_t i = 0; i < m_Curve->GetPointCount(); i++)
		{
			Engine::LineVertex vertex = (*m_Curve)[i];

			glm::vec3 offset = vertex.Position - worldCoordinate;

			if (glm::dot(offset, offset) < glm::dot(0.04f, 0.04f))
			{
				m_IsDragging = true;
				m_DragID = i;
			}
		}
	}

	if (pressedEvent.GetButton() == Engine::Mouse::ButtonRight && Engine::Input::IsKeyPressed(Engine::Key::LeftShift))
	{
		glm::vec2 mousePosition = Engine::Input::GetMousePosition();
		glm::vec3 worldCoordinate = m_Camera.ScreenToWorldPoint({ mousePosition.x, mousePosition.y, 0.0f });
		m_Curve->AddSegment({ worldCoordinate });

		m_BaseCurveVertices.clear();

		for (auto lineVertex : *m_Curve)
			m_BaseCurveVertices.push_back(lineVertex);
	}


	return true;
}

void LineLayer::OnResize()
{
	m_FBO->Resize(m_ViewportWidth, m_ViewportHeight);

	uint32_t halfWidth = m_ViewportWidth / 2;
	uint32_t halfHeight = m_ViewportHeight / 2;
	halfWidth += (m_BloomWorkGroupSize - (halfWidth % m_BloomWorkGroupSize));
	halfHeight += (m_BloomWorkGroupSize - (halfHeight % m_BloomWorkGroupSize));

	m_BloomComputeTextures[0]->Resize(halfWidth, halfHeight);
	m_BloomComputeTextures[1]->Resize(halfWidth, halfHeight);
	m_BloomComputeTextures[2]->Resize(halfWidth, halfHeight);
}
