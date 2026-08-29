add_library(
    life-game-presentation
    STATIC
        application/raylib-application.cpp
        camera/camera-controller.cpp
        camera/coordinate-converter.cpp
        input/drag-rasterizer.cpp
        input/input-router.cpp
        raylib/raygui-implementation.cpp
        rendering/field-renderer.cpp
        screens/field-screen.cpp
        screens/start-screen.cpp
        ui/error-dialog.cpp
        ui/name-dialog.cpp
        ui/numeric-field.cpp
        ui/settings-panel.cpp
        ui/status-message.cpp
        ui/text-field.cpp
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
