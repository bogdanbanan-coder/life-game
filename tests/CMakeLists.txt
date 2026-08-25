include(Catch)

add_executable(life-game-foundation-tests unit/foundation/result-test.cpp)

target_compile_features(life-game-foundation-tests PRIVATE cxx_std_23)
target_link_libraries(life-game-foundation-tests PRIVATE life-game-foundation Catch2::Catch2WithMain)
life_game_enable_warnings(life-game-foundation-tests)
life_game_enable_sanitizers(life-game-foundation-tests)

catch_discover_tests(life-game-foundation-tests)
add_test(NAME life-game-smoke COMMAND life-game)
