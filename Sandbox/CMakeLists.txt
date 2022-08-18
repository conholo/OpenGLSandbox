cmake_minimum_required(VERSION 3.2.2)
project(Sandbox CXX C)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# Set directory paths
set(SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(ASSET_DIR ${CMAKE_CURRENT_SOURCE_DIR}/assets)
set(SHADER_DIR ${ASSET_DIR}/shaders)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

file(GLOB_RECURSE sources ${SOURCE_DIR}/*.cpp)
file(GLOB_RECURSE includes ${SOURCE_DIR}/*.h)

add_executable(Sandbox ${sources} ${includes})
target_include_directories(Sandbox PRIVATE ${SOURCE_DIR})
target_include_directories(Sandbox PRIVATE ${CMAKE_SOURCE_DIR}/OpenGLSandbox/src/Engine.h)
target_link_libraries(Sandbox ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/OpenGLSandbox.lib)

# Enable C++17
set_target_properties(Sandbox PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS NO)


add_custom_target(CopyAssets COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_LIST_DIR}/assets ${CMAKE_BINARY_DIR}/assets)
add_dependencies(Sandbox CopyAssets)

# Set project folders
set_target_properties(Sandbox PROPERTIES FOLDER ${PROJECT_NAME})

install(TARGETS Sandbox RUNTIME DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
