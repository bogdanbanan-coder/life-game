include_guard(GLOBAL)

include(FetchContent)

set(
    FETCHCONTENT_TRY_FIND_PACKAGE_MODE
    NEVER
    CACHE STRING "Do not satisfy pinned dependencies from installed packages"
    FORCE
)

FetchContent_Declare(
    raylib
    URL https://github.com/raysan5/raylib/archive/refs/tags/6.0.tar.gz
    URL_HASH SHA256=2b3ee1e2120c7a0796b33062c7e9a694dd8a8caa56a96319ac8c8ecf54a90d0b
)

FetchContent_Declare(
    raygui
    URL https://github.com/raysan5/raygui/archive/refs/tags/5.0.tar.gz
    URL_HASH SHA256=0f194c4a5e837c0930aca0b6315db45d00f76fa0052d841eea94598d390c39d6
)

FetchContent_Declare(
    sqlite3
    URL https://www.sqlite.org/2026/sqlite-autoconf-3530400.tar.gz
    URL_HASH SHA256=0e9483900e92cd5de8fd48d16bf9200145a61f7fd5be542a5ac81d8a9516eb9c
)

if(LIFE_GAME_ENABLE_TESTING)
    FetchContent_Declare(
        catch2
        URL https://github.com/catchorg/Catch2/archive/refs/tags/v3.15.3.tar.gz
        URL_HASH SHA256=b0299ae552918220a7a6e21e7de5b714777f4e8c883fb70c4bb23fe01df8c6e3
    )
endif()

# Third-party projects keep their own warning and sanitizer policy.
set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(CUSTOMIZE_BUILD ON CACHE BOOL "" FORCE)
set(SUPPORT_MODULE_RAUDIO OFF CACHE BOOL "" FORCE)
# The application relies on EndDrawing for presentation, timing, and event polling.
set(
    SUPPORT_CUSTOM_FRAME_CONTROL
    OFF
    CACHE BOOL
    "Force standard raylib EndDrawing frame lifecycle (custom frame control disabled)"
    FORCE
)
set(RAYLIB_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(CATCH_DEVELOPMENT_BUILD OFF CACHE BOOL "" FORCE)
set(CATCH_BUILD_TESTING OFF CACHE BOOL "" FORCE)
set(CATCH_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(CATCH_BUILD_EXTRA_TESTS OFF CACHE BOOL "" FORCE)
set(CATCH_BUILD_BENCHMARKS OFF CACHE BOOL "" FORCE)

# Declare every archive above before materializing any dependency.
FetchContent_MakeAvailable(raylib)

FetchContent_MakeAvailable(raygui)

add_library(life-game-raygui INTERFACE)
target_include_directories(life-game-raygui SYSTEM INTERFACE "${raygui_SOURCE_DIR}/src")

FetchContent_MakeAvailable(sqlite3)

add_library(life-game-sqlite STATIC EXCLUDE_FROM_ALL "${sqlite3_SOURCE_DIR}/sqlite3.c")
target_include_directories(life-game-sqlite SYSTEM PUBLIC "${sqlite3_SOURCE_DIR}")
set_target_properties(
    life-game-sqlite
    PROPERTIES
        C_STANDARD 11
        C_STANDARD_REQUIRED ON
        C_EXTENSIONS OFF
)

if(LIFE_GAME_ENABLE_TESTING)
    FetchContent_MakeAvailable(catch2)
    list(APPEND CMAKE_MODULE_PATH "${catch2_SOURCE_DIR}/extras")
endif()
