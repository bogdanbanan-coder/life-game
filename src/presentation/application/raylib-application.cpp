#include <presentation/application/raylib-application.hpp>

#include <chrono>
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
        auto previousIterationStart = std::chrono::steady_clock::now();
        while (!WindowShouldClose()) {
            const auto iterationStart = std::chrono::steady_clock::now();
            const auto elapsed = iterationStart - previousIterationStart;
            previousIterationStart = iterationStart;
            const auto frameInput = processIteration(elapsed);

            BeginDrawing();
            fieldScreen_.render(field_, frameInput.viewportWidth, frameInput.viewportHeight,
                                paintMode_);
            EndDrawing();
        }

        CloseWindow();
        return 0;
    }

    FrameInput RaylibApplication::processIteration(
        application::SimulationScheduler::Duration elapsed,
        std::optional<FrameInput> input) {
        static_cast<void>(simulationScheduler_.advance(field_, elapsed));

        if (!input) {
            const auto mousePosition = GetMousePosition();
            input = FrameInput{
                GetScreenWidth(),
                GetScreenHeight(),
                PointerSample{
                    LogicalPoint{mousePosition.x, mousePosition.y},
                    IsMouseButtonPressed(MOUSE_BUTTON_LEFT),
                    IsMouseButtonDown(MOUSE_BUTTON_LEFT),
                    IsMouseButtonReleased(MOUSE_BUTTON_LEFT),
                },
            };
        }
        processInput(*input);
        return *input;
    }

    const domain::Field& RaylibApplication::field() const noexcept { return field_; }

    application::PaintMode RaylibApplication::paintMode() const noexcept { return paintMode_; }

    void RaylibApplication::processInput(const FrameInput& input) {
        const auto commands = inputRouter_.sample(field_, input.viewportWidth,
                                                   input.viewportHeight, input.pointer, paintMode_);
        if (commands.selectedPaintMode) {
            paintMode_ = *commands.selectedPaintMode;
        }
        for (const auto& command : commands.paintCommands) {
            application::FieldCommandExecutor::execute(field_, command);
        }
    }

} // namespace lifeGame::presentation
