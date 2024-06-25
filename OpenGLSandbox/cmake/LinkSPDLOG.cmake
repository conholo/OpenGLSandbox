include(FetchContent)

macro(LinkSPDLOG TARGET ACCESS)
    FetchContent_Declare(
        spdlog
        GIT_REPOSITORY https://github.com/gabime/spdlog
        GIT_TAG v1.10.0
        CONFIGURE_COMMAND ""
        BUILD_COMMAND ""
    )

    FetchContent_GetProperties(spdlog)

    if (NOT spdlog_POPULATED)
        FetchContent_Populate(spdlog)
    endif()

    target_include_directories(${TARGET} ${ACCESS} ${spdlog_SOURCE_DIR}/include)
endmacro()