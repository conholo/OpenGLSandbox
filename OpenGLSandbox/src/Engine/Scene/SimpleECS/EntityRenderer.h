#pragma once

#include "Engine/Rendering/Camera.h"
#include "Engine/Rendering/Material.h"
#include "Engine/Rendering/Mesh.h"
#include "Engine/Rendering/Shader.h"
#include "Engine/Rendering/VertexArray.h"

namespace Engine
{
	class EntityTransform;
	class Light;

	class EntityRenderer
	{
	public:
		EntityRenderer(PrimitiveType primitiveType, const std::string& shaderName, const std::string& entityName = "Entity");
		EntityRenderer(const Ref<Mesh>& mesh, const std::string& shaderName, const std::string& entityName = "Entity");
		EntityRenderer(const Ref<Mesh>& mesh);

		void Begin() const;
		void Draw() const;
		void End() const;
		void DrawPoints() const;
		void Draw(const Camera& Camera, const Ref<Light>& DirectionalLight, const Ref<EntityTransform>& Transform) const;

		void Bind() const;
		void Unbind() const;
		void SetPrimitiveType(PrimitiveType type);

		const Ref<Mesh>& GetMesh() const { return m_Mesh; }
		const Ref<Shader>& GetShader() const { return m_Material->GetShader(); }
		Material& GetMaterial() const { return *m_Material; }

		static float s_EnvironmentMapIntensity;

	private:
		static void StageSceneUniforms(Material& Material, const Camera& Camera, const Ref<Light>& DirectionalLight, const glm::mat4& modelMatrix);
		void Initialize();

	private:
		Ref<Material> m_Material;
		Ref<VertexArray> m_VertexArray;
		Ref<VertexBuffer> m_VertexBuffer;
		Ref<IndexBuffer> m_IndexBuffer;
		Ref<Mesh> m_Mesh;
	};
}
