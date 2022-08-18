#pragma once
#include "Log.h"

#define ENABLE_ASSERTS
#define DEBUG_BREAK __debugbreak()

#ifdef ENABLE_ASSERTS
    #define ASSERT_MESSAGE_INTERNAL(...)  ::Engine::Log::PrintAssertMessage("Assertion Failed", __VA_ARGS__)
    #define ASSERT(condition, ...) { if(!(condition)) { ASSERT_MESSAGE_INTERNAL(__VA_ARGS__); DEBUG_BREAK; } }
#else
    #define ASSERT(condition, ...)
#endif

