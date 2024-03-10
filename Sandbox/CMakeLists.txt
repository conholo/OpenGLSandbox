cmake_minimum_required(VERSION 3.23)
project(Sandbox CXX C)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# Set directory paths
set(SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(ASSET_DIR ${CMAKE_CURRENT_SOURCE_DIR}/assets)
set(SHADER_DIR ${ASSET_DIR}/shaders)

file(GLOB_RECURSE sources ${SOURCE_DIR}/*.cpp)

add_executable(Sandbox ${sources})
target_include_directories(Sandbox PRIVATE ${SOURCE_DIR})
target_include_directories(Sandbox PRIVATE ${CMAKE_SOURCE_DIR}/OpenGLSandbox/src)
target_include_directories(Sandbox PRIVATE ${THIRD_PARTY_DIR}/spdlog/include)
target_include_directories(Sandbox PRIVATE ${THIRD_PARTY_DIR}/entt/include)
target_include_directories(Sandbox PRIVATE ${THIRD_PARTY_DIR}/imgui)

target_link_libraries(Sandbox OpenGLSandbox)

# Enable C++17
set_target_properties(Sandbox PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS NO)

# Function to create a symbolic link
function(create_symlink src dst)
    if (UNIX)
        # UNIX systems
        execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink ${src} ${dst})
    else()
        # Windows systems
        execute_process(COMMAND cmd.exe /c mklink /J ${dst} ${src})
    endif()
endfunction()

# Define source and destination directories
set(SourceDirectory "${CMAKE_CURRENT_SOURCE_DIR}/assets")
set(DestinationDirectory "${CMAKE_BINARY_DIR}/Sandbox/assets")

# Check if the destination directory exists
if (NOT EXISTS ${DestinationDirectory})
    # If it doesn't exist, create the symlink
    create_symlink(${SourceDirectory} ${DestinationDirectory})
elseif (IS_SYMLINK ${DestinationDirectory})
    # Handle case where the destination is an existing symlink
    # (Optional) Remove or update the symlink if necessary
endif()

# Add custom command with dependency on the created symlink
add_custom_command(TARGET Sandbox PRE_BUILD COMMAND ${CMAKE_COMMAND} -E echo "Symlink for assets directory created.")
