#include "Layers/BloomDemoLayer.h"
#include <vector>
#include <imgui.h>
#include <unordered_map>

#define PI 3.14159265358979
#include <string>

#include "Utility/Curves/CurveSerializer.h"
#include "Utility/UI/ImGuiUtility.h"

BloomDemoLayer::BloomDemoLayer()
{

}

BloomDemoLayer::~BloomDemoLayer()
{

}

void BloomDemoLayer::OnAttach()
{
	Engine::Application::GetApplication().GetWindow().ToggleMaximize(true);
	//m_Camera.SetOrthographic();

	m_WhiteTexture = Engine::Texture2D::CreateWhiteTexture();
	m_ViewportWidth = Engine::Application::GetApplication().GetWindow().GetWidth();
	m_ViewportHeight = Engine::Application::GetApplication().GetWindow().GetHeight();

	m_Surface = Engine::CreateRef<Engine::BezierSurface>();
	m_Surface->SetScale( glm::vec3(1.5f, 1.0f, 1.0f) * 10.0f);
	m_Surface->SetPosition({7.0f, 10.0f, 0.0f});

	Engine::FramebufferSpecification fboSpec
	{
		m_ViewportWidth, m_ViewportHeight,
		{ Engine::FramebufferTextureFormat::RGBA32F, Engine::FramebufferTextureFormat::Depth }
	};
	m_FBO = Engine::CreateRef<Engine::Framebuffer>(fboSpec);


	Engine::Texture2DSpecification fileTexSpec =
	{
		Engine::ImageUtils::WrapMode::Repeat,
		Engine::ImageUtils::WrapMode::Repeat,
		Engine::ImageUtils::FilterMode::Linear,
		Engine::ImageUtils::FilterMode::Linear,
		Engine::ImageUtils::ImageInternalFormat::FromImage,
		Engine::ImageUtils::ImageDataLayout::FromImage,
		Engine::ImageUtils::ImageDataType::UByte,
	};

	m_BloomDirtTexture = Engine::CreateRef<Engine::Texture2D>("assets/textures/dirt-mask.png", fileTexSpec);
	//m_BrickWallTexture = Engine::CreateRef<Engine::Texture2D>("assets/textures/brick-red.jpg", fileTexSpec);
	m_BrickWallTexture = Engine::CreateRef<Engine::Texture2D>("assets/textures/space.jpg", fileTexSpec);
	m_GroundTexture = Engine::CreateRef<Engine::Texture2D>("assets/textures/floor3.jpg", fileTexSpec);
	//m_GroundTexture = Engine::CreateRef<Engine::Texture2D>("assets/textures/ground-blue.jpg", fileTexSpec);
	Engine::ShaderLibrary::Load("assets/shaders/BlinnPhong/BlinnPhongSimple.glsl");

	Engine::Texture2DSpecification bloomSpec =
	{
		Engine::ImageUtils::WrapMode::ClampToEdge,
		Engine::ImageUtils::WrapMode::ClampToEdge,
		Engine::ImageUtils::FilterMode::LinearMipLinear,
		Engine::ImageUtils::FilterMode::Linear,
		Engine::ImageUtils::ImageInternalFormat::RGBA32F,
		Engine::ImageUtils::ImageDataLayout::RGBA,
		Engine::ImageUtils::ImageDataType::Float,
		1, 1
	};

	m_BloomComputeTextures.resize(3);
	m_BloomComputeTextures[0] = Engine::CreateRef<Engine::Texture2D>(bloomSpec);
	m_BloomComputeTextures[1] = Engine::CreateRef<Engine::Texture2D>(bloomSpec);
	m_BloomComputeTextures[2] = Engine::CreateRef<Engine::Texture2D>(bloomSpec);

	m_FullScreenQuad = Engine::CreateRef<Engine::SimpleEntity>(Engine::PrimitiveType::FullScreenQuad, "BloomPostProcessing");

	m_Table = Engine::CreateRef<Engine::SimpleEntity>(Engine::PrimitiveType::Cube, "BlinnPhongWS");
	m_Table->GetEntityTransform()->SetScale({ 26.940f, 3.00f, 19.480f });
	m_Table->GetEntityTransform()->SetPosition({ 4.730f, -10.010f, 6.100f});

	m_BrickWall = Engine::CreateRef<Engine::SimpleEntity>(Engine::PrimitiveType::Cube, "BlinnPhongWS");
	m_BrickWall->GetEntityTransform()->SetScale({ 26.780f, 26.010, 2.00f });
	m_BrickWall->GetEntityTransform()->SetPosition({ 4.730f, 1.890f, -2.630f });

	Engine::LightSpecification sunSpec =
	{
		Engine::LightType::Directional,
		{1.0f, 1.0f, 1.0f},
		1.0f
	};

	m_Light = Engine::CreateRef<Engine::Light>(sunSpec);
	m_Light->GetLightTransform()->SetPosition({ 10.0f, 50.0f, 10.0f });

	std::vector<std::string> ramenSignFiles = CurveSerializer::GetCurveSavesFromDirectory("Ramen");

	Engine::RenderCommand::SetPointSize(10.0f);
	for (auto save : ramenSignFiles)
	{
		Engine::Ref<Engine::BezierCurve> curve = Engine::CreateRef<Engine::BezierCurve>();

		CurveSerializer::DeserializeAndWriteToCurve(save, curve);
		if (save == "Ramen/Chopstick2.txt")
		{
			m_ChopstickA = curve;
			m_ChopStickAColor = curve->GetCurveColor();
		}
		else if (save == "Ramen/Chopstick1.txt")
		{
			m_ChopstickB = curve;
			m_ChopStickBColor = curve->GetCurveColor();
			m_ChopstickB->SetCurveColor({ 0.1f, 0.1f, 0.1f });
		}

		m_Curves.push_back(curve);
	}

	OnResize();
}

