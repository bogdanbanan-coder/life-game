add_library(
    life-game-presentation
    STATIC
        application/raylib-application.cpp
        raylib/raygui-implementation.cpp
        rendering/field-renderer.cpp
        screens/field-screen.cpp
        ui/toolbar.cpp
)

target_compile_features(life-game-presentation PUBLIC cxx_std_23)
target_include_directories(life-game-presentation PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/..")
target_link_libraries(
    life-game-presentation
    PUBLIC
        life-game-application
        raylib
    PRIVATE
        life-game-raygui
)
life_game_enable_warnings(life-game-presentation)
life_game_enable_sanitizers(life-game-presentation)
