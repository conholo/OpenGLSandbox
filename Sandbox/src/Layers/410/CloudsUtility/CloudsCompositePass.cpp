#include "Layers/410/CloudsUtility/CloudsCompositePass.h"

#include "Engine/Core/Memory.h"
#include "Engine/Scene/SimpleECS/SimpleEntity.h"
#include "Engine/Rendering/RenderCommand.h"
#include "Engine/Rendering/Camera.h"

CloudsCompositePass::CloudsCompositePass()
{
	m_CompositeQuad = Engine::CreateRef<Engine::SimpleEntity>(Engine::PrimitiveType::FullScreenQuad, "CloudsComposite");
}

CloudsCompositePass::~CloudsCompositePass()
{

}

void CloudsCompositePass::ExecutePass(const Engine::Camera& camera, const Engine::Ref<MainCloudRenderPass>& mainRenderPass)
{
	Engine::RenderCommand::ClearColor(m_ClearColor);
	Engine::RenderCommand::Clear(true, true);
	mainRenderPass->BindMainColorAttachment(0);
	m_CompositeQuad->GetEntityRenderer()->GetShader()->Bind();
	m_CompositeQuad->GetEntityRenderer()->GetShader()->UploadUniformInt("u_SceneTexture", 0);
	m_CompositeQuad->DrawEntity(camera.GetViewProjection());
}
