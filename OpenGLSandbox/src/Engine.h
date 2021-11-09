

//--------------------- CORE ---------------------//
#include "Engine/Core/Math.h"
#include "Engine/Core/Application.h"
#include "Engine/Core/Input.h"
#include "Engine/Core/Layer.h"
#include "Engine/Core/Time.h"
#include "Engine/Core/Buffer.h"
#include "Engine/Core/Utility.h"
#include "Engine/Core/Random.h"
#include "Engine/Event/KeyEvent.h"
#include "Engine/Event/MouseEvent.h"
#include "Engine/Event/WindowEvent.h"
//--------------------- CORE ---------------------//

//--------------------- Simple ECS ---------------------//
#include "Engine/Scene/SimpleECS/SimpleEntity.h"
#include "Engine/Scene/SimpleECS/Light.h"
// This is in Simple ECS because it depends on Entity Transform.
// Needs the updated ECS equivalent.
#include "Engine/Rendering/Line.h"
//--------------------- Simple ECS ---------------------//


//--------------------- Scene ---------------------//
#include "Engine/Scene/Component.h"
#include "Engine/Scene/Entity.h"
#include "Engine/Scene/Scene.h"
//--------------------- Scene ---------------------//


//--------------------- RENDERING ---------------------//
#include "Engine/Rendering/VertexArray.h"
#include "Engine/Rendering/Shader.h"
#include "Engine/Rendering/RenderCommand.h"
#include "Engine/Rendering/ShaderStorageBuffer.h"
#include "Engine/Rendering/Mesh.h"
#include "Engine/Rendering/Texture.h"
#include "Engine/Rendering/CubeMap.h"
#include "Engine/Rendering/Camera.h"
#include "Engine/Rendering/EditorGrid.h"
#include "Engine/Rendering/FrameBuffer.h"
#include "Engine/Rendering/Renderer.h"
#include "Engine/Rendering/SceneRenderer.h"
#include "Engine/Rendering/UniformBuffer.h"
//--------------------- RENDERING ---------------------//