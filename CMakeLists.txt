cmake_minimum_required(VERSION 3.2.2)
project(OpenGLSandbox)

set(THIRD_PARTY_DIR ${CMAKE_SOURCE_DIR}/OpenGLSandbox/third_party)


add_subdirectory(${THIRD_PARTY_DIR}/glad)
add_subdirectory(${THIRD_PARTY_DIR}/GLFW)
add_subdirectory(${THIRD_PARTY_DIR}/imgui)
add_subdirectory(${THIRD_PARTY_DIR}/stbi)
add_subdirectory(${THIRD_PARTY_DIR}/yaml-cpp)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/OpenGLSandbox)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/Sandbox)
