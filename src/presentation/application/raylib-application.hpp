#pragma once

#include <optional>

#include <application/simulation/simulation-scheduler.hpp>
#include <domain/field/field.hpp>
#include <presentation/input/input-router.hpp>
#include <presentation/screens/field-screen.hpp>

namespace lifeGame::presentation {

    struct FrameInput {
        int viewportWidth;
        int viewportHeight;
        PointerSample pointer;
    };

    class RaylibApplication {
      public:
        explicit RaylibApplication(domain::Field field);

        [[nodiscard]] int run();
        /// Processes the simulation and input phases before the caller renders the frame.
        [[nodiscard]] FrameInput processIteration(
            application::SimulationScheduler::Duration elapsed,
            std::optional<FrameInput> input = std::nullopt);
        [[nodiscard]] const domain::Field& field() const noexcept;

      private:
        void processInput(const FrameInput& input);

        domain::Field field_;
        FieldScreen fieldScreen_;
        InputRouter inputRouter_;
        application::SimulationScheduler simulationScheduler_;
    };

} // namespace lifeGame::presentation
