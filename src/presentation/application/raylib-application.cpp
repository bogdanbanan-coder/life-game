#include <presentation/application/raylib-application.hpp>

#include <utility>

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
            BeginDrawing();
            fieldScreen_.render(field_, GetScreenWidth(), GetScreenHeight());
            EndDrawing();
        }

        CloseWindow();
        return 0;
    }

} // namespace lifeGame::presentation
