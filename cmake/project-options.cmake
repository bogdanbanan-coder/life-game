include_guard(GLOBAL)

option(LIFE_GAME_ENABLE_TESTING "Build and register project tests" ON)
option(LIFE_GAME_ENABLE_DEBUG_TOOLS "Build read-only debug tools" OFF)
option(LIFE_GAME_WARNINGS_AS_ERRORS "Treat project compiler warnings as errors" OFF)
option(LIFE_GAME_ENABLE_ASAN "Enable AddressSanitizer for project targets" OFF)
option(LIFE_GAME_ENABLE_UBSAN "Enable UndefinedBehaviorSanitizer for project targets" OFF)

if(LIFE_GAME_ENABLE_ASAN OR LIFE_GAME_ENABLE_UBSAN)
    if(NOT CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
        message(FATAL_ERROR "Project sanitizers require a Clang or GNU compiler")
    endif()
endif()
