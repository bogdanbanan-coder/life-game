include(Catch)

add_executable(life-game-foundation-tests unit/foundation/result-test.cpp)

target_compile_features(life-game-foundation-tests PRIVATE cxx_std_23)
target_link_libraries(life-game-foundation-tests PRIVATE life-game-foundation Catch2::Catch2WithMain)
life_game_enable_warnings(life-game-foundation-tests)
life_game_enable_sanitizers(life-game-foundation-tests)

catch_discover_tests(life-game-foundation-tests)

add_executable(life-game-field-dimensions-tests unit/domain/field-dimensions-test.cpp)

target_compile_features(life-game-field-dimensions-tests PRIVATE cxx_std_23)
target_link_libraries(
    life-game-field-dimensions-tests
    PRIVATE
        life-game-domain
        Catch2::Catch2WithMain
)
life_game_enable_warnings(life-game-field-dimensions-tests)
life_game_enable_sanitizers(life-game-field-dimensions-tests)

catch_discover_tests(life-game-field-dimensions-tests)

add_executable(life-game-field-tests unit/domain/field-test.cpp)

target_compile_features(life-game-field-tests PRIVATE cxx_std_23)
target_link_libraries(life-game-field-tests PRIVATE life-game-domain Catch2::Catch2WithMain)
life_game_enable_warnings(life-game-field-tests)
life_game_enable_sanitizers(life-game-field-tests)

catch_discover_tests(life-game-field-tests)

add_executable(
    life-game-field-presentation-tests
    unit/presentation/field-renderer-test.cpp
    unit/presentation/coordinate-converter-test.cpp
    unit/presentation/drag-rasterizer-test.cpp
    unit/presentation/input-router-test.cpp
    unit/presentation/toolbar-test.cpp
)

target_compile_features(life-game-field-presentation-tests PRIVATE cxx_std_23)
target_link_libraries(
    life-game-field-presentation-tests
    PRIVATE
        life-game-presentation
        Catch2::Catch2WithMain
)
life_game_enable_warnings(life-game-field-presentation-tests)
life_game_enable_sanitizers(life-game-field-presentation-tests)

catch_discover_tests(life-game-field-presentation-tests)

add_test(NAME life-game-smoke COMMAND life-game --smoke-test)