void BloomDemoLayer::OnDetach()
{

}

void BloomDemoLayer::CheckForResize()
{
	uint32_t width = Engine::Application::GetApplication().GetWindow().GetWidth();
	uint32_t height = Engine::Application::GetApplication().GetWindow().GetHeight();
	if (m_ViewportWidth != width || m_ViewportHeight != height)
	{
		m_ViewportWidth = width;
		m_ViewportHeight = height;

		OnResize();
	}
}

void BloomDemoLayer::PollCurvePicking()
{
	if (m_Curves.size() <= 0) return;

	auto curve = m_Curves[m_CurveEditIndex];

	glm::vec2 mousePosition = Engine::Input::GetMousePosition();
	glm::vec3 worldCoordinate = m_Camera.ScreenToWorldPoint({ mousePosition.x, mousePosition.y, 0.0f });

	if (m_IsDragging && m_DragID != -1)
	{
		Engine::LineVertex dragPoint = (*curve)[m_DragID];
		worldCoordinate.z = 0.0;

		glm::vec2 pointTransformOffset = glm::vec2(curve->GetTransform()->GetPosition());

		glm::vec2 dragPointWorld = glm::vec2(curve->GetTransform()->Transform() * glm::vec4(dragPoint.Position, 1.0));

		glm::vec2 worldPoint = pointTransformOffset + glm::vec2(dragPoint.Position);

		if (glm::vec2(worldCoordinate) != glm::vec2(dragPointWorld))
		{
			glm::vec3 curveSpace = glm::vec3(glm::inverse(curve->GetTransform()->Transform()) * glm::vec4(worldCoordinate, 1.0f));
			curveSpace.z = 0.0f;
			curve->MovePoint(m_DragID, curveSpace);
		}
	}
}

void BloomDemoLayer::AnimateChopsticks()
{
	if (Engine::Time::Elapsed() > m_NextChopStickAnimationTime)
	{
		if (m_ChopSticksAOn)
		{
			m_ChopstickA->SetCurveColor({ 0.1f, 0.1f, 0.1f });
			m_ChopstickB->SetCurveColor(m_ChopStickBColor);
		}
		else
		{
			m_ChopstickA->SetCurveColor(m_ChopStickAColor);
			m_ChopstickB->SetCurveColor({ 0.1f, 0.1f, 0.1f });
		}
		m_ChopSticksAOn = !m_ChopSticksAOn;

		m_NextChopStickAnimationTime = Engine::Time::Elapsed() +  m_ChopStickAnimationPeriod;
	}
}

