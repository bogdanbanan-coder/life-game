#include <presentation/application/raylib-application.hpp>

#include <chrono>
#include <utility>

#include <application/field-command-executor.hpp>
#include <raylib.h>

namespace lifeGame::presentation {

    namespace {

        [[nodiscard]] auto sampleTextInput() -> TextInput {
            TextInput input;
            input.enter = IsKeyPressed(KEY_ENTER);
            input.escape = IsKeyPressed(KEY_ESCAPE);
            return input;
        }

        [[nodiscard]] auto sampleFrameInput() -> FrameInput {
            const auto mousePosition = GetMousePosition();
            // Raylib exposes both values in logical client pixels; render dimensions are physical
            // pixels and must not be used for UI or field hit testing.
            return FrameInput{GetScreenWidth(),
                              GetScreenHeight(),
                              PointerSample{
                                  LogicalPoint{mousePosition.x, mousePosition.y},
                                  IsMouseButtonPressed(MOUSE_BUTTON_LEFT),
                                  IsMouseButtonDown(MOUSE_BUTTON_LEFT),
                                  IsMouseButtonReleased(MOUSE_BUTTON_LEFT),
                              },
                              sampleTextInput()};
        }

    } // namespace

    RaylibApplication::RaylibApplication(domain::Field field)
        : legacyField_{std::move(field)} {}

    RaylibApplication::RaylibApplication(application::SettingsService& settingsService,
                                         application::SessionService& sessionService)
        : settingsService_{&settingsService},
          sessionService_{&sessionService},
          startScreen_{std::in_place, settingsService, sessionService} {}

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
            if (const auto* field = activeField(); field != nullptr) {
                fieldScreen_.render(*field, frameInput.viewportWidth, frameInput.viewportHeight,
                                    paintMode_, runState_);
            } else if (startScreen_) {
                startScreen_->render(frameInput.viewportWidth, frameInput.viewportHeight);
            }
            EndDrawing();
        }

        CloseWindow();
        return 0;
    }

    FrameInput RaylibApplication::processIteration(
        application::SimulationScheduler::Duration elapsed,
        std::optional<FrameInput> input) {
        if (auto* field = mutableActiveField(); field != nullptr &&
            runState_ == application::RunState::Running) {
            static_cast<void>(simulationScheduler_.advance(*field, elapsed));
        }

        if (!input) {
            input = sampleFrameInput();
        }
        if (hasActiveSession() || legacyField_) {
            processInput(*input);
        } else {
            processStartScreenInput(*input);
        }
        return *input;
    }

    const domain::Field& RaylibApplication::field() const noexcept {
        if (const auto* active = activeField(); active != nullptr) {
            return *active;
        }
        return emptyField();
    }

    const domain::Field* RaylibApplication::activeField() const noexcept {
        if (legacyField_) {
            return &*legacyField_;
        }
        if (sessionService_ && activeSessionId_) {
            const auto* session = sessionService_->find(*activeSessionId_);
            return session == nullptr ? nullptr : &session->field();
        }
        return nullptr;
    }

    bool RaylibApplication::hasActiveSession() const noexcept {
        return sessionService_ != nullptr && activeSessionId_.has_value();
    }

    std::optional<domain::SessionId> RaylibApplication::activeSessionId() const noexcept {
        return activeSessionId_;
    }

    const StartScreen* RaylibApplication::startScreen() const noexcept {
        return startScreen_ ? &*startScreen_ : nullptr;
    }

    StartScreen* RaylibApplication::startScreen() noexcept {
        return startScreen_ ? &*startScreen_ : nullptr;
    }

    application::PaintMode RaylibApplication::paintMode() const noexcept { return paintMode_; }

    application::RunState RaylibApplication::runState() const noexcept { return runState_; }

    CameraState RaylibApplication::cameraState() const noexcept {
        return fieldScreen_.cameraState();
    }

    ZoomLevel RaylibApplication::zoomLevel() const noexcept {
        return fieldScreen_.cameraState().zoomLevel;
    }

    void RaylibApplication::processInput(const FrameInput& input) {
        auto* field = mutableActiveField();
        if (field == nullptr) {
            return;
        }

        const auto commands = inputRouter_.sample(*field, input.viewportWidth,
                                                   input.viewportHeight, input.pointer, paintMode_,
                                                   runState_, fieldScreen_.cameraState());
        if (commands.selectedPaintMode) {
            paintMode_ = *commands.selectedPaintMode;
        }
        for (const auto& command : commands.paintCommands) {
            application::FieldCommandExecutor::execute(*field, command);
        }
        for (const auto& command : commands.panCommands) {
            fieldScreen_.applyCameraPan(*field, input.viewportWidth, input.viewportHeight,
                                        command.deltaX, command.deltaY);
        }
        if (commands.zoomRequest) {
            const auto& command = *commands.zoomRequest;
            fieldScreen_.applyZoom(*field, input.viewportWidth, input.viewportHeight,
                                   command.direction, command.anchorX, command.anchorY);
        }
        if (commands.pauseRequest) {
            pause();
        }
        if (commands.resumeRequest) {
            resume();
        }
        if (commands.exitRequest) {
            closeSession();
        }
    }

    void RaylibApplication::processStartScreenInput(const FrameInput& input) {
        if (!startScreen_) {
            return;
        }

        const auto action = startScreen_->processInput(input.viewportWidth, input.viewportHeight,
                                                       input.pointer, input.textInput);
        if (action.kind == StartScreenActionKind::OpenSession && action.sessionId) {
            openSession(*action.sessionId);
        }
    }

    void RaylibApplication::openSession(domain::SessionId sessionId) noexcept {
        if (sessionService_ == nullptr || settingsService_ == nullptr ||
            sessionService_->find(sessionId) == nullptr) {
            return;
        }

        activeSessionId_ = sessionId;
        const auto interval = simulationScheduler_.setInterval(
            settingsService_->activeSettings().generationInterval());
        if (!interval) {
            activeSessionId_.reset();
            return;
        }
        simulationScheduler_.clearAccumulator();
        inputRouter_.reset();
        fieldScreen_.resetNavigation();
        paintMode_ = application::PaintMode::Live;
        runState_ = application::RunState::Running;
    }

    void RaylibApplication::closeSession() noexcept {
        if (!hasActiveSession()) {
            return;
        }

        activeSessionId_.reset();
        simulationScheduler_.clearAccumulator();
        inputRouter_.reset();
        fieldScreen_.resetNavigation();
        paintMode_ = application::PaintMode::Live;
        runState_ = application::RunState::Running;
    }

    void RaylibApplication::pause() noexcept {
        if (runState_ != application::RunState::Running) {
            return;
        }

        simulationScheduler_.clearAccumulator();
        runState_ = application::RunState::Paused;
    }

    void RaylibApplication::resume() noexcept {
        if (runState_ != application::RunState::Paused) {
            return;
        }

        simulationScheduler_.clearAccumulator();
        if (paintMode_ != application::PaintMode::Move) {
            paintMode_ = application::PaintMode::Live;
        }
        runState_ = application::RunState::Running;
    }

    domain::Field* RaylibApplication::mutableActiveField() noexcept {
        if (legacyField_) {
            return &*legacyField_;
        }
        if (sessionService_ && activeSessionId_) {
            auto* session = sessionService_->find(*activeSessionId_);
            return session == nullptr ? nullptr : &session->field();
        }
        return nullptr;
    }

    const domain::Field& RaylibApplication::emptyField() noexcept {
        static const auto empty = domain::Field::create(1, 1);
        return empty.value();
    }

} // namespace lifeGame::presentation
