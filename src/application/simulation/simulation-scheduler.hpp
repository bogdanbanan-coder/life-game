#pragma once

#include <chrono>
#include <cstddef>

#include <domain/field/field.hpp>

namespace lifeGame::application {

    class SimulationScheduler {
      public:
        using Duration = std::chrono::steady_clock::duration;

        static constexpr auto GENERATION_INTERVAL = std::chrono::milliseconds{250};
        static constexpr std::size_t MAX_CATCH_UP_GENERATIONS = 4;

        /// Accumulates one iteration's elapsed steady-clock duration and advances due steps.
        /// At most four 250 ms generations run; whole excess intervals are discarded and
        /// the sub-interval remainder is retained. Non-positive durations are ignored.
        [[nodiscard]] std::size_t advance(domain::Field& field, Duration elapsed) noexcept;

        /// Discards all accumulated timing debt without changing the Field.
        void clearAccumulator() noexcept;

      private:
        Duration accumulator_{};
    };

} // namespace lifeGame::application
