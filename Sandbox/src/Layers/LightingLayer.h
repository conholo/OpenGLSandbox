#pragma once

#include "Engine.h"
#include <vector>
#include <glm/glm.hpp>
#include <functional>

using CatWalkFn = std::function<bool(Engine::Ref<Engine::SimpleEntity>, float)>;

struct BlinnPhongMaterialProperties
{
	std::string ShaderName;
	glm::vec3 AmbientColor = glm::vec3(1.0f, 1.0f, 1.0f);
	glm::vec3 DiffuseColor = glm::vec3(1.0f, 1.0f, 1.0f);
	float AmbientStrength = 1.0f;
	float DiffuseStrength = 1.0f;
	float SpecularStrength = 1.0f;
	float Shininess = 1.0f;
	int	IsSmooth = 1;
};

struct SDFEntity
{
	SDFEntity() = default;
	SDFEntity(const Engine::Ref<Engine::SimpleEntity>& entity, const BlinnPhongMaterialProperties& properties, const glm::vec3& startingPositon)
		: SceneSDFEntity(entity), MaterialProperties(properties), StartingPosition(startingPositon) 
	{
		entity->GetEntityTransform()->SetPosition(startingPositon);
		entity->GetEntityTransform()->SetScale({4.0f, 4.0f, 4.0f});
	}

	Engine::Ref<Engine::SimpleEntity> SceneSDFEntity;
	BlinnPhongMaterialProperties MaterialProperties;
	glm::vec3 StartingPosition;
};

struct CatWalkData
{
	CatWalkData() = default;
	CatWalkData(const Engine::Ref<Engine::SimpleEntity>& entity, BlinnPhongMaterialProperties properties, const CatWalkFn& fn, const glm::vec3& startingPositon):
		CatWalkEntity(entity), MaterialProperties(properties), Fn(fn), StartingPosition(startingPositon) 
	{
		entity->GetEntityTransform()->SetPosition(startingPositon);
	}

	bool Execute(float percent)
	{
		return Fn(CatWalkEntity, percent);
	}

	Engine::Ref<Engine::SimpleEntity> CatWalkEntity;
	BlinnPhongMaterialProperties MaterialProperties;
	CatWalkFn Fn;
	glm::vec3 StartingPosition;
};

class LightingLayer : public Engine::Layer
{
public:
	LightingLayer();
	~LightingLayer();

	void OnAttach() override;
	void OnDetach() override;
	void OnUpdate(float deltaTime) override;
	void OnEvent(Engine::Event& event) override;

private:
	void PerformCatwalkRoutine();
	void UploadLights(float x, float z);
	void DrawEntities();
	void DrawSDFEntities();
	void DrawDebugLights();
	bool OnKeyPressed(Engine::KeyPressedEvent& event);

private:
	bool WalkUpComplete = false;
	bool ActionComplete = false;
	bool WalkBackComplete = false;
	bool m_RenderDebugLights = false;
	bool m_MovePointLights = false;
	bool m_GroundPlaneHasTexture = true;
	bool m_UseDirectional = true;
	bool m_UsePointLights = true;
	bool m_UseSpotLights = true;
	float m_Counter = 0;
	float m_CatWalkCounter = 0;
	Engine::Ref<Engine::Texture2D> m_WhiteTexture;
	Engine::Ref<Engine::Texture2D> m_MapTexture;
	Engine::Ref<Engine::Texture2D> m_GroundTexture;
	Engine::Ref<Engine::Texture2D> m_WallTexture;
	Engine::Camera m_Camera;


	Engine::Ref<Engine::Light> m_DirectionalLight;
	uint32_t m_SpotLightCount = 10;
	std::vector<Engine::Ref<Engine::Light>> m_SpotLights;
	std::vector<glm::vec3> m_SpotLightFocalPoints;
	std::vector<glm::vec3> m_PointLightPositions;

	uint32_t m_PointLightCount = 15;
	std::vector<Engine::Ref<Engine::Light>> m_PointLights;

	glm::vec3 m_SphereOffset = glm::vec3(0.0f);

	uint32_t m_CatWalkPointer = 0;
	std::vector<CatWalkData> m_CatWalkEntities;
	std::vector<SDFEntity> m_SDFEntities;

	Engine::Ref<Engine::SimpleEntity> m_DiscoBall;
	Engine::Ref<Engine::SimpleEntity> m_Plane;
	Engine::Ref<Engine::SimpleEntity> m_Wall;
	Engine::Ref<Engine::SimpleEntity> m_DisplayCube;
	Engine::Ref<Engine::SimpleEntity> m_DisplaySphere;
	Engine::Ref<Engine::SimpleEntity> m_TexturedDisplaySphere;

	BlinnPhongMaterialProperties m_FlatShadedShinyProperties;
	BlinnPhongMaterialProperties m_SmoothProperties;
	BlinnPhongMaterialProperties m_RoughProperties;
};