#pragma once

#include <optional>
#include <utility>

#include <application/configuration/settings-service.hpp>
#include <application/commands/field-command.hpp>
#include <application/commands/run-command.hpp>
#include <application/session/session-service.hpp>
#include <application/simulation/simulation-scheduler.hpp>
#include <domain/field/field.hpp>
#include <presentation/input/input-router.hpp>
#include <presentation/screens/field-screen.hpp>
#include <presentation/screens/start-screen.hpp>

namespace lifeGame::presentation {

    struct FrameInput {
        FrameInput(int viewportWidthValue, int viewportHeightValue, PointerSample pointerValue,
                   TextInput textInputValue = {})
            : viewportWidth{viewportWidthValue},
              viewportHeight{viewportHeightValue},
              pointer{pointerValue},
              textInput{std::move(textInputValue)} {}

        int viewportWidth;
        int viewportHeight;
        PointerSample pointer;
        TextInput textInput;
    };

    class RaylibApplication {
      public:
        explicit RaylibApplication(domain::Field field);
        RaylibApplication(application::SettingsService& settingsService,
                          application::SessionService& sessionService);

        [[nodiscard]] int run();
        /// Processes the simulation and input phases before the caller renders the frame.
        [[nodiscard]] FrameInput processIteration(
            application::SimulationScheduler::Duration elapsed,
            std::optional<FrameInput> input = std::nullopt);
        [[nodiscard]] const domain::Field& field() const noexcept;
        [[nodiscard]] const domain::Field* activeField() const noexcept;
        [[nodiscard]] bool hasActiveSession() const noexcept;
        [[nodiscard]] std::optional<domain::SessionId> activeSessionId() const noexcept;
        [[nodiscard]] const StartScreen* startScreen() const noexcept;
        [[nodiscard]] StartScreen* startScreen() noexcept;
        [[nodiscard]] application::PaintMode paintMode() const noexcept;
        [[nodiscard]] application::RunState runState() const noexcept;

      private:
        void processInput(const FrameInput& input);
        void processStartScreenInput(const FrameInput& input);
        void openSession(domain::SessionId sessionId) noexcept;
        void closeSession() noexcept;
        void pause() noexcept;
        void resume() noexcept;

        [[nodiscard]] domain::Field* mutableActiveField() noexcept;
        [[nodiscard]] static const domain::Field& emptyField() noexcept;

        std::optional<domain::Field> legacyField_;
        application::SettingsService* settingsService_ = nullptr;
        application::SessionService* sessionService_ = nullptr;
        std::optional<domain::SessionId> activeSessionId_;
        std::optional<StartScreen> startScreen_;
        FieldScreen fieldScreen_;
        InputRouter inputRouter_;
        application::SimulationScheduler simulationScheduler_;
        application::PaintMode paintMode_ = application::PaintMode::Live;
        application::RunState runState_ = application::RunState::Running;
    };

} // namespace lifeGame::presentation
