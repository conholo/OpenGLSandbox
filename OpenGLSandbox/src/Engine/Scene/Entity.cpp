#include "epch.h"
#include "Engine/Scene/Entity.h"

namespace Engine
{
	Entity::Entity(entt::entity handle, Scene* scene)
		:m_EntityHandle(handle), m_Scene(scene)
	{
	}
}