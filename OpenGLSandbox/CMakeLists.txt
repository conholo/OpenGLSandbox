cmake_minimum_required(VERSION 3.2.2)
project(OpenGLSandbox CXX C)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

set(SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(THIRD_PARTY_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/third_party)
set(CMAKE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

file(GLOB_RECURSE sources ${SOURCE_DIR}/*.cpp ${SOURCE_DIR}/*.c)
add_library(OpenGLSandbox STATIC ${sources})

set(OpenGL_GL_PREFERENCE GLVND)
find_package(OpenGL REQUIRED)
target_include_directories(OpenGLSandbox PUBLIC src/)
target_include_directories(OpenGLSandbox PUBLIC third_party/glad/include/)
target_include_directories(OpenGLSandbox PUBLIC third_party/GLFW/include/)
target_include_directories(OpenGLSandbox PUBLIC third_party/entt/include/)
target_include_directories(OpenGLSandbox PUBLIC third_party/stbi/include/)
target_include_directories(OpenGLSandbox PUBLIC third_party/yaml-cpp/include/)
target_include_directories(OpenGLSandbox PUBLIC third_party/spdlog/include/)
target_include_directories(OpenGLSandbox PUBLIC third_party/glm/include/)
target_include_directories(OpenGLSandbox PUBLIC third_party/ImGui/)
target_include_directories(OpenGLSandbox PUBLIC third_party/entt/include)
target_include_directories(OpenGLSandbox PUBLIC ${OPENGL_INCLUDE_DIRS})

target_link_libraries(OpenGLSandbox PUBLIC glfw glad ImGui stbi yaml-cpp ${OPENGL_LIBRARIES})

# Enable C++17
set_target_properties(OpenGLSandbox PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS NO)
