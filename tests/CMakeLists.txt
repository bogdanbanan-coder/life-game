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
    life-game-session-domain-tests
    unit/domain/session-name-test.cpp
    unit/domain/session-test.cpp
)

target_compile_features(life-game-session-domain-tests PRIVATE cxx_std_23)
target_link_libraries(
    life-game-session-domain-tests
    PRIVATE
        life-game-domain
        Catch2::Catch2WithMain
)
life_game_enable_warnings(life-game-session-domain-tests)
life_game_enable_sanitizers(life-game-session-domain-tests)

catch_discover_tests(life-game-session-domain-tests)

add_executable(life-game-conway-simulation-tests unit/domain/conway-simulation-test.cpp)

target_compile_features(life-game-conway-simulation-tests PRIVATE cxx_std_23)
target_link_libraries(
    life-game-conway-simulation-tests
    PRIVATE
        life-game-domain
        Catch2::Catch2WithMain
)
life_game_enable_warnings(life-game-conway-simulation-tests)
life_game_enable_sanitizers(life-game-conway-simulation-tests)

catch_discover_tests(life-game-conway-simulation-tests)

add_executable(
    life-game-simulation-scheduler-tests
    unit/application/simulation-scheduler-test.cpp
)

target_compile_features(life-game-simulation-scheduler-tests PRIVATE cxx_std_23)
target_link_libraries(
    life-game-simulation-scheduler-tests
    PRIVATE
        life-game-application
        Catch2::Catch2WithMain
)
life_game_enable_warnings(life-game-simulation-scheduler-tests)
life_game_enable_sanitizers(life-game-simulation-scheduler-tests)

catch_discover_tests(life-game-simulation-scheduler-tests)

add_executable(
    life-game-configuration-tests
    unit/application/settings-service-test.cpp
    unit/application/session-service-test.cpp
)

target_compile_features(life-game-configuration-tests PRIVATE cxx_std_23)
target_link_libraries(
    life-game-configuration-tests
    PRIVATE
        life-game-application
        Catch2::Catch2WithMain
)
life_game_enable_warnings(life-game-configuration-tests)
life_game_enable_sanitizers(life-game-configuration-tests)

catch_discover_tests(life-game-configuration-tests)

add_executable(
    life-game-field-presentation-tests
    unit/presentation/raylib-application-test.cpp
    unit/presentation/coordinate-converter-test.cpp
    unit/presentation/drag-rasterizer-test.cpp
    unit/presentation/input-router-test.cpp
    unit/presentation/raylib-application-story-test.cpp
    unit/presentation/settings-panel-test.cpp
    unit/presentation/start-screen-test.cpp
    unit/presentation/text-field-test.cpp
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

add_executable(
    life-game-field-renderer-tests
    unit/presentation/field-renderer-test.cpp
    ../src/presentation/rendering/field-renderer.cpp
)

target_compile_features(life-game-field-renderer-tests PRIVATE cxx_std_23)
target_include_directories(life-game-field-renderer-tests PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../src")
target_link_libraries(
    life-game-field-renderer-tests
    PRIVATE
        life-game-domain
        raylib
        Catch2::Catch2WithMain
)
life_game_enable_warnings(life-game-field-renderer-tests)
life_game_enable_sanitizers(life-game-field-renderer-tests)

catch_discover_tests(life-game-field-renderer-tests)

add_test(
    NAME life-game-raylib-frame-lifecycle-config
    COMMAND
        "${CMAKE_COMMAND}"
        "-DPROJECT_BINARY_DIR=${CMAKE_BINARY_DIR}"
        -P "${CMAKE_CURRENT_SOURCE_DIR}/cmake/raylib-frame-lifecycle-config-test.cmake"
)

add_test(NAME life-game-smoke COMMAND life-game --smoke-test)
