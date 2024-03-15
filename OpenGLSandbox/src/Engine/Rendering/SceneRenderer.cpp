#include "epch.h"
#include "Engine/Rendering/SceneRenderer.h"
#include "Engine/Rendering/Renderer.h"
#include "Engine/Rendering/FrameBuffer.h"
#include "Engine/Core/Application.h"
#include "Engine/Rendering/Shader.h"
#include "Engine/Rendering/RenderCommand.h"
#include "Engine/Scene/Component.h"

#include <glm/glm.hpp>

namespace Engine
{
	Ref<Scene> SceneRenderer::s_ActiveScene = nullptr;

	Camera SceneRenderer::s_Camera(45.0f, 1920.0f/1080.0f, 0.1f, 1000.0f);

	Ref<RenderPass> SceneRenderer::s_GeometryPass = nullptr;
	Ref<RenderPass> SceneRenderer::s_ShadowPass = nullptr;
	Ref<Texture2D> SceneRenderer::s_WhiteTexture = nullptr;
	Ref<UniformBuffer> SceneRenderer::s_ShadowUniformbuffer = nullptr;

	void SceneRenderer::LoadScene(const Ref<Scene>& runtimeScene)
	{
		s_ActiveScene = runtimeScene;
	}

	void SceneRenderer::UnloadScene()
	{
		s_ActiveScene = nullptr;
	}

	void SceneRenderer::InitializeShadowPass()
	{
		FramebufferSpecification shadowFrameBufferSpec;

		const uint32_t SHADOW_WIDTH = 2048, SHADOW_HEIGHT = 2048;
		const uint32_t SHADOW_UNIFORM_BLOCK_BINDING = 2;

		shadowFrameBufferSpec.AttachmentSpecification = { FramebufferTextureFormat::Depth };
		shadowFrameBufferSpec.Width = SHADOW_WIDTH;
		shadowFrameBufferSpec.Height = SHADOW_HEIGHT;

		RenderPassSpecification shadowRenderPassSpec;
		shadowRenderPassSpec.ColorWrite = false;
		shadowRenderPassSpec.ClearColor = glm::vec4(0.0f);
		shadowRenderPassSpec.TargetFramebuffer = CreateRef<Framebuffer>(shadowFrameBufferSpec);
		shadowRenderPassSpec.RenderPassShader = ShaderLibrary::Get("Depth");
		shadowRenderPassSpec.Flags |= (uint32_t)RenderFlag::DepthTest;
		shadowRenderPassSpec.Flags |= (uint32_t)RenderFlag::Blend;

		s_ShadowUniformbuffer = CreateRef<UniformBuffer>(sizeof(glm::mat4), SHADOW_UNIFORM_BLOCK_BINDING);
		s_ShadowPass = CreateRef<RenderPass>(shadowRenderPassSpec);
	}

	void SceneRenderer::InitializeGeometryPass()
	{
		RenderPassSpecification geoRenderPassSpec;
		geoRenderPassSpec.ClearColor = glm::vec4(0.3f, 0.3f, 0.3f, 0.0f);
		geoRenderPassSpec.Flags |= (uint32_t)RenderFlag::DepthTest;
		geoRenderPassSpec.Flags |= (uint32_t)RenderFlag::Blend;
		geoRenderPassSpec.Type = PassType::DefaultFBO;
		s_WhiteTexture = Texture2D::CreateWhiteTexture();

		s_GeometryPass = CreateRef<RenderPass>(geoRenderPassSpec);
		s_GeometryPass->GetRenderPassSpecification().RenderPassShader = ShaderLibrary::Get("EngineBP");
	}

	void SceneRenderer::ShadowPass()
	{
		Renderer::BeginPass(s_ShadowPass);

		std::pair<TransformComponent, LightComponent> directionalLightComponents = s_ActiveScene->GetDirectionalLight();

		glm::mat4 lightProjection = glm::ortho(-20.0f, 20.0f, -20.0f, 20.0f, 1.0f, 20.0f);
		glm::mat4 lightView = glm::lookAt(directionalLightComponents.first.Translation, directionalLightComponents.first.Forward(), directionalLightComponents.first.Up());
		glm::mat4 shadowMatrix = lightProjection * lightView;

		s_ShadowUniformbuffer->SetData((void*)&shadowMatrix, sizeof(shadowMatrix));

		auto group = s_ActiveScene->m_Registry.group<TransformComponent, MeshRendererComponent>();

		s_ShadowPass->GetRenderPassSpecification().RenderPassShader->Bind();

		RenderCommand::SetFaceCullMode(FaceCullMode::Front);

		for (auto entity : group)
		{
			auto [transform, meshRenderer] = group.get<TransformComponent, MeshRendererComponent>(entity);
			Renderer::UploadCameraUniformData(s_Camera, transform);
			Renderer::DrawPrimitive(meshRenderer.Type);
		}

		RenderCommand::SetFaceCullMode(FaceCullMode::Back);

		s_ShadowPass->GetRenderPassSpecification().RenderPassShader->Unbind();
		s_ShadowPass->GetRenderPassSpecification().TargetFramebuffer->BindDepthTexture(1);
		Renderer::EndPass(s_ShadowPass);
	}

