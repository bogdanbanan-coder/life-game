#include <catch2/catch_test_macros.hpp>
#include <domain/field/field-dimensions.hpp>
#include <presentation/input/drag-rasterizer.hpp>

namespace {

    using lifeGame::domain::CellCoordinate;
    using lifeGame::presentation::DragRasterizer;

    TEST_CASE("Drag rasterization fills sparse horizontal segments") {
        const auto cells = DragRasterizer::rasterize(CellCoordinate{2, 4}, CellCoordinate{6, 4});

        REQUIRE(cells.size() == 5);
        for (std::size_t index = 0; index < cells.size(); ++index) {
            CHECK(cells[index].x == index + 2);
            CHECK(cells[index].y == 4);
        }
    }

    TEST_CASE("Drag rasterization fills sparse diagonal segments") {
        const auto cells = DragRasterizer::rasterize(CellCoordinate{0, 0}, CellCoordinate{4, 2});
        const CellCoordinate expected[] = {{0, 0}, {1, 0}, {2, 1}, {3, 1}, {4, 2}};

        REQUIRE(cells.size() == 5);
        for (std::size_t index = 0; index < cells.size(); ++index) {
            CHECK(cells[index].x == expected[index].x);
            CHECK(cells[index].y == expected[index].y);
        }
    }

    TEST_CASE("Drag rasterization handles reverse and vertical segments") {
        const auto reverse = DragRasterizer::rasterize(CellCoordinate{6, 4}, CellCoordinate{2, 4});
        const auto vertical = DragRasterizer::rasterize(CellCoordinate{3, 6}, CellCoordinate{3, 2});

        REQUIRE(reverse.size() == 5);
        REQUIRE(vertical.size() == 5);
        for (std::size_t index = 0; index < reverse.size(); ++index) {
            CHECK(reverse[index].x == 6 - index);
            CHECK(reverse[index].y == 4);
            CHECK(vertical[index].x == 3);
            CHECK(vertical[index].y == 6 - index);
        }
    }

    TEST_CASE("Drag rasterization handles steep and reverse diagonal segments") {
        const auto steep = DragRasterizer::rasterize(CellCoordinate{2, 1}, CellCoordinate{3, 6});
        const auto reverse = DragRasterizer::rasterize(CellCoordinate{4, 3}, CellCoordinate{0, 0});
        const CellCoordinate expectedSteep[] = {
            {2, 1}, {2, 2}, {2, 3}, {3, 4}, {3, 5}, {3, 6}};
        const CellCoordinate expectedReverse[] = {{4, 3}, {3, 2}, {2, 1}, {1, 1}, {0, 0}};

        REQUIRE(steep.size() == 6);
        REQUIRE(reverse.size() == 5);
        for (std::size_t index = 0; index < steep.size(); ++index) {
            CHECK(steep[index].x == expectedSteep[index].x);
            CHECK(steep[index].y == expectedSteep[index].y);
        }
        for (std::size_t index = 0; index < reverse.size(); ++index) {
            CHECK(reverse[index].x == expectedReverse[index].x);
            CHECK(reverse[index].y == expectedReverse[index].y);
        }
    }

    TEST_CASE("Drag rasterization emits one cell for a same-cell segment") {
        const auto cells = DragRasterizer::rasterize(CellCoordinate{3, 3}, CellCoordinate{3, 3});

        REQUIRE(cells.size() == 1);
        CHECK(cells.front().x == 3);
        CHECK(cells.front().y == 3);
    }

    TEST_CASE("Drag rasterization rejects spans beyond the field safety limit") {
        const auto cells = DragRasterizer::rasterize(
            CellCoordinate{0, 0},
            CellCoordinate{lifeGame::domain::FieldDimensions::MAX_TOTAL_CELLS, 0});

        CHECK(cells.empty());
    }

} // namespace
