#pragma once

#include <chrono>
#include <cstddef>

#include <domain/field/field.hpp>
#include <foundation/error-code.hpp>
#include <foundation/result.hpp>

namespace lifeGame::application {

    class SimulationScheduler {
      public:
        using Duration = std::chrono::steady_clock::duration;
        using Interval = std::chrono::duration<long double>;

        static constexpr auto GENERATION_INTERVAL = std::chrono::milliseconds{250};
        static constexpr std::size_t MAX_CATCH_UP_GENERATIONS = 4;

        SimulationScheduler() noexcept;

        [[nodiscard]] static auto create(Interval interval)
            -> foundation::Result<SimulationScheduler, foundation::ErrorCode>;

        [[nodiscard]] auto setInterval(Interval interval)
            -> foundation::Result<void, foundation::ErrorCode>;
        [[nodiscard]] Interval interval() const noexcept;
        [[nodiscard]] Interval generationInterval() const noexcept;
        [[nodiscard]] Interval accumulated() const noexcept;

        /// Accumulates one iteration's elapsed steady-clock duration and advances due steps.
        /// At most four configured generations run; whole excess intervals are discarded and
        /// the sub-interval remainder is retained. Non-positive durations are ignored.
        [[nodiscard]] std::size_t advance(domain::Field& field, Duration elapsed) noexcept;

        /// Discards all accumulated timing debt without changing the Field.
        void clearAccumulator() noexcept;

      private:
        static constexpr long double DEFAULT_INTERVAL_SECONDS = 0.25L;

        long double intervalSeconds_ = DEFAULT_INTERVAL_SECONDS;
        long double accumulatorSeconds_ = 0.0L;
    };

} // namespace lifeGame::application