	void SceneRenderer::GeometryPass()
	{
		Renderer::BeginPass(s_GeometryPass);

		// Debug Depth Draw to Quad
		{
			Ref<VertexArray> vao = CreateRef<VertexArray>();
			Ref<Mesh> quad = MeshFactory::FullScreenQuad();
			float* baseVertexPtr = &quad->GetVertices().data()->Position.x;
			Ref<VertexBuffer> vbo = CreateRef<VertexBuffer>(baseVertexPtr, quad->GetVertices().size() * sizeof(Vertex));

			BufferLayout layout = BufferLayout
			(
				{
					{ "a_Position", ShaderAttributeType::Float3 },
					{ "a_TexCoord", ShaderAttributeType::Float2 },
					{ "a_Normal",	ShaderAttributeType::Float3 },
				}
			);

			vbo->SetLayout(layout);

			uint32_t* indexPtr = quad->GetIndices().data();
			Ref<IndexBuffer> ebo = CreateRef<IndexBuffer>(indexPtr, quad->GetIndices().size());

			glm::mat4 quadTransform = glm::translate(glm::mat4(1.0), { 0.0f, 5.0f, 0.0f });

			ShaderLibrary::Get("DebugDepth")->Bind();
			ShaderLibrary::Get("DebugDepth")->UploadUniformInt("u_DepthMap", 1);
			ShaderLibrary::Get("DebugDepth")->UploadUniformMat4("u_MVP", s_Camera.GetViewProjection() * quadTransform);
			ShaderLibrary::Get("DebugDepth")->UploadUniformFloat("u_NearPlane", 1.0f);
			ShaderLibrary::Get("DebugDepth")->UploadUniformFloat("u_FarPlane", 20.0f);

			vao->EnableVertexAttributes(vbo);
			vao->SetIndexBuffer(ebo);

			RenderCommand::DrawIndexed(vao);
			vao->Unbind();
		}

		auto group = s_ActiveScene->m_Registry.group<TransformComponent, MeshRendererComponent>();
		std::pair<TransformComponent, LightComponent> directionalLightComponents = s_ActiveScene->GetDirectionalLight();

		s_WhiteTexture->BindToSamplerSlot(0);
		s_GeometryPass->GetRenderPassSpecification().RenderPassShader->Bind();
		s_GeometryPass->GetRenderPassSpecification().RenderPassShader->UploadUniformInt("u_Texture", 0);
		s_GeometryPass->GetRenderPassSpecification().RenderPassShader->UploadUniformInt("u_ShadowMap", 1);
		s_GeometryPass->GetRenderPassSpecification().RenderPassShader->UploadUniformFloat3("u_DirectionalLight.Position", directionalLightComponents.first.Translation);
		s_GeometryPass->GetRenderPassSpecification().RenderPassShader->UploadUniformFloat3("u_DirectionalLight.Color", directionalLightComponents.second.Color);
		s_GeometryPass->GetRenderPassSpecification().RenderPassShader->UploadUniformFloat("u_DirectionalLight.Intensity", directionalLightComponents.second.Intensity);
		s_GeometryPass->GetRenderPassSpecification().RenderPassShader->UploadUniformFloat3("u_CameraPosition", s_Camera.GetPosition());

		for (auto entity : group)
		{
			auto [transform, meshRenderer] = group.get<TransformComponent, MeshRendererComponent>(entity);
			UploadMaterialProperties(meshRenderer.Properties);
			Renderer::UploadCameraUniformData(s_Camera, transform);

			Renderer::DrawPrimitive(meshRenderer.Type);
		}

		s_GeometryPass->GetRenderPassSpecification().RenderPassShader->Unbind();

		Renderer::EndPass(s_GeometryPass);
	}

	void SceneRenderer::InitializePipeline()
	{
		InitializeShadowPass();
		InitializeGeometryPass();
	}

	void SceneRenderer::SubmitPipeline()
	{
		ShadowPass();
		GeometryPass();
	}

	void SceneRenderer::UpdateCamera(float deltaTime)
	{
		s_Camera.Update(deltaTime);
	}

	void SceneRenderer::OnEvent(Event& e)
	{
		s_Camera.OnEvent(e);
	}

	void SceneRenderer::ValidateResize(glm::vec2 viewportSize)
	{
		if (s_GeometryPass == nullptr || s_GeometryPass->GetRenderPassSpecification().TargetFramebuffer == nullptr)
		{
			return;
		}

		if (FramebufferSpecification specification = s_GeometryPass->GetRenderPassSpecification().TargetFramebuffer->GetFramebufferSpecification();
			viewportSize.x > 0.0f && viewportSize.y > 0.0f &&
			(specification.Width != viewportSize.x || specification.Height != viewportSize.y))
		{
			s_GeometryPass->GetRenderPassSpecification().TargetFramebuffer->Resize((uint32_t)viewportSize.x, (uint32_t)viewportSize.y);
			s_Camera.SetViewportSize(viewportSize.x, viewportSize.y);
		}
	}

	void SceneRenderer::UploadMaterialProperties(const RendererMaterialProperties& properties)
	{
		s_GeometryPass->GetRenderPassSpecification().RenderPassShader->UploadUniformFloat3("u_MaterialProperties.AmbientColor", properties.AmbientColor);
		s_GeometryPass->GetRenderPassSpecification().RenderPassShader->UploadUniformFloat3("u_MaterialProperties.DiffuseColor", properties.DiffuseColor);
		s_GeometryPass->GetRenderPassSpecification().RenderPassShader->UploadUniformFloat("u_MaterialProperties.AmbientStrength", properties.AmbientStrength);
		s_GeometryPass->GetRenderPassSpecification().RenderPassShader->UploadUniformFloat("u_MaterialProperties.DiffuseStrength", properties.DiffuseStrength);
		s_GeometryPass->GetRenderPassSpecification().RenderPassShader->UploadUniformFloat("u_MaterialProperties.SpecularStrength", properties.SpecularStrength);
		s_GeometryPass->GetRenderPassSpecification().RenderPassShader->UploadUniformFloat("u_MaterialProperties.Shininess", properties.Shininess);
	}
}