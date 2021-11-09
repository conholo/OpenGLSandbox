#pragma once

#include "Engine/Scene/Scene.h"
#include <entt/entt.hpp>

namespace Engine
{
	class Entity
	{
	public:
		Entity() = default;
		Entity(const Entity&) = default;
		Entity(entt::entity handle, Scene* scene);


		template<typename T, typename ... Args>
		T& AddComponent(Args&& ... args)
		{
			T& component = m_Scene->m_Registry.emplace<T>(m_EntityHandle, std::forward<Args>(args)...);
			return component;
		}

		template<typename T>
		T& GetComponent()
		{
			return  m_Scene->m_Registry.get<T>(m_EntityHandle);;
		}

		template<typename T>
		bool HasComponent()
		{
			return m_Scene->m_Registry.has<T>(m_EntityHandle);
		}

		template<typename T>
		bool RemoveComponent()
		{
			if (typeid(T).name() == typeid(TransformComponent).name())
			{
				return false;
			}

			if (HasComponent<T>())
			{
				m_Scene->m_Registry.remove<T>(m_EntityHandle);
				return true;
			}

			return false;
		}

		bool operator ==(const Entity& other) const
		{
			return m_EntityHandle == other.m_EntityHandle && m_Scene == other.m_Scene;
		}

		bool operator !=(const Entity& other) const
		{
			return !(*this == other);
		}

		operator bool() const { return m_EntityHandle != entt::null; }
		operator uint32_t() const { return (uint32_t)m_EntityHandle; }
		operator entt::entity() const { return m_EntityHandle; }

	private:
		Scene* m_Scene = nullptr;
		entt::entity m_EntityHandle{ entt::null };
	};
}