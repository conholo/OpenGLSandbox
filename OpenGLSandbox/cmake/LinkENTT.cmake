include(FetchContent)

macro(LinkENTT TARGET ACCESS)
    FetchContent_Declare(
        entt
        GIT_REPOSITORY https://github.com/skypjack/entt
        GIT_TAG v3.10.3
    )

    FetchContent_GetProperties(entt)

    if (NOT entt_POPULATED)
        FetchContent_Populate(entt)
    endif()

    target_include_directories(${TARGET} ${ACCESS} ${entt_SOURCE_DIR})
endmacro()