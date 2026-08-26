#include <cstdint>
#include <initializer_list>
#include <vector>

#include <catch2/catch_test_macros.hpp>
#include <domain/field/cell-coordinate.hpp>
#include <domain/field/field.hpp>
#include <domain/simulation/conway-simulation.hpp>

namespace {

    using lifeGame::domain::CellCoordinate;
    using lifeGame::domain::ConwaySimulation;
    using lifeGame::domain::Field;

    void setLive(Field& field, std::initializer_list<CellCoordinate> coordinates) {
        for (const auto coordinate : coordinates) {
            REQUIRE(field.setLive(coordinate, true));
        }
    }

    TEST_CASE("A lone cell dies after one Conway generation") {
        auto fieldResult = Field::create(3, 3);
        REQUIRE(fieldResult);
        auto& field = fieldResult.value();
        setLive(field, {{1, 1}});

        ConwaySimulation::advance(field);
        CHECK(field.cells() == std::vector<std::uint8_t>(9, 0));

        ConwaySimulation::advance(field);
        CHECK(field.cells() == std::vector<std::uint8_t>(9, 0));
    }

    TEST_CASE("Underpopulation and overpopulation kill live cells") {
        auto fieldResult = Field::create(7, 5);
        REQUIRE(fieldResult);
        auto& field = fieldResult.value();
        setLive(field, {{1, 2}, {0, 2}});
        setLive(field, {{5, 2}, {4, 2}, {6, 2}, {5, 1}, {5, 3}});

        ConwaySimulation::advance(field);

        CHECK_FALSE(field.isLive({1, 2}));
        CHECK_FALSE(field.isLive({5, 2}));
    }

    TEST_CASE("A stable block remains unchanged") {
        auto fieldResult = Field::create(4, 4);
        REQUIRE(fieldResult);
        auto& field = fieldResult.value();
        setLive(field, {{1, 1}, {2, 1}, {1, 2}, {2, 2}});
        const auto before = field.cells();

        ConwaySimulation::advance(field);

        CHECK(field.cells() == before);
    }

    TEST_CASE("A blinker has period two") {
        auto fieldResult = Field::create(5, 5);
        REQUIRE(fieldResult);
        auto& field = fieldResult.value();
        setLive(field, {{2, 1}, {2, 2}, {2, 3}});

        ConwaySimulation::advance(field);
        const auto firstGeneration = field.cells();
        auto expectedFirst = std::vector<std::uint8_t>(25, 0);
        expectedFirst[11] = 1;
        expectedFirst[12] = 1;
        expectedFirst[13] = 1;
        CHECK(firstGeneration == expectedFirst);

        ConwaySimulation::advance(field);
        auto expectedSecond = std::vector<std::uint8_t>(25, 0);
        expectedSecond[7] = 1;
        expectedSecond[12] = 1;
        expectedSecond[17] = 1;
        CHECK(field.cells() == expectedSecond);
    }

    TEST_CASE("Out-of-bounds neighbors are permanently dead") {
        auto fieldResult = Field::create(3, 3);
        REQUIRE(fieldResult);
        auto& field = fieldResult.value();
        setLive(field, {{0, 0}, {2, 0}, {0, 2}});

        ConwaySimulation::advance(field);

        auto expected = std::vector<std::uint8_t>(9, 0);
        expected[4] = 1;
        CHECK(field.cells() == expected);
    }

    TEST_CASE("Rectangular and one-dimensional fields use finite boundaries") {
        auto rectangularResult = Field::create(4, 2);
        auto horizontalResult = Field::create(1, 3);
        auto verticalResult = Field::create(3, 1);
        REQUIRE(rectangularResult);
        REQUIRE(horizontalResult);
        REQUIRE(verticalResult);

        auto& rectangular = rectangularResult.value();
        setLive(rectangular, {{1, 0}, {2, 0}, {1, 1}, {2, 1}});
        ConwaySimulation::advance(rectangular);
        CHECK(rectangular.isLive({1, 0}));
        CHECK(rectangular.isLive({2, 0}));
        CHECK(rectangular.isLive({1, 1}));
        CHECK(rectangular.isLive({2, 1}));

        auto& horizontal = horizontalResult.value();
        setLive(horizontal, {{0, 0}, {0, 1}, {0, 2}});
        ConwaySimulation::advance(horizontal);
        CHECK_FALSE(horizontal.isLive({0, 0}));
        CHECK(horizontal.isLive({0, 1}));
        CHECK_FALSE(horizontal.isLive({0, 2}));

        auto& vertical = verticalResult.value();
        setLive(vertical, {{0, 0}, {1, 0}, {2, 0}});
        ConwaySimulation::advance(vertical);
        CHECK_FALSE(vertical.isLive({0, 0}));
        CHECK(vertical.isLive({1, 0}));
        CHECK_FALSE(vertical.isLive({2, 0}));
    }

    TEST_CASE("Repeated generation evaluation is deterministic") {
        auto firstResult = Field::create(5, 5);
        auto secondResult = Field::create(5, 5);
        REQUIRE(firstResult);
        REQUIRE(secondResult);
        auto& first = firstResult.value();
        auto& second = secondResult.value();
        setLive(first, {{1, 2}, {2, 2}, {3, 2}});
        setLive(second, {{1, 2}, {2, 2}, {3, 2}});

        ConwaySimulation::advance(first);
        ConwaySimulation::advance(second);
        ConwaySimulation::advance(first);
        ConwaySimulation::advance(second);

        CHECK(first.cells() == second.cells());
    }

    TEST_CASE("A complete next buffer is overwritten before publication") {
        auto fieldResult = Field::create(5, 5);
        REQUIRE(fieldResult);
        auto& field = fieldResult.value();
        setLive(field, {{2, 1}, {2, 2}, {2, 3}});

        ConwaySimulation::advance(field);
        for (std::size_t y = 0; y < field.height(); ++y) {
            for (std::size_t x = 0; x < field.width(); ++x) {
                REQUIRE(field.setLive({x, y}, false));
            }
        }

        ConwaySimulation::advance(field);

        CHECK(field.cells() == std::vector<std::uint8_t>(25, 0));
    }

} // namespace
