#include "Engine/Scene/Scene.h"
#include "Engine/Scene/Entity.h"
#include "Engine/Rendering/Renderer.h"

#include <string>

namespace Engine
{
	Scene::Scene(const std::string& name)
		: m_SceneName(name)
	{
	}

	Entity Scene::Create(const std::string& name)
	{
		entt::entity id = m_Registry.create();
		Entity entity = { id, this };

		entity.AddComponent<TagComponent>(name);
		entity.AddComponent<TransformComponent>();

		return entity;
	}

	bool Scene::Destroy(Entity entity)
	{
		if (m_Registry.valid(entity))
		{
			m_Registry.destroy(entity);
			return true;
		}

		return false;
	}


}