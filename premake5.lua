workspace "OpenGLSandbox"
    configurations { "Debug", "Release", "Dist" }
    targetdir "build"
	startproject "Sandbox"

	filter "language:C++ or language:C"
		architecture "x86_64"
	filter ""

	flags
	{
		"MultiProcessorCompile"
	}

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

IncludeDirectories = {}
IncludeDirectories["assimp"] =      "OpenGLSandbox/third_party/assimp/include"
IncludeDirectories["entt"] =        "OpenGLSandbox/third_party/entt/include"
IncludeDirectories["ImGui"] =       "OpenGLSandbox/third_party/ImGui"
IncludeDirectories["glad"] =        "OpenGLSandbox/third_party/glad/include"
IncludeDirectories["GLFW"] =        "OpenGLSandbox/third_party/GLFW/include"
IncludeDirectories["glm"] =         "OpenGLSandbox/third_party/glm"
IncludeDirectories["stb_image"] =   "OpenGLSandbox/third_party/stb_image"
IncludeDirectories["yaml_cpp"] =    "OpenGLSandbox/third_party/yaml-cpp"

Binaries = {}
Binaries["assimp_Bin"] = "OpenGLSandbox/third_party/assimp/lib/assimp-vc142-mtd.dll"

Libraries = {}
Libraries["assimp_Lib"] = "OpenGLSandbox/third_party/assimp/lib/assimp-vc142-mtd.lib"

group "Dependencies"
	include "OpenGLSandbox/third_party/GLFW"
	include "OpenGLSandbox/third_party/glad"
	include "OpenGLSandbox/third_party/ImGui"
	include "OpenGLSandbox/third_party/yaml-cpp"
group ""

project "OpenGLSandbox"
	kind "StaticLib"
	language "C++"
	cppdialect "C++17"
	location "OpenGLSandbox"
	staticruntime "off"

	targetdir ("bin/" .. outputdir .. "/%{prj.name}")
	objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

    pchheader "epch.h"
	pchsource "OpenGLSandbox/src/epch.cpp"

	defines
	{
		"_CRT_SECURE_NO_WARNINGS",
		"GLFW_INCLUDE_NONE"
	}

	files
	{
		"%{prj.name}/src/**.h",
		"%{prj.name}/src/**.c",
		"%{prj.name}/src/**.cpp",
		"%{prj.name}/src/**.hpp",
		"%{prj.name}/third_party/stb_image/stb_image.h",
		"%{prj.name}/third_party/stb_image/stb_image.cpp",
		"%{prj.name}/third_party/glm/glm/**.hpp",
		"%{prj.name}/third_party/glm/glm/**.inl",
		"%{prj.name}/third_party/entt/include/entt.hpp",
	}

	includedirs 
	{
		"%{prj.name}/src",
        "%{prj.name}/third_party/spdlog/include",
        "%{IncludeDirectories.assimp}",
		"%{IncludeDirectories.GLFW}",
		"%{IncludeDirectories.glad}",
		"%{IncludeDirectories.glm}",
		"%{IncludeDirectories.entt}",
		"%{IncludeDirectories.ImGui}",
		"%{IncludeDirectories.stb_image}",
		"%{IncludeDirectories.yaml_cpp}",
	}

	links
	{
		"GLFW",
		"glad",
		"ImGui",
		"yaml-cpp",
        "opengl32.lib"
	}

    filter "files:OpenGLSandbox/third_party/yaml-cpp/src/**.cpp"
	flags { "NoPCH" }

	pchheader "epch.h"
	pchsource "%{prj.name}/src/epch.cpp"

	filter "system:windows"
		systemversion "latest"

	filter "configurations:Debug"
		defines "GLSB_DEBUG"
		runtime "Debug"
		symbols "on"

	filter "configurations:Release"
		defines "GLSB_RELEASE"
		runtime "Release"
		optimize "on"

	filter "configurations:Dist"
		defines "GLSB_DIST"
		runtime "Release"
		optimize "on"


project "Sandbox"
	kind "ConsoleApp"
	language "C++"
	cppdialect "C++17"
	location "Sandbox"
	staticruntime "off"

	targetdir ("bin/" .. outputdir .. "/%{prj.name}")
	objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

	files
	{
		"%{prj.name}/src/**.h",
		"%{prj.name}/src/**.c",
		"%{prj.name}/src/**.cpp",
		"%{prj.name}/src/**.hpp",
	}

	includedirs 
	{
		"%{prj.name}/src",
		"%{wks.location}/OpenGLSandbox/src",
		"%{wks.location}/OpenGLSandbox/third_party",
        "%{wks.location}/OpenGLSandbox/third_party/spdlog/include",

		"%{IncludeDirectories.glm}",
		"%{IncludeDirectories.entt}",
		"%{IncludeDirectories.yaml_cpp}/include",
	}

	links 
	{
		"OpenGLSandbox",
        "%{Libraries}.assimp_Lib"
	}

    postbuildcommands 
    {
        '{COPY} "%{Binaries.assimp_Bin}" "%{cfg.targetdir}"',
    }

	filter "system:windows"
		systemversion "latest"

	filter "configurations:Debug"
		defines "SB_DEBUG"
		runtime "Debug"
		symbols "on"

	filter "configurations:Release"
		defines "SB_RELEASE"
		runtime "Release"
		optimize "on"

	filter "configurations:Dist"
		defines "SB_DIST"
		runtime "Release"
		optimize "on"