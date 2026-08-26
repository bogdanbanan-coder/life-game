add_executable(life-game main.cpp)

target_compile_features(life-game PRIVATE cxx_std_23)
target_link_libraries(life-game PRIVATE life-game-presentation)
life_game_enable_warnings(life-game)
life_game_enable_sanitizers(life-game)
