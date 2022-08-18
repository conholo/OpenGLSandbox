cmake_minimum_required(VERSION 3.2.2)
project(OpenGLSandbox CXX C)

#===========================================================================================
# GLAD CONFIGURATION
#
# Configure GLAD loader generation variables (lines below pulled from the GLAD top-level
# CMakeLists.txt file, with slight modifications.
# Refer to https://github.com/Dav1dde/glad for more information)

set(GLAD_OUT_DIR        "${CMAKE_CURRENT_BINARY_DIR}" CACHE STRING "Output directory" FORCE)
set(GLAD_PROFILE        "core" CACHE STRING "OpenGL profile" FORCE)
set(GLAD_API            "gl=4.5" CACHE STRING "API type/version pairs, like \"gl=3.2,gles=\", no version means latest" FORCE)
set(GLAD_GENERATOR      "c" CACHE STRING "Language to generate the binding for" FORCE)
set(GLAD_EXTENSIONS     "" CACHE STRING "Path to extensions file or comma separated list of extensions, if missing all extensions are included" FORCE)
set(GLAD_SPEC           "gl" CACHE STRING "Name of the spec" FORCE)
set(GLAD_ALL_EXTENSIONS OFF CACHE BOOL "Include all extensions instead of those specified by GLAD_EXTENSIONS" FORCE)
set(GLAD_NO_LOADER      OFF CACHE BOOL "No loader" FORCE)
set(GLAD_REPRODUCIBLE   OFF CACHE BOOL "Reproducible build" FORCE)

#===========================================================================================

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# Set directory paths

set(SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(THIRD_PARTY_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/third_party)
set(CMAKE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

file(GLOB_RECURSE sources ${SOURCE_DIR}/*.cpp ${SOURCE_DIR}/*.c)
file(GLOB_RECURSE includes ${SOURCE_DIR}/*.h)
file(GLOB_RECURSE third_party_sources ${THIRD_PARTY_SOURCE_DIR}/*.cpp ${THIRD_PARTY_SOURCE_DIR}/*.c)

add_library(OpenGLSandbox STATIC ${sources} ${includes} ${third_party_sources})
target_include_directories(OpenGLSandbox PRIVATE ${SOURCE_DIR})

include(${CMAKE_DIR}/PCH.cmake)
add_precompiled_header(OpenGLSandbox 
    ${SOURCE_DIR}/epch.h 
    SOURCE_CXX ${SOURCE_DIR}/epch.cpp 
    SOURCE_C ${SOURCE_DIR}/epch.c 
    FORCEINCLUDE)

# Perform dependency linkage

include(${CMAKE_DIR}/LinkENTT.cmake)
LinkENTT(OpenGLSandbox PRIVATE)

include(${CMAKE_DIR}/LinkGLAD.cmake)
LinkGLAD(OpenGLSandbox PRIVATE)

include(${CMAKE_DIR}/LinkGLFW.cmake)
LinkGLFW(OpenGLSandbox PRIVATE)

include(${CMAKE_DIR}/LinkGLM.cmake)
LinkGLM(OpenGLSandbox PRIVATE)

include(${CMAKE_DIR}/LinkIMGUI.cmake)
LinkIMGUI(OpenGLSandbox PRIVATE)

include(${CMAKE_DIR}/LinkSPDLOG.cmake)
LinkSPDLOG(OpenGLSandbox PRIVATE)

include(${CMAKE_DIR}/LinkSTB.cmake)
LinkSTB(OpenGLSandbox PRIVATE)

set(ASSIMP_DIR ${THIRD_PARTY_SOURCE_DIR}/assimp)
target_include_directories(OpenGLSandbox PRIVATE "${THIRD_PARTY_SOURCE_DIR}/assimp/include/")
target_link_libraries(OpenGLSandbox PRIVATE ${ASSIMP_DIR}/lib/assimp-vc142-mt.lib)

find_package(OpenGL REQUIRED)
if (OpenGL_FOUND)
    target_include_directories(OpenGLSandbox PRIVATE ${OPENGL_INCLUDE_DIRS})
    target_link_libraries(OpenGLSandbox PRIVATE ${OPENGL_LIBRARIES})
else()
    message(FATAL_ERROR "OpenGL was not found on the system")
endif()

get_property(dirs DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY INCLUDE_DIRECTORIES)
foreach(dir ${dirs})
  message(STATUS "dir='${dir}'")
endforeach()

# Enable C++17
set_target_properties(OpenGLSandbox PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS NO)

set_target_properties(OpenGLSandbox
    PROPERTIES
    FOLDER ${PROJECT_NAME}
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
)

install(TARGETS OpenGLSandbox
    LIBRARY DESTINATION ${LIBRARY_OUTPUT_DIRECTORY}
    ARCHIVE DESTINATION ${ARCHIVE_OUTPUT_DIRECTORY})
