#include <cstddef>
#include <cstdint>

#include <catch2/catch_test_macros.hpp>
#include <domain/field/cell-coordinate.hpp>
#include <domain/field/field.hpp>
#include <foundation/error-code.hpp>

namespace {

    using lifeGame::domain::CellCoordinate;
    using lifeGame::domain::Field;
    using lifeGame::foundation::ErrorCode;

    TEST_CASE("Field starts with a dense dead-cell buffer") {
        const auto field = Field::create(50, 50);

        REQUIRE(field);
        CHECK(field.value().dimensions().width == 50);
        CHECK(field.value().dimensions().height == 50);
        CHECK(field.value().cells().size() == 2'500);
        for (const auto cell : field.value().cells()) {
            CHECK(cell == std::uint8_t{0});
        }
    }

    TEST_CASE("Field exposes independent row-major positions") {
        auto fieldResult = Field::create(4, 3);
        REQUIRE(fieldResult);
        auto& field = fieldResult.value();

        CHECK(field.setLive(CellCoordinate{1, 0}, true));
        CHECK(field.setLive(CellCoordinate{0, 1}, true));
        CHECK(field.setLive(CellCoordinate{2, 1}, true));

        CHECK(field.cells()[1] == std::uint8_t{1});
        CHECK(field.cells()[4] == std::uint8_t{1});
        CHECK(field.cells()[6] == std::uint8_t{1});
        std::uint8_t value = 0;
        CHECK(field.readCell(CellCoordinate{1, 0}, value));
        CHECK(value == std::uint8_t{1});
        CHECK(field.readCell(CellCoordinate{0, 1}, value));
        CHECK(value == std::uint8_t{1});
        CHECK(field.readCell(CellCoordinate{2, 1}, value));
        CHECK(value == std::uint8_t{1});
    }

    TEST_CASE("Field reads and writes in-bounds live state") {
        auto fieldResult = Field::create(3, 2);
        REQUIRE(fieldResult);
        auto& field = fieldResult.value();
        const auto before = field.cells();

        const auto coordinate = CellCoordinate{2, 1};
        CHECK_FALSE(field.isLive(coordinate));
        CHECK(field.setLive(coordinate, true));
        CHECK(field.isLive(coordinate));
        std::uint8_t value = 0;
        CHECK(field.readCell(coordinate, value));
        CHECK(value == std::uint8_t{1});
        CHECK(field.cells()[5] == std::uint8_t{1});
        for (std::size_t index = 0; index < field.cells().size(); ++index) {
            if (index != std::size_t{5}) {
                CHECK(field.cells()[index] == before[index]);
            }
        }
        CHECK(field.setLive(coordinate, false));
        CHECK_FALSE(field.isLive(coordinate));
        CHECK(field.readCell(coordinate, value));
        CHECK(value == std::uint8_t{0});
        CHECK(field.cells() == before);
    }

    TEST_CASE("Field rejects invalid dimensions without constructing storage") {
        const auto field = Field::create(0, 50);

        REQUIRE_FALSE(field);
        CHECK(field.error() == ErrorCode::InvalidArgument);
    }

    TEST_CASE("Field out-of-bounds access is a false or no-op result") {
        auto fieldResult = Field::create(3, 2);
        REQUIRE(fieldResult);
        auto& field = fieldResult.value();
        const auto before = field.cells();

        const auto outOfBounds = CellCoordinate{3, 0};
        std::uint8_t value = 7;
        CHECK_FALSE(field.contains(outOfBounds));
        CHECK_FALSE(field.isLive(outOfBounds));
        CHECK_FALSE(field.readCell(outOfBounds, value));
        CHECK(value == std::uint8_t{7});
        CHECK_FALSE(field.setLive(outOfBounds, true));
        CHECK(field.cells() == before);

        const auto otherOutOfBounds = CellCoordinate{0, 2};
        CHECK_FALSE(field.setLive(otherOutOfBounds, true));
        CHECK(field.cells() == before);
    }

} // namespace
