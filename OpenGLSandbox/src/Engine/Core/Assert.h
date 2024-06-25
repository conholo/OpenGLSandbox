#pragma once

#include "Utility.h"
#include "Log.h"

#define ENABLE_ASSERTS

#if defined(E_PLATFORM_WINDOWS)
    #define E_DEBUGBREAK() __debugbreak()
#elif defined(E_PLATFORM_LINUX)
    #include <csignal>
    #define E_DEBUGBREAK() raise(SIGTRAP)
#elif defined(E_PLATFORM_MACOS)
    #define E_DEBUGBREAK() __builtin_trap()
#else
    #error "Platform doesn't support debugbreak yet!"
#endif

#ifdef ENABLE_ASSERTS
#define ASSERT_MESSAGE_INTERNAL(...)                                                            \
    ::Engine::Log::PrintAssertMessage("Assertion Failed", __VA_ARGS__)
#define ASSERT(condition, ...)                                                                  \
{                                                                                               \
    if (!(condition))                                                                           \
    {                                                                                           \
        ASSERT_MESSAGE_INTERNAL(__VA_ARGS__);                                                   \
        E_DEBUGBREAK();                                                                         \
    }                                                                                           \
}
#else
    #define ASSERT(condition, ...)
#endif