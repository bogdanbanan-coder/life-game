#include <presentation/application/raylib-application.hpp>

#include <utility>

#include <application/field-command-executor.hpp>
#include <raylib.h>

namespace lifeGame::presentation {

    RaylibApplication::RaylibApplication(domain::Field field) : field_{std::move(field)} {}

    int RaylibApplication::run() {
        SetConfigFlags(FLAG_WINDOW_HIGHDPI);
        InitWindow(1280, 720, "Life Game");
        if (!IsWindowReady()) {
            CloseWindow();
            return 1;
        }

        SetTargetFPS(60);
        while (!WindowShouldClose()) {
            const auto viewportWidth = GetScreenWidth();
            const auto viewportHeight = GetScreenHeight();
            const auto mousePosition = GetMousePosition();
            const auto pointer = PointerSample{
                LogicalPoint{mousePosition.x, mousePosition.y},
                IsMouseButtonPressed(MOUSE_BUTTON_LEFT),
                IsMouseButtonDown(MOUSE_BUTTON_LEFT),
                IsMouseButtonReleased(MOUSE_BUTTON_LEFT),
            };
            const auto commands = inputRouter_.sample(field_, viewportWidth, viewportHeight,
                                                       pointer);
            for (const auto& command : commands) {
                application::FieldCommandExecutor::execute(field_, command);
            }

            BeginDrawing();
            fieldScreen_.render(field_, viewportWidth, viewportHeight);
            EndDrawing();
        }

        CloseWindow();
        return 0;
    }

} // namespace lifeGame::presentation
