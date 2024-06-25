#pragma once
#include "Engine/Core/PlatformDetection.h"

#include <string>

#if defined(__clang__) || defined(__GNUG__)
    #include <cxxabi.h>
#endif

#define BIND_FN(fn)                                             \
[this](auto&&... args) -> decltype(auto) {                  \
return this->fn(std::forward<decltype(args)>(args)...); \
}

template <typename T>
std::string TypeName()
{
    std::string TypeName = typeid(T).name();
#if defined(__clang__) || defined(__GNUG__)
    int	  status;
    char* demangled_name = abi::__cxa_demangle(TypeName.c_str(), nullptr, nullptr, &status);
    if (status == 0)
    {
        TypeName = demangled_name;
        std::free(demangled_name);
    }
#endif
    return TypeName;
}

#include "Engine/Core/Memory.h"
#include "Engine/Core/Log.h"
#include "Engine/Core/Assert.h"