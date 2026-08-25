cmake_minimum_required(VERSION 3.28)

project(life-game
    VERSION 0.1.0
    LANGUAGES C CXX
)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(cmake/project-options.cmake)
include(cmake/warnings.cmake)
include(cmake/sanitizers.cmake)
include(cmake/dependencies.cmake)

add_subdirectory(src)

if(LIFE_GAME_ENABLE_TESTING)
    enable_testing()
    add_subdirectory(tests)
endif()
