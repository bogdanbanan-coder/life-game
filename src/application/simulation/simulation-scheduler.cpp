#include <application/simulation/simulation-scheduler.hpp>

#include <algorithm>
#include <chrono>
#include <cmath>
#include <limits>
#include <utility>

#include <domain/simulation/conway-simulation.hpp>

namespace lifeGame::application {

    SimulationScheduler::SimulationScheduler() noexcept = default;

    auto SimulationScheduler::create(Interval interval)
        -> foundation::Result<SimulationScheduler, foundation::ErrorCode> {
        if (!std::isfinite(interval.count()) || interval <= Interval::zero()) {
            return foundation::Result<SimulationScheduler, foundation::ErrorCode>::failure(
                foundation::ErrorCode::InvalidArgument);
        }

        SimulationScheduler scheduler;
        scheduler.intervalSeconds_ = interval.count();
        return foundation::Result<SimulationScheduler, foundation::ErrorCode>::success(
            std::move(scheduler));
    }

    auto SimulationScheduler::setInterval(Interval interval)
        -> foundation::Result<void, foundation::ErrorCode> {
        if (!std::isfinite(interval.count()) || interval <= Interval::zero()) {
            return foundation::Result<void, foundation::ErrorCode>::failure(
                foundation::ErrorCode::InvalidArgument);
        }

        intervalSeconds_ = interval.count();
        return foundation::Result<void, foundation::ErrorCode>::success();
    }

    SimulationScheduler::Interval SimulationScheduler::interval() const noexcept {
        return Interval{intervalSeconds_};
    }

    SimulationScheduler::Interval SimulationScheduler::generationInterval() const noexcept {
        return interval();
    }

    SimulationScheduler::Interval SimulationScheduler::accumulated() const noexcept {
        return Interval{accumulatorSeconds_};
    }

    std::size_t SimulationScheduler::advance(domain::Field& field, Duration elapsed) noexcept {
        if (elapsed <= Duration::zero()) {
            return 0;
        }

        if (!std::isfinite(intervalSeconds_) || intervalSeconds_ <= 0.0L) {
            return 0;
        }

        const auto elapsedSeconds = std::chrono::duration_cast<Interval>(elapsed).count();
        if (!std::isfinite(elapsedSeconds) || elapsedSeconds <= 0.0L) {
            return 0;
        }

        accumulatorSeconds_ += elapsedSeconds;
        if (!std::isfinite(accumulatorSeconds_)) {
            accumulatorSeconds_ = 0.0L;
            for (std::size_t generation = 0; generation < MAX_CATCH_UP_GENERATIONS;
                 ++generation) {
                domain::ConwaySimulation::advance(field);
            }
            return MAX_CATCH_UP_GENERATIONS;
        }

        const auto ratio = accumulatorSeconds_ / intervalSeconds_;
        // The elapsed input has steady-clock resolution; this tolerance only absorbs
        // representation noise from accumulating and subtracting fractional seconds.
        const auto tolerance =
            std::numeric_limits<long double>::epsilon() * std::max(1.0L, std::abs(ratio)) * 32.0L;
        const auto dueGenerations = std::floor(ratio + tolerance);
        const auto generations = dueGenerations >=
                                          static_cast<long double>(MAX_CATCH_UP_GENERATIONS)
                                      ? MAX_CATCH_UP_GENERATIONS
                                      : static_cast<std::size_t>(dueGenerations);

        for (std::size_t generation = 0; generation < generations; ++generation) {
            domain::ConwaySimulation::advance(field);
        }

        if (generations == MAX_CATCH_UP_GENERATIONS) {
            accumulatorSeconds_ = std::fmod(accumulatorSeconds_, intervalSeconds_);
        } else {
            accumulatorSeconds_ -=
                static_cast<long double>(generations) * intervalSeconds_;
            if (accumulatorSeconds_ < 0.0L) {
                accumulatorSeconds_ = 0.0L;
            }
        }

        return generations;
    }

    void SimulationScheduler::clearAccumulator() noexcept { accumulatorSeconds_ = 0.0L; }

} // namespace lifeGame::application
