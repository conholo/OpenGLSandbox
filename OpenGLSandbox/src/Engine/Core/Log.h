#pragma once
#include <spdlog/spdlog.h>
#include "glm/gtx/string_cast.hpp"

namespace Engine
{
	class Log
	{
	public:
		static void Init();
		static Ref<spdlog::logger>& GetLogger() { return s_Logger; }

		template<typename... Args>
		static void PrintAssertMessage(std::string_view Prefix, Args&&... args);

	private:
		static Ref<spdlog::logger> s_Logger;
	};
}

template<typename OStream>
OStream& operator<<(OStream& os, const glm::vec3& vec)
{
	return os << '(' << vec.x << ", " << vec.y << ", " << vec.z << ')';
}

template<typename OStream>
OStream& operator<<(OStream& os, const glm::vec4& vec)
{
	return os << '(' << vec.x << ", " << vec.y << ", " << vec.z << ", " << vec.w << ')';
}


namespace Engine
{
	template<typename... Args>
	void Log::PrintAssertMessage(std::string_view Prefix, Args&&... args)
	{
		GetLogger()->error("{0}: {1}", Prefix, fmt::format(std::forward<Args>(args)...));
	}
}


#define LOG_TRACE(...)		::Engine::Log::GetLogger()->trace(__VA_ARGS__)
#define LOG_INFO(...)		::Engine::Log::GetLogger()->info(__VA_ARGS__)
#define LOG_WARN(...)		::Engine::Log::GetLogger()->warn(__VA_ARGS__)
#define LOG_ERROR(...)		::Engine::Log::GetLogger()->error(__VA_ARGS__)
#define LOG_CRITICAL(...)	::Engine::Log::GetLogger()->critical(__VA_ARGS__)
