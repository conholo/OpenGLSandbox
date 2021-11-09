#include "Engine/Rendering/Renderer.h"
#include "Engine/Rendering/RenderCommand.h"
#include "Engine/Rendering/FrameBuffer.h"
#include "Engine/Rendering/VertexArray.h"
#include "Engine/Core/Application.h"
#include "Engine/Scene/Component.h"
#include "Engine/Rendering/UniformBuffer.h"

#include <iostream>
#include <sstream>
#include <unordered_map>

namespace Engine
{
	struct RendererData
	{
		Ref<VertexArray> VAO;
		Ref<VertexBuffer> VBO = nullptr;
		Ref<IndexBuffer> EBO = nullptr;
		std::unordered_map<PrimitiveType, Ref<Mesh>> Primitives;

		struct GlobalData
		{
			float ElapsedTime;
			float DeltaTime;
		};

		struct CameraData
		{
			glm::mat4 u_ViewProjectionMatrix;
			glm::mat4 u_ModelMatrix;
			glm::mat4 u_NormalMatrix;
		};

		Ref<UniformBuffer> CameraBuffer;
		Ref<UniformBuffer> GlobalBuffer;
	};

	static RendererData* s_RenderData = nullptr;

	void Renderer::UploadCameraUniformData(const Camera& camera, const TransformComponent& transform)
	{
		glm::mat4 normalMatrix = glm::transpose(glm::inverse(transform.Transform()));
		glm::mat4 viewProjection = camera.GetViewProjection();

		RendererData::CameraData cameraData{ viewProjection, transform.Transform(), normalMatrix };
		s_RenderData->CameraBuffer->SetData(&cameraData, sizeof(RendererData::CameraData));
	}

	void Renderer::Initialize()
	{
		s_RenderData = new RendererData;

		s_RenderData->VAO = CreateRef<VertexArray>();

		s_RenderData->GlobalBuffer = CreateRef<UniformBuffer>(sizeof(RendererData::GlobalBuffer), 0);
		s_RenderData->CameraBuffer = CreateRef<UniformBuffer>(sizeof(RendererData::CameraData), 1);

		s_RenderData->Primitives[PrimitiveType::Cube] = MeshFactory::Create(PrimitiveType::Cube);
		s_RenderData->Primitives[PrimitiveType::Quad] = MeshFactory::Create(PrimitiveType::Quad);
		s_RenderData->Primitives[PrimitiveType::FullScreenQuad] = MeshFactory::Create(PrimitiveType::FullScreenQuad);
		s_RenderData->Primitives[PrimitiveType::Sphere] = MeshFactory::Create(PrimitiveType::Sphere);
		s_RenderData->Primitives[PrimitiveType::Plane] = MeshFactory::Create(PrimitiveType::Plane);
		s_RenderData->Primitives[PrimitiveType::Triangle] = MeshFactory::Create(PrimitiveType::Triangle);
		s_RenderData->Primitives[PrimitiveType::TessellatedQuad] = MeshFactory::Create(PrimitiveType::TessellatedQuad);
	}

	void Renderer::BeginPass(const Ref<RenderPass>& renderPass)
	{
		RenderCommand::SetFlags(renderPass->GetRenderPassSpecification().Flags);

		if (renderPass->GetRenderPassSpecification().Type != PassType::DefaultFBO)
		{
			Ref<Framebuffer> passFB = renderPass->GetRenderPassSpecification().TargetFramebuffer;
			passFB->Bind();
		}
		else
			RenderCommand::SetViewport(Application::GetApplication().GetWindow().GetWidth(), Application::GetApplication().GetWindow().GetHeight());

		auto& specification = renderPass->GetRenderPassSpecification();

		RenderCommand::ClearColor(specification.ClearColor);
		RenderCommand::Clear(specification.ColorWrite, specification.DepthRead);
	}

	void Renderer::EndPass(const Ref<RenderPass>& renderPass)
	{
		if (renderPass->GetRenderPassSpecification().Type == PassType::DefaultFBO) return;

		renderPass->GetRenderPassSpecification().TargetFramebuffer->Unbind();
	}

	void Renderer::Begin()
	{
		s_RenderData->VAO->ClearIndexBuffer();
		s_RenderData->VBO = nullptr;
		s_RenderData->EBO = nullptr;
	}

	void Renderer::End()
	{
	}

	void Renderer::DrawPrimitive(PrimitiveType type)
	{
		s_RenderData->VAO->Bind();
		CreateRenderPrimitivesForMesh(type);
		RenderCommand::DrawIndexed(s_RenderData->VAO);
		s_RenderData->VAO->Unbind();
	}

	void Renderer::DrawFullScreenQuad()
	{

	}

	void Renderer::Shutdown()
	{
		delete s_RenderData;
	}

	void Renderer::CreateRenderPrimitivesForMesh(PrimitiveType type)
	{
		// Setup Render Primitives for mesh
		s_RenderData->VAO->ClearIndexBuffer();
		s_RenderData->EBO = nullptr;
		s_RenderData->VBO = nullptr;

		Ref<Mesh> mesh = s_RenderData->Primitives[type];
		float* baseVertexPtr = &mesh->GetVertices().data()->Position.x;
		s_RenderData->VBO = CreateRef<VertexBuffer>(baseVertexPtr, mesh->GetVertices().size() * sizeof(Vertex));

		BufferLayout layout = BufferLayout
		(
			{
				{ "a_Position", ShaderAttributeType::Float3 },
				{ "a_TexCoord", ShaderAttributeType::Float2 },
				{ "a_Normal",	ShaderAttributeType::Float3 },
			}
		);

		s_RenderData->VBO->SetLayout(layout);

		uint32_t* indexPtr = mesh->GetIndices().data();
		s_RenderData->EBO = CreateRef<IndexBuffer>(indexPtr, mesh->GetIndices().size());

		s_RenderData->VAO->EnableVertexAttributes(s_RenderData->VBO);
		s_RenderData->VAO->SetIndexBuffer(s_RenderData->EBO);
	}
}