#pragma once

#include "Engine/Core/Memory.h"
#include "Engine/Scene/Component.h"
#include "Engine/Rendering/Camera.h"

#include <utility>
#include <entt/entt.hpp>
#include <glm/glm.hpp>

namespace Engine
{
	class Entity;

	class Scene
	{
	public:
		Scene(const std::string& name = "Sample Scene");
		Entity Create(const std::string& name = "Entity");
		bool Destroy(Entity entity);
		const std::string& GetName() const { return m_SceneName; }

		void RegisterDirectionalLight(uint32_t id) { m_DirectionalLightID = id; }
		std::pair<TransformComponent, LightComponent> GetDirectionalLight() 
		{
			TransformComponent transform = m_Registry.get<TransformComponent>((entt::entity)m_DirectionalLightID);
			LightComponent light = m_Registry.get<LightComponent>((entt::entity)m_DirectionalLightID);

			return std::make_pair(transform, light);
		}

	private:
		std::string m_SceneName;
		entt::registry m_Registry;
		uint32_t m_DirectionalLightID;

		friend class Entity;
		friend class SceneRenderer;
	};
}