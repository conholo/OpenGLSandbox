include(FetchContent)

macro(LinkIMGUI TARGET ACCESS)
    FetchContent_Declare(
        imgui
        GIT_REPOSITORY https://github.com/ocornut/imgui.git
        GIT_TAG docking
    )
    FetchContent_MakeAvailable(imgui)
    FetchContent_GetProperties(imgui)

    if (NOT imgui_POPULATED)
        FetchContent_Populate(imgui)
    endif()

    target_include_directories(${TARGET} ${ACCESS} ${imgui_SOURCE_DIR})

endmacro()