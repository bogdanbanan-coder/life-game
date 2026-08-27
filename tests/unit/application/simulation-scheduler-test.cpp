#include <chrono>

#include <application/field-command-executor.hpp>
#include <application/commands/field-command.hpp>
#include <application/simulation/simulation-scheduler.hpp>
#include <catch2/catch_test_macros.hpp>
#include <domain/field/field.hpp>
#include <domain/simulation/conway-simulation.hpp>

namespace {

    using namespace std::chrono_literals;

    using lifeGame::application::SimulationScheduler;
    using lifeGame::domain::Field;

    TEST_CASE("A generation becomes due at the default interval") {
        auto fieldResult = Field::create(3, 3);
        REQUIRE(fieldResult);
        auto& field = fieldResult.value();
        SimulationScheduler scheduler;

        CHECK(scheduler.advance(field, 249ms) == 0);
        CHECK(scheduler.advance(field, 1ms) == 1);
    }

    TEST_CASE("The scheduler retains a fractional interval remainder") {
        auto fieldResult = Field::create(3, 3);
        REQUIRE(fieldResult);
        auto& field = fieldResult.value();
        SimulationScheduler scheduler;

        CHECK(scheduler.advance(field, 125ms) == 0);
        CHECK(scheduler.advance(field, 124ms) == 0);
        CHECK(scheduler.advance(field, 1ms) == 1);
    }

    TEST_CASE("The scheduler caps catch-up and discards excess backlog") {
        auto fieldResult = Field::create(3, 3);
        REQUIRE(fieldResult);
        auto& field = fieldResult.value();
        SimulationScheduler scheduler;

        CHECK(scheduler.advance(field, 1'375ms) == 4);
        CHECK(scheduler.advance(field, 124ms) == 0);
        CHECK(scheduler.advance(field, 1ms) == 1);
    }

    TEST_CASE("Capped catch-up retains a pre-existing fractional remainder") {
        auto fieldResult = Field::create(1, 1);
        REQUIRE(fieldResult);
        auto& field = fieldResult.value();
        SimulationScheduler scheduler;

        CHECK(scheduler.advance(field, 125ms) == 0);
        CHECK(scheduler.advance(field, 1'126ms) == 4);
        CHECK(scheduler.advance(field, 248ms) == 0);
        CHECK(scheduler.advance(field, 1ms) == 1);
    }

    TEST_CASE("Capped catch-up applies four sequential generations") {
        auto fieldResult = Field::create(5, 5);
        auto expectedResult = Field::create(5, 5);
        REQUIRE(fieldResult);
        REQUIRE(expectedResult);
        auto& field = fieldResult.value();
        auto& expected = expectedResult.value();
        for (auto* target : {&field, &expected}) {
            REQUIRE(target->setLive({1, 0}, true));
            REQUIRE(target->setLive({2, 1}, true));
            REQUIRE(target->setLive({0, 2}, true));
            REQUIRE(target->setLive({1, 2}, true));
            REQUIRE(target->setLive({2, 2}, true));
        }
        for (int generation = 0; generation < 4; ++generation) {
            lifeGame::domain::ConwaySimulation::advance(expected);
        }
        SimulationScheduler scheduler;

        CHECK(scheduler.advance(field, 1'250ms) == 4);
        CHECK(field.cells() == expected.cells());
    }

    TEST_CASE("Catch-up generations run sequentially without intermediate scheduling") {
        auto fieldResult = Field::create(5, 5);
        REQUIRE(fieldResult);
        auto& field = fieldResult.value();
        REQUIRE(field.setLive({2, 1}, true));
        REQUIRE(field.setLive({2, 2}, true));
        REQUIRE(field.setLive({2, 3}, true));
        SimulationScheduler scheduler;

        CHECK(scheduler.advance(field, 500ms) == 2);
        CHECK(field.isLive({2, 1}));
        CHECK(field.isLive({2, 2}));
        CHECK(field.isLive({2, 3}));
        CHECK_FALSE(field.isLive({1, 2}));
        CHECK_FALSE(field.isLive({3, 2}));
    }

    TEST_CASE("Non-positive elapsed time does not advance or mutate the field") {
        auto fieldResult = Field::create(3, 3);
        REQUIRE(fieldResult);
        auto& field = fieldResult.value();
        REQUIRE(field.setLive({1, 1}, true));
        SimulationScheduler scheduler;

        CHECK(scheduler.advance(field, 125ms) == 0);
        CHECK(scheduler.advance(field, 0ms) == 0);
        CHECK(scheduler.advance(field, -1ms) == 0);
        CHECK(scheduler.advance(field, 124ms) == 0);
        CHECK(scheduler.advance(field, 1ms) == 1);
    }

    TEST_CASE("The scheduler handles the largest representable elapsed duration") {
        auto fieldResult = Field::create(1, 1);
        REQUIRE(fieldResult);
        auto& field = fieldResult.value();
        SimulationScheduler scheduler;

        CHECK(scheduler.advance(field, SimulationScheduler::Duration::max()) == 4);
    }

    TEST_CASE("Post-batch edits remain for the next generation") {
        auto fieldResult = Field::create(3, 3);
        REQUIRE(fieldResult);
        auto& field = fieldResult.value();
        SimulationScheduler scheduler;

        CHECK(scheduler.advance(field, 250ms) == 1);
        lifeGame::application::FieldCommandExecutor::execute(
            field, lifeGame::application::PaintLiveCommand{{1, 1}});
        CHECK(field.isLive({1, 1}));

        CHECK(scheduler.advance(field, 250ms) == 1);
        CHECK_FALSE(field.isLive({1, 1}));
    }

    TEST_CASE("Clearing the accumulator discards partial timing debt") {
        auto fieldResult = Field::create(5, 5);
        REQUIRE(fieldResult);
        auto& field = fieldResult.value();
        REQUIRE(field.setLive({2, 1}, true));
        REQUIRE(field.setLive({2, 2}, true));
        REQUIRE(field.setLive({2, 3}, true));
        const auto before = field.cells();
        SimulationScheduler scheduler;

        CHECK(scheduler.advance(field, 125ms) == 0);
        scheduler.clearAccumulator();

        CHECK(field.cells() == before);
        CHECK(scheduler.advance(field, 249ms) == 0);
        CHECK(scheduler.advance(field, 1ms) == 1);
        CHECK(field.isLive({1, 2}));
        CHECK(field.isLive({2, 2}));
        CHECK(field.isLive({3, 2}));
    }

} // namespace