void BloomDemoLayer::OnUpdate(float deltaTime)
{
	CheckForResize();
	m_Camera.Update(deltaTime);
	PollCurvePicking();
	m_Surface->UpdateDragCurve(m_Camera);

	m_FBO->Bind();
	Engine::RenderCommand::ClearColor(m_ClearColor);
	Engine::RenderCommand::Clear(true, true);

	if (m_AnimateChopsticks)
		AnimateChopsticks();

	if (m_DrawSign)
	{
		for (auto curve : m_Curves)
			curve->Draw(m_Camera.GetViewProjection());
	}

	if (m_DrawGround)
	{
		m_Table->GetEntityRenderer()->GetShader()->Bind();
		m_WhiteTexture->BindToSamplerSlot(0);
		m_Table->GetEntityRenderer()->GetShader()->UploadUniformMat4("u_ModelMatrix", m_Table->GetEntityTransform()->Transform());
		m_GroundTexture->BindToSamplerSlot(0);
		m_Table->GetEntityRenderer()->GetShader()->UploadUniformInt("u_Texture", 0);
		m_Table->GetEntityRenderer()->GetShader()->UploadUniformFloat3("u_MaterialProperties.AmbientColor", m_TableProperties.AmbientColor);
		m_Table->GetEntityRenderer()->GetShader()->UploadUniformFloat3("u_MaterialProperties.DiffuseColor", m_TableProperties.DiffuseColor);
		m_Table->GetEntityRenderer()->GetShader()->UploadUniformFloat("u_MaterialProperties.AmbientStrength", m_TableProperties.AmbientStrength);
		m_Table->GetEntityRenderer()->GetShader()->UploadUniformFloat("u_MaterialProperties.DiffuseStrength", m_TableProperties.DiffuseStrength);
		m_Table->GetEntityRenderer()->GetShader()->UploadUniformFloat("u_MaterialProperties.SpecularStrength", m_TableProperties.SpecularStrength);
		m_Table->GetEntityRenderer()->GetShader()->UploadUniformFloat("u_MaterialProperties.Shininess", m_TableProperties.Shininess);
		m_Table->GetEntityRenderer()->GetShader()->UploadUniformFloat3("u_Light.Position", m_Light->GetLightTransform()->GetPosition());
		m_Table->GetEntityRenderer()->GetShader()->UploadUniformFloat("u_Light.Intensity", m_Light->GetLightIntensity());
		m_Table->GetEntityRenderer()->GetShader()->UploadUniformFloat3("u_Light.Color", m_Light->GetLightColor());
		m_Table->GetEntityRenderer()->GetShader()->UploadUniformFloat3("u_CameraPosition", m_Camera.GetPosition());
		m_Table->DrawEntity(m_Camera.GetViewProjection());
	}

	if (m_DrawWall)
	{
		m_BrickWall->GetEntityRenderer()->GetShader()->Bind();
		m_BrickWallTexture->BindToSamplerSlot(0);
		m_BrickWall->GetEntityRenderer()->GetShader()->UploadUniformMat4("u_ModelMatrix", m_BrickWall->GetEntityTransform()->Transform());
		m_Table->GetEntityRenderer()->GetShader()->UploadUniformInt("u_Texture", 0);
		m_BrickWall->GetEntityRenderer()->GetShader()->UploadUniformFloat3("u_MaterialProperties.AmbientColor", m_BrickProperties.AmbientColor);
		m_BrickWall->GetEntityRenderer()->GetShader()->UploadUniformFloat3("u_MaterialProperties.DiffuseColor", m_BrickProperties.DiffuseColor);
		m_BrickWall->GetEntityRenderer()->GetShader()->UploadUniformFloat("u_MaterialProperties.AmbientStrength", m_BrickProperties.AmbientStrength);
		m_BrickWall->GetEntityRenderer()->GetShader()->UploadUniformFloat("u_MaterialProperties.DiffuseStrength", m_BrickProperties.DiffuseStrength);
		m_BrickWall->GetEntityRenderer()->GetShader()->UploadUniformFloat("u_MaterialProperties.SpecularStrength", m_BrickProperties.SpecularStrength);
		m_BrickWall->GetEntityRenderer()->GetShader()->UploadUniformFloat("u_MaterialProperties.Shininess", m_BrickProperties.Shininess);
		m_BrickWall->GetEntityRenderer()->GetShader()->UploadUniformFloat3("u_Light.Position", m_Light->GetLightTransform()->GetPosition());
		m_BrickWall->GetEntityRenderer()->GetShader()->UploadUniformFloat("u_Light.Intensity", m_Light->GetLightIntensity());
		m_BrickWall->GetEntityRenderer()->GetShader()->UploadUniformFloat3("u_Light.Color", m_Light->GetLightColor());
		m_BrickWall->GetEntityRenderer()->GetShader()->UploadUniformFloat3("u_CameraPosition", m_Camera.GetPosition());
		m_BrickWall->DrawEntity(m_Camera.GetViewProjection());
	}

	if (m_DrawFlag)
	{
		m_Surface->GetShader()->Bind();
		m_Surface->GetShader()->UploadUniformFloat3("u_CameraPosition", m_Camera.GetPosition());
		m_Surface->GetShader()->UploadUniformFloat3("u_LightPosition", m_Light->GetLightTransform()->GetPosition());
		m_Surface->GetShader()->UploadUniformFloat3("u_LightColor", m_Light->GetLightColor());
		m_Surface->GetShader()->UploadUniformFloat("u_LightIntensity", m_Light->GetLightIntensity());
		m_Surface->GetShader()->UploadUniformFloat("u_AmbientStrength", m_FlagProperties.AmbientStrength);
		m_Surface->GetShader()->UploadUniformFloat("u_DiffuseStrength", m_FlagProperties.DiffuseStrength);
		m_Surface->GetShader()->UploadUniformFloat("u_SpecularStrength", m_FlagProperties.SpecularStrength);
		m_Surface->GetShader()->UploadUniformFloat("u_Shininess", m_FlagProperties.Shininess);
		m_Surface->GetShader()->UploadUniformFloat3("u_AmbientColor", m_FlagProperties.AmbientColor);
		m_Surface->GetShader()->UploadUniformFloat3("u_DiffuseColor", m_FlagProperties.DiffuseColor);
		m_Surface->AnimateControls();
		m_Surface->Draw(m_Camera.GetViewProjection());
	}
	m_FBO->Unbind();

	BloomComputePass();

	Engine::RenderCommand::Clear(true, true);
	m_FullScreenQuad->GetEntityRenderer()->GetShader()->Bind();
	m_FullScreenQuad->GetEntityRenderer()->GetShader()->UploadUniformFloat("u_Exposure", m_Exposure);
	m_FullScreenQuad->GetEntityRenderer()->GetShader()->UploadUniformFloat("u_BloomIntensity", m_BloomEnabled ? m_BloomIntensity : 0.0f);
	m_FullScreenQuad->GetEntityRenderer()->GetShader()->UploadUniformFloat("u_BloomDirtIntensity", m_BloomEnabled && m_BloomDirtEnabled ? m_BloomDirtIntensity : 0.0f);

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


void BloomDemoLayer::BloomComputePass()
{
	Engine::ShaderLibrary::Get("Bloom")->Bind();

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
		Engine::ShaderLibrary::Get("Bloom")->UploadUniformFloat4("u_Params", bloomConstants.Params);
		Engine::ShaderLibrary::Get("Bloom")->UploadUniformFloat("u_LOD", bloomConstants.LOD);
		Engine::ShaderLibrary::Get("Bloom")->UploadUniformInt("u_Mode", bloomConstants.Mode);
		m_FBO->BindColorAttachment(0);
		Engine::ShaderLibrary::Get("Bloom")->UploadUniformInt("u_Texture", 0);
		m_BloomComputeTextures[0]->BindToImageSlot(0, 0, Engine::ImageUtils::TextureAccessLevel::WriteOnly, Engine::ImageUtils::TextureShaderDataFormat::RGBA32F);

		Engine::ShaderLibrary::Get("Bloom")->DispatchCompute(workGroupsX, workGroupsY, 1);
		Engine::ShaderLibrary::Get("Bloom")->EnableShaderImageAccessBarrierBit();

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
			Engine::ShaderLibrary::Get("Bloom")->UploadUniformInt("u_Texture", 0);
			Engine::ShaderLibrary::Get("Bloom")->UploadUniformInt("u_Mode", bloomConstants.Mode);
			Engine::ShaderLibrary::Get("Bloom")->UploadUniformFloat("u_LOD", bloomConstants.LOD);
			Engine::ShaderLibrary::Get("Bloom")->DispatchCompute(workGroupsX, workGroupsY, 1);
			Engine::ShaderLibrary::Get("Bloom")->EnableShaderImageAccessBarrierBit();
		}

		{
			bloomConstants.LOD = mip;
			// Write to 0
			m_BloomComputeTextures[0]->BindToImageSlot(0, mip, Engine::ImageUtils::TextureAccessLevel::WriteOnly, Engine::ImageUtils::TextureShaderDataFormat::RGBA32F);
			// Read from 1
			m_BloomComputeTextures[1]->BindToSamplerSlot(0);

			Engine::ShaderLibrary::Get("Bloom")->UploadUniformInt("u_Texture", 0);
			Engine::ShaderLibrary::Get("Bloom")->UploadUniformInt("u_Mode", bloomConstants.Mode);
			Engine::ShaderLibrary::Get("Bloom")->UploadUniformFloat("u_LOD", bloomConstants.LOD);
			Engine::ShaderLibrary::Get("Bloom")->DispatchCompute(workGroupsX, workGroupsY, 1);
			Engine::ShaderLibrary::Get("Bloom")->EnableShaderImageAccessBarrierBit();
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

		Engine::ShaderLibrary::Get("Bloom")->UploadUniformInt("u_Texture", 0);
		Engine::ShaderLibrary::Get("Bloom")->UploadUniformInt("u_Mode", bloomConstants.Mode);
		Engine::ShaderLibrary::Get("Bloom")->UploadUniformFloat("u_LOD", bloomConstants.LOD);

		auto [mipWidth, mipHeight] = m_BloomComputeTextures[2]->GetMipSize(mips - 2);
		workGroupsX = (uint32_t)glm::ceil((float)mipWidth / (float)m_BloomWorkGroupSize);
		workGroupsY = (uint32_t)glm::ceil((float)mipHeight / (float)m_BloomWorkGroupSize);
		Engine::ShaderLibrary::Get("Bloom")->DispatchCompute(workGroupsX, workGroupsY, 1);
		Engine::ShaderLibrary::Get("Bloom")->EnableShaderImageAccessBarrierBit();
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
			Engine::ShaderLibrary::Get("Bloom")->EnableShaderImageAccessBarrierBit();
			m_BloomComputeTextures[2]->BindToImageSlot(0, mip, Engine::ImageUtils::TextureAccessLevel::WriteOnly, Engine::ImageUtils::TextureShaderDataFormat::RGBA32F);
			// Read from 0	
			m_BloomComputeTextures[0]->BindToSamplerSlot(0);
			Engine::ShaderLibrary::Get("Bloom")->UploadUniformInt("u_Texture", 0);
			m_BloomComputeTextures[2]->BindToSamplerSlot(1);
			Engine::ShaderLibrary::Get("Bloom")->UploadUniformInt("u_BloomTexture", 1);
			Engine::ShaderLibrary::Get("Bloom")->UploadUniformInt("u_Mode", bloomConstants.Mode);
			Engine::ShaderLibrary::Get("Bloom")->UploadUniformFloat("u_LOD", bloomConstants.LOD);
			Engine::ShaderLibrary::Get("Bloom")->DispatchCompute(workGroupsX, workGroupsY, 1);
			Engine::ShaderLibrary::Get("Bloom")->EnableShaderImageAccessBarrierBit();
		}
	}
	//------------------ UPSAMPLE -----------------//

	for (auto tex : m_BloomComputeTextures)
		tex->Unbind();
	Engine::ShaderLibrary::Get("Bloom")->Unbind();
}



