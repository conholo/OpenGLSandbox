#pragma once
#include "Engine/Core/Memory.h"
#include "Engine/Scene/SimpleECS/EntityTransform.h"
#include "Engine/Scene/SimpleECS/EntityRenderer.h"
#include "Engine/Rendering/Mesh.h"
#include "Engine/Rendering/UniformBuffer.h"

#include <string>
#include <stdint.h>
#include <glm/glm.hpp>

struct PlanetMaterialProperties
{
	glm::vec4 Diffuse{ 0.0f, 0.0f, 0.0f, 0.1f };
	glm::vec4 Ambient{ 0.3f, 0.3f, 0.3f, 0.5f };
	float SpecularStrength = 1.0f;
	float Shininess = 32.0f;
};

struct PlanetProperties
{
	std::string Name = "Planet";
	glm::vec3 Position{ 0.0f, 0.0f, 0.0f };
	glm::vec3 Scale{ 1.0f, 1.0f, 1.0f };
	float Mass = 100.0f;
	float Radius = 10.0f;
	float AtmosphereRadius = 15.0f;
	float Gravity = -9.81f;
};

class Planet
{
public:
	Planet(const PlanetProperties& properties = PlanetProperties());
	~Planet();

	const Engine::Ref<Engine::EntityTransform>& GetTransform() const { return m_Transform; }
	const Engine::Ref<Engine::EntityRenderer>& GetRenderer() const { return m_Renderer; }

	void SetRadius(float radius);

	PlanetMaterialProperties& GetMaterialProperties() { return m_MaterialProperties; }
	const PlanetProperties& GetProperties() const { return m_Properties; }
	PlanetProperties& GetProperties() { return m_Properties; }

	void UpdateMesh(int resolution);
	void Draw(const glm::mat4& viewProjection);

private:
	Engine::Ref<Engine::UniformBuffer> m_MaterialPropertiesUBO;
	Engine::Ref<Engine::EntityRenderer> m_Renderer;
	Engine::Ref<Engine::Mesh> m_PlanetMesh;
	Engine::Ref<Engine::EntityTransform> m_Transform;
	PlanetMaterialProperties m_MaterialProperties;
	PlanetProperties m_Properties;
};