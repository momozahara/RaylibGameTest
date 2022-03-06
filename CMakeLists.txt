cmake_minimum_required(VERSION 3.8)

project(Game)

file(GLOB ASOURCE
    ./src/*.cpp
)

add_executable(Game ${ASOURCE})

target_include_directories(Game
    PRIVATE ./raylib-4.0.0_win64_msvc16/include/
)

target_link_directories(Game
    PRIVATE ./raylib-4.0.0_win64_msvc16/lib/
)

target_link_libraries(Game
    raylib
)

set_target_properties(Game PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_SOURCE_DIR}/output/
    COMPILE_FLAGS "/std:c++latest"
    COMPILE_DEFINITIONS "DEVELOPMENT;_CRT_SECURE_NO_WARNINGS"
)

set(CONSOLE TRUE)

if (NOT CONSOLE)
    set_target_properties(Game PROPERTIES
        LINK_FLAGS "/SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup"
    )
endif()