void BloomDemoLayer::OnEvent(Engine::Event& event)
{
	m_Camera.OnEvent(event);

	Engine::EventDispatcher dispatcher(event);

	dispatcher.Dispatch<Engine::KeyPressedEvent>(BIND_FN(BloomDemoLayer::OnKeyPressed));
	dispatcher.Dispatch<Engine::MouseButtonReleasedEvent>(BIND_FN(BloomDemoLayer::OnMouseButtonReleased));
	dispatcher.Dispatch<Engine::MouseButtonPressedEvent>(BIND_FN(BloomDemoLayer::OnMouseButtonPressed));
}

void BloomDemoLayer::OnImGuiRender()
{
	ImGui::Begin("Scene Parameters");

	if (ImGui::TreeNodeEx("Scene Settings"))
	{
		if (ImGui::TreeNodeEx("Table Settings"))
		{
			ImGui::DragFloat3("Table Ambient Color", &m_TableProperties.AmbientColor.x, 0.01);
			ImGui::DragFloat3("Table Diffuse Color", &m_TableProperties.DiffuseColor.x, 0.01);
			ImGui::DragFloat("Table Ambient Strength", &m_TableProperties.AmbientStrength, 0.01, 0.0f, 1.0f);
			ImGui::DragFloat("Table Diffuse Strength", &m_TableProperties.DiffuseStrength, 0.01, 0.0f, 1.0f);
			ImGui::DragFloat("Table Specular Strength", &m_TableProperties.SpecularStrength, 0.01, 0.0f, 1.0f);
			ImGui::DragFloat("Table Shininess", &m_TableProperties.Shininess, 0.01, 2.0f, 256.0f);
			ImGui::DragFloat3("Table Position", &m_Table->GetEntityTransform()->GetPosition().x, 0.01);
			ImGui::DragFloat3("Table Scale", &m_Table->GetEntityTransform()->GetScale().x, 0.01);
			ImGui::Checkbox("Draw Table", &m_DrawGround);

			ImGui::TreePop();
		}
		ImGui::Separator();
		if (ImGui::TreeNodeEx("Brick Wall Settings"))
		{
			ImGui::DragFloat3("Brick Wall Ambient Color", &m_BrickProperties.AmbientColor.x, 0.01);
			ImGui::DragFloat3("Brick Wall Diffuse Color", &m_BrickProperties.DiffuseColor.x, 0.01);
			ImGui::DragFloat("Brick Wall Ambient Strength", &m_BrickProperties.AmbientStrength, 0.01, 0.0f, 1.0f);
			ImGui::DragFloat("Brick Wall Diffuse Strength", &m_BrickProperties.DiffuseStrength, 0.01, 0.0f, 1.0f);
			ImGui::DragFloat("Brick Wall Specular Strength", &m_BrickProperties.SpecularStrength, 0.01, 0.0f, 1.0f);
			ImGui::DragFloat("Brick Wall Shininess", &m_BrickProperties.Shininess, 0.01, 2.0f, 256.0f);
			ImGui::DragFloat3("Brick Wall Position", &m_BrickWall->GetEntityTransform()->GetPosition().x, 0.01);
			ImGui::DragFloat3("Brick Wall Scale", &m_BrickWall->GetEntityTransform()->GetScale().x, 0.01);
			ImGui::Checkbox("Draw Wall", &m_DrawWall);

			ImGui::TreePop();
		}

		ImGui::Separator();

		if (ImGui::TreeNodeEx("Light Settings"))
		{
			ImGui::DragFloat3("Light Position", &m_Light->GetLightTransform()->GetPosition().x);
			ImGui::DragFloat3("Light Color", &m_Light->GetLightColor().x);
			float intensity = m_Light->GetLightIntensity();
			ImGui::DragFloat("Light Intensity", &intensity);
			if (intensity != m_Light->GetLightIntensity())
				m_Light->SetIntensity(intensity);

			ImGui::TreePop();
		}
		ImGui::Separator();

		if (ImGui::TreeNodeEx("Flag Settings"))
		{
			ImGui::DragFloat3("Flag Ambient Color", &m_FlagProperties.AmbientColor.x, 0.01);
			ImGui::DragFloat3("Flag Diffuse Color", &m_FlagProperties.DiffuseColor.x, 0.01);
			ImGui::DragFloat("Flag Ambient Strength", &m_FlagProperties.AmbientStrength, 0.01, 0.0f, 1.0f);
			ImGui::DragFloat("Flag Diffuse Strength", &m_FlagProperties.DiffuseStrength, 0.01, 0.0f, 1.0f);
			ImGui::DragFloat("Flag Specular Strength", &m_FlagProperties.SpecularStrength, 0.01, 0.0f, 1.0f);
			ImGui::DragFloat("Flag Shininess", &m_FlagProperties.Shininess, 0.01, 2.0f, 256.0f);

			glm::vec3 surfacePosition = m_Surface->GetPosition();
			glm::vec3 surfaceScale = m_Surface->GetScale();
			glm::vec3 surfaceRotation = m_Surface->GetRotation();
			ImGui::DragFloat3("Flag Position", &surfacePosition.x, 0.01);
			ImGui::DragFloat3("Flag Rotation", &surfaceRotation.x, 0.01);
			ImGui::DragFloat3("Flag Scale", &surfaceScale.x, 0.01);

			if (surfaceScale != m_Surface->GetScale())
				m_Surface->SetScale(surfaceScale);
			if (surfacePosition != m_Surface->GetPosition())
				m_Surface->SetPosition(surfacePosition);
			if (surfaceRotation != m_Surface->GetRotation())
				m_Surface->SetRotation(surfaceRotation);

			ImGui::Separator();

			bool animate = m_Surface->IsAnimating();
			ImGui::Checkbox("Animate", &animate);
			if (animate != m_Surface->IsAnimating())
				m_Surface->ToggleAnimation();

			bool wireFrame = m_Surface->IsWireFrame();
			ImGui::Checkbox("Wire Frame", &wireFrame);
			if (wireFrame != m_Surface->IsWireFrame())
				m_Surface->ToggleWireFrame();

			bool drawCurves = m_Surface->IsDrawCurves();
			ImGui::Checkbox("Draw Curves", &drawCurves);
			if (drawCurves != m_Surface->IsDrawCurves())
				m_Surface->ToggleDrawCurves();

			if (ImGui::Button("Reset Flag"))
				m_Surface->ResetCurves();

			ImGui::Checkbox("Draw Flag", &m_DrawFlag);

			ImGui::TreePop();
		}
		ImGui::TreePop();
	}

	if (ImGui::TreeNodeEx("Curve Settings"))
	{
		ImGui::Checkbox("Draw Sign", &m_DrawSign);

		if (ImGui::Button("Create Curve"))
		{
			if (m_Curves.size() > 0)
				m_Curves[m_CurveEditIndex]->SetDebug(false);
			auto curve = Engine::CreateRef<Engine::BezierCurve>();
			curve->SetPointSize(10.0f);
			curve->SetCurveColor({ 10.0f, 1.0f, 6.0f });
			curve->SetDebug(true);
			m_Curves.push_back(curve);
			m_CurveEditIndex = m_Curves.size() - 1;
		}

		if (m_ChopstickA != nullptr && m_ChopstickB != nullptr)
			ImGui::Checkbox("Animate Chopsticks", &m_AnimateChopsticks);

		if (m_Curves.size() > 0)
		{
			int editIndex = m_CurveEditIndex;
			ImGui::SliderInt("Curve Index", &editIndex, 0, m_Curves.size() - 1);

			if (editIndex != m_CurveEditIndex)
			{
				m_Curves[m_CurveEditIndex]->SetDebug(false);
				m_CurveEditIndex = editIndex;
				m_Curves[m_CurveEditIndex]->SetDebug(true);
			}

			Engine::Ref<Engine::BezierCurve> curve = m_Curves[m_CurveEditIndex];

			std::string curveLabel = "Curve " + std::to_string(m_CurveEditIndex) + " Properties";
			if (ImGui::TreeNodeEx(curveLabel.c_str()))
			{
				std::string transformLabel = curveLabel + std::string(" Transform");
				if (ImGui::TreeNodeEx("Transform"))
				{
					ImGuiUtility::DrawVec3Controls("Position", curve->GetTransform()->GetPosition());
					ImGuiUtility::DrawVec3Controls("Rotation", curve->GetTransform()->GetRotation());
					ImGuiUtility::DrawVec3Controls("Scale", curve->GetTransform()->GetScale(), 1.0f);

					ImGui::TreePop();
				}

				if (ImGui::TreeNodeEx("Appearance Properties"))
				{
					ImGui::ColorEdit3("Curve Color", &curve->GetCurveColor().x, ImGuiColorEditFlags_HDR | ImGuiColorEditFlags_Float);

					bool loop = curve->GetIsLooped();
					ImGui::Checkbox("Loop", &loop);
					if (loop != curve->GetIsLooped())
						curve->ToggleLooped();

					bool debug = curve->GetIsDisplayingDebug();
					ImGui::Checkbox("Debug Control Points", &debug);
					if (debug != curve->GetIsDisplayingDebug())
						curve->ToggleDebug();

					ImGui::TreePop();
				}


				if (ImGui::TreeNodeEx("Curve Points"))
				{
					for (uint32_t i = 0; i < curve->GetSegmentCount(); i++)
					{
						std::string segmentLabel = "Segment: " + std::to_string(i);

						if (ImGui::TreeNodeEx(segmentLabel.c_str()))
						{
							std::vector<Engine::LineVertex> segmentPoints = curve->GetPointsInSegment(i);

							for (uint32_t p = 0; p < segmentPoints.size(); p++)
							{
								std::string pointTypeDescriptor = p % 3 == 0 ? "Anchor: " : "Control: ";
								pointTypeDescriptor += std::to_string(p);
								std::string pointLabel = std::to_string(i) + " - " + pointTypeDescriptor;

								glm::vec3 position = segmentPoints[p].Position;
								if (ImGui::DragFloat3(pointLabel.c_str(), &position.x, 0.01f))
								{
									curve->MovePoint(i * 3 + p, position);
								}
							}

							ImGui::TreePop();
						}
					}
					ImGui::TreePop();
				}

				ImGui::TreePop();
			}

			if (ImGui::TreeNodeEx("Save/Load"))
			{
				if (ImGui::TreeNodeEx("Save"))
				{
					char buffer[256];
					memset(buffer, 0, sizeof(buffer));
					std::strncpy(buffer, m_SaveNameHolder.c_str(), sizeof(buffer));
					if (ImGui::InputText("Save Name", buffer, sizeof(buffer)))
						m_SaveNameHolder = std::string(buffer);

					if (ImGui::Button("Save Curve"))
					{
						bool containsSubDirectory = m_SaveNameHolder.find("/") != std::string::npos;

						if (containsSubDirectory)
						{
							std::string fileName = m_SaveNameHolder.substr(m_SaveNameHolder.find_first_of("/") + 1, m_SaveNameHolder.length() - m_SaveNameHolder.find_first_of("/") + 1);
							CurveSerializer::SerializeCurve(fileName, curve, m_SaveNameHolder.substr(0, m_SaveNameHolder.find_first_of("/")));
						}
						else
							CurveSerializer::SerializeCurve(m_SaveNameHolder, curve, "My Curves");
					}
					
					ImGui::TreePop();
				}

				std::unordered_map<std::string, std::vector<std::string>> saves = CurveSerializer::GetAllCurvePaths();
				if (ImGui::TreeNodeEx("Load"))
				{
					for (auto directoryFilesNamesPair : saves)
					{
						std::vector<std::string> fileNames = CurveSerializer::GetCurveSavesFromDirectory(directoryFilesNamesPair.first);

						if (ImGui::TreeNodeEx(directoryFilesNamesPair.first.c_str()))
						{
							for (uint32_t i = 0; i < fileNames.size(); i++)
							{
								std::string name = fileNames[i];
								std::string displayName = name.substr(directoryFilesNamesPair.first.length() + 1, fileNames[i].length() - directoryFilesNamesPair.first.length());
								if (ImGui::Button(displayName.c_str()))
								{
									CurveSerializer::DeserializeAndWriteToCurve(name, curve);
								}
							}

							ImGui::TreePop();
						}
					}

					ImGui::TreePop();
				}
				ImGui::TreePop();
			}

			if (ImGui::Button("Delete Curve"))
			{
				m_Curves.erase(m_Curves.begin() + m_CurveEditIndex);
				if(!m_Curves.empty())
					m_CurveEditIndex = m_CurveEditIndex % m_Curves.size();
			}
		}

		ImGui::TreePop();
	}

	if (ImGui::TreeNodeEx("Bloom"))
	{
		ImGui::Checkbox("Enabled", &m_BloomEnabled);
		ImGui::Checkbox("Enable Bloom Dirt", &m_BloomDirtEnabled);
		ImGui::DragFloat("Bloom Dirt Intensity", &m_BloomDirtIntensity, 0.01f);
		ImGui::DragFloat("Bloom Intensity", &m_BloomIntensity, 0.01f);
		ImGui::DragFloat("Bloom Threshold", &m_BloomThreshold, 0.01f);
		ImGui::DragFloat("Bloom Knee", &m_BloomKnee, 0.001f);
		ImGui::DragFloat("Exposure", &m_Exposure, 0.01f);


		float aspect = (float)m_ViewportWidth / (float)m_ViewportHeight;
		ImGui::Image((ImTextureID)(m_BloomComputeTextures[0]->GetID()), { 300 * aspect, 300 }, { 0, 1 }, { 1, 0 });
		ImGui::Image((ImTextureID)(m_BloomComputeTextures[1]->GetID()), { 300 * aspect, 300 }, { 0, 1 }, { 1, 0 });
		ImGui::Image((ImTextureID)(m_BloomComputeTextures[2]->GetID()), { 300 * aspect, 300 }, { 0, 1 }, { 1, 0 });

		//if (m_BloomView == nullptr)
		//	m_BloomView = Engine::CreateRef<Engine::Texture2DImageView>(m_BloomComputeTextures[2], 0, 1);
		//ImGui::SliderInt("Mip Level", &m_ViewMipLevel, 0, m_BloomComputeTextures[2]->GetMipLevelCount() - 1);
		//
		//if (m_ViewMipLevel != m_BloomView->GetBaseMip())
		//	m_BloomView->ChangeToMip(m_BloomComputeTextures[2], m_ViewMipLevel, 1);
		//
		//ImGui::Image((ImTextureID)(m_BloomView->GetID()), { 300 * aspect, 300 }, { 0, 1 }, { 1, 0 });

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


bool BloomDemoLayer::OnKeyPressed(Engine::KeyPressedEvent& keyPressedEvent)
{
	if (m_Curves.size() > 0)
	{
		auto curve = m_Curves[m_CurveEditIndex];

		if (keyPressedEvent.GetKeyCode() == Engine::Key::Space)
			curve->Clear();
	}

	return true;
}

bool BloomDemoLayer::OnMouseButtonReleased(Engine::MouseButtonReleasedEvent& releasedEvent)
{
	if (m_IsDragging && releasedEvent.GetButton() == Engine::Mouse::ButtonLeft)
	{
		m_IsDragging = false;
		m_DragID = -1;
	}

	m_Surface->StopPicking(releasedEvent.GetButton());

	return true;
}

bool BloomDemoLayer::OnMouseButtonPressed(Engine::MouseButtonPressedEvent& pressedEvent)
{
	if (m_Curves.size() > 0 && !m_IsDragging && pressedEvent.GetButton() == Engine::Mouse::ButtonLeft)
	{
		auto curve = m_Curves[m_CurveEditIndex];
		glm::vec2 mousePosition = Engine::Input::GetMousePosition();
		glm::vec3 worldCoordinate = m_Camera.ScreenToWorldPoint({ mousePosition.x, mousePosition.y, 0.0f });

		for (uint32_t i = 0; i < curve->GetPointCount(); i++)
		{
			Engine::LineVertex vertex = (*curve)[i];
			glm::vec3 transformOffset = curve->GetTransform()->GetPosition();
			transformOffset.z = 0.0f;

			glm::vec3 toWorld = glm::vec3(curve->GetTransform()->Transform() * glm::vec4(vertex.Position, 1.0));

			glm::vec3 offset = toWorld - worldCoordinate;

			if (glm::dot(offset, offset) < glm::dot(0.04f, 0.04f))
			{
				m_IsDragging = true;
				m_DragID = i;
			}
		}
	}

	if (m_Curves.size() > 0 && pressedEvent.GetButton() == Engine::Mouse::ButtonRight && Engine::Input::IsKeyPressed(Engine::Key::LeftShift))
	{
		auto curve = m_Curves[m_CurveEditIndex];

		glm::vec2 mousePosition = Engine::Input::GetMousePosition();
		glm::vec3 worldCoordinate = m_Camera.ScreenToWorldPoint({ mousePosition.x, mousePosition.y, 0.0f });
		curve->AddSegment({ worldCoordinate });
	}

	m_Surface->StartPicking(pressedEvent.GetButton(), m_Camera);

	return true;
}

void BloomDemoLayer::OnResize()
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
