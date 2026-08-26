add_library(
    life-game-application
    STATIC
        field-command-executor.cpp
        simulation/simulation-scheduler.cpp
)

target_compile_features(life-game-application PUBLIC cxx_std_23)
target_include_directories(life-game-application PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/..")
target_link_libraries(life-game-application PUBLIC life-game-domain)
life_game_enable_warnings(life-game-application)
life_game_enable_sanitizers(life-game-application)
