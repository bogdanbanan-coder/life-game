#include <application/simulation/simulation-scheduler.hpp>

#include <chrono>

#include <domain/simulation/conway-simulation.hpp>

namespace lifeGame::application {

    std::size_t SimulationScheduler::advance(domain::Field& field, Duration elapsed) noexcept {
        if (elapsed <= Duration::zero()) {
            return 0;
        }

        const auto interval =
            std::chrono::duration_cast<Duration>(GENERATION_INTERVAL);
        if (interval <= Duration::zero()) {
            return 0;
        }

        const auto elapsedWholeIntervals = elapsed / interval;
        const auto elapsedRemainder = elapsed % interval;
        const auto remainderToNextGeneration = interval - accumulator_;
        bool carriesGeneration = false;
        if (elapsedRemainder >= remainderToNextGeneration) {
            accumulator_ = elapsedRemainder - remainderToNextGeneration;
            carriesGeneration = true;
        } else {
            accumulator_ += elapsedRemainder;
        }

        const auto maxCatchUp = static_cast<decltype(elapsedWholeIntervals)>(
            MAX_CATCH_UP_GENERATIONS);
        auto generations = std::size_t{0};
        if (elapsedWholeIntervals >= maxCatchUp) {
            generations = MAX_CATCH_UP_GENERATIONS;
        } else {
            generations = static_cast<std::size_t>(elapsedWholeIntervals);
            if (carriesGeneration && generations < MAX_CATCH_UP_GENERATIONS) {
                ++generations;
            }
        }

        for (std::size_t generation = 0; generation < generations; ++generation) {
            domain::ConwaySimulation::advance(field);
        }

        return generations;
    }

    void SimulationScheduler::clearAccumulator() noexcept { accumulator_ = Duration::zero(); }

} // namespace lifeGame::application
