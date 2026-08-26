#include <array>
#include <cstddef>
#include <cstdint>
#include <vector>

#include <catch2/catch_test_macros.hpp>
#include <application/field-command-executor.hpp>
#include <domain/field/field.hpp>
#include <presentation/input/input-router.hpp>
#include <presentation/rendering/field-renderer.hpp>
#include <presentation/ui/toolbar.hpp>

namespace {

    using lifeGame::application::FieldCommandExecutor;
    using lifeGame::domain::Field;
    using lifeGame::presentation::FieldRenderer;
    using lifeGame::presentation::InputRouter;
    using lifeGame::presentation::LogicalPoint;
    using lifeGame::presentation::PointerSample;

    auto pointAt(const Field& field, std::size_t x, std::size_t y) -> LogicalPoint {
        const auto plan = FieldRenderer::calculateRenderPlan(field, 1280, 720);
        const auto cellSize = static_cast<float>(plan.cellSize);
        const auto cellCenter = cellSize / 2.0F;
        return LogicalPoint{plan.fieldRectangle.x + static_cast<float>(x) * cellSize + cellCenter,
                            plan.fieldRectangle.y + static_cast<float>(y) * cellSize + cellCenter};
    }

    void execute(Field& field,
                 const std::vector<lifeGame::application::PaintLiveCommand>& commands) {
        for (const auto& command : commands) {
            FieldCommandExecutor::execute(field, command);
        }
    }

    TEST_CASE("Input router paints exactly the pressed in-bounds cell") {
        auto fieldResult = Field::create(50, 50);
        REQUIRE(fieldResult);
        auto& field = fieldResult.value();
        const auto before = field.cells();
        InputRouter router;

        const auto commands = router.sample(
            field, 1280, 720, PointerSample{pointAt(field, 2, 3), true, true, false});
        REQUIRE(commands.size() == 1);
        CHECK(commands.front().coordinate.x == 2);
        CHECK(commands.front().coordinate.y == 3);
        execute(field, commands);

        for (std::size_t index = 0; index < field.cells().size(); ++index) {
            const auto expected = index == 3 * field.width() + 2 ? std::uint8_t{1} : before[index];
            CHECK(field.cells()[index] == expected);
        }
    }

    TEST_CASE("Input router fills captured sparse drags and ends capture on release") {
        auto fieldResult = Field::create(50, 50);
        REQUIRE(fieldResult);
        auto& field = fieldResult.value();
        InputRouter router;

        execute(field, router.sample(field, 1280, 720,
                                     PointerSample{pointAt(field, 1, 1), true, true, false}));
        execute(field, router.sample(field, 1280, 720,
                                     PointerSample{pointAt(field, 6, 1), false, true, false}));
        execute(field, router.sample(field, 1280, 720,
                                     PointerSample{pointAt(field, 6, 1), false, true, false}));

        for (std::size_t x = 1; x <= 6; ++x) {
            CHECK(field.isLive(lifeGame::domain::CellCoordinate{x, 1}));
        }

        const auto beforeOutside = field.cells();
        CHECK(router.sample(field, 1280, 720, PointerSample{LogicalPoint{0.0F, 0.0F}, false,
                                                            true, false})
                  .empty());
        CHECK(field.cells() == beforeOutside);

        execute(field, router.sample(field, 1280, 720,
                                     PointerSample{pointAt(field, 8, 1), false, true, false}));
        for (std::size_t x = 1; x <= 8; ++x) {
            CHECK(field.isLive(lifeGame::domain::CellCoordinate{x, 1}));
        }

        const auto beforeReleaseOutside = field.cells();
        CHECK(router.sample(field, 1280, 720,
                            PointerSample{LogicalPoint{0.0F, 0.0F}, false, false, true})
                  .empty());
        CHECK(field.cells() == beforeReleaseOutside);

        const auto newPress = router.sample(
            field, 1280, 720, PointerSample{pointAt(field, 10, 1), true, true, false});
        REQUIRE(newPress.size() == 1);
        CHECK(newPress.front().coordinate.x == 10);
        CHECK(newPress.front().coordinate.y == 1);
    }

    TEST_CASE("Input router paints the final in-bounds cell on release") {
        auto fieldResult = Field::create(50, 50);
        REQUIRE(fieldResult);
        auto& field = fieldResult.value();
        InputRouter router;

        execute(field, router.sample(field, 1280, 720,
                                     PointerSample{pointAt(field, 1, 1), true, true, false}));
        execute(field, router.sample(field, 1280, 720,
                                     PointerSample{pointAt(field, 4, 1), false, false, true}));

        for (std::size_t x = 1; x <= 4; ++x) {
            CHECK(field.isLive(lifeGame::domain::CellCoordinate{x, 1}));
        }
    }

    TEST_CASE("Input router consumes modal, toolbar, and out-of-field presses") {
        auto fieldResult = Field::create(50, 50);
        REQUIRE(fieldResult);
        auto& field = fieldResult.value();
        InputRouter router;
        const auto before = field.cells();
        const auto toolbar = lifeGame::presentation::Toolbar::calculateLayout(1280, 720);
        const auto toolbarPoint = LogicalPoint{toolbar.panel.x + 1.0F, toolbar.panel.y + 1.0F};
        const auto fieldPoint = pointAt(field, 4, 4);

        CHECK(router.sample(field, 1280, 720,
                            PointerSample{toolbarPoint, true, true, false})
                  .empty());
        CHECK(router.sample(field, 1280, 720,
                            PointerSample{fieldPoint, true, true, false}, true)
                  .empty());
        CHECK(router.sample(field, 1280, 720,
                            PointerSample{fieldPoint, true, false, false})
                  .empty());
        CHECK(router.sample(field, 1280, 720,
                            PointerSample{LogicalPoint{0.0F, 0.0F}, true, true, false})
                  .empty());
        CHECK(field.cells() == before);
    }

    TEST_CASE("Input router ignores held and released input in gray space") {
        auto fieldResult = Field::create(50, 50);
        REQUIRE(fieldResult);
        auto& field = fieldResult.value();
        InputRouter router;
        const auto before = field.cells();
        const auto plan = FieldRenderer::calculateRenderPlan(field, 1280, 720);
        const auto topGray = LogicalPoint{
            plan.fieldRectangle.x + plan.fieldRectangle.width / 2.0F,
            plan.fieldRectangle.y - 1.0F};
        const auto rightGray = LogicalPoint{
            plan.fieldRectangle.x + plan.fieldRectangle.width + 1.0F,
            plan.fieldRectangle.y + plan.fieldRectangle.height / 2.0F};
        const auto bottomGray = LogicalPoint{
            plan.fieldRectangle.x + plan.fieldRectangle.width / 2.0F,
            plan.fieldRectangle.y + plan.fieldRectangle.height + 1.0F};

        CHECK(router.sample(field, 1280, 720, PointerSample{topGray, true, true, false})
                  .empty());
        CHECK(router.sample(field, 1280, 720, PointerSample{rightGray, true, true, false})
                  .empty());
        CHECK(router.sample(field, 1280, 720, PointerSample{rightGray, false, true, false})
                  .empty());
        CHECK(router.sample(field, 1280, 720, PointerSample{bottomGray, false, false, true})
                  .empty());
        CHECK(field.cells() == before);
    }

    TEST_CASE("Input router does not capture after a rejected press") {
        auto fieldResult = Field::create(50, 50);
        REQUIRE(fieldResult);
        auto& field = fieldResult.value();
        InputRouter router;
        const auto before = field.cells();
        const auto toolbar = lifeGame::presentation::Toolbar::calculateLayout(1280, 720);
        const auto toolbarPoint = LogicalPoint{toolbar.panel.x + 1.0F, toolbar.panel.y + 1.0F};

        CHECK(router.sample(field, 1280, 720,
                            PointerSample{toolbarPoint, true, true, false})
                  .empty());
        CHECK(router.sample(field, 1280, 720,
                            PointerSample{pointAt(field, 5, 5), false, true, false})
                  .empty());
        CHECK(field.cells() == before);
    }

    TEST_CASE("Input router does not paint through ownership changes during a gesture") {
        auto fieldResult = Field::create(1280, 720);
        REQUIRE(fieldResult);
        auto& field = fieldResult.value();
        InputRouter router;
        const auto fieldPoint = pointAt(field, 10, 100);
        const auto toolbar = lifeGame::presentation::Toolbar::calculateLayout(1280, 720);
        const auto toolbarPoint = LogicalPoint{toolbar.panel.x + 1.0F, toolbar.panel.y + 1.0F};

        execute(field, router.sample(field, 1280, 720,
                                     PointerSample{fieldPoint, true, true, false}));
        CHECK(router.sample(field, 1280, 720,
                            PointerSample{toolbarPoint, false, true, false})
                  .empty());
        CHECK(router.sample(field, 1280, 720,
                            PointerSample{fieldPoint, false, true, false}, true)
                  .empty());
        CHECK(router.sample(field, 1280, 720,
                            PointerSample{fieldPoint, false, false, true}, true)
                  .empty());

        const auto beforeNextPress = field.cells();
        const auto nextPress = router.sample(
            field, 1280, 720, PointerSample{pointAt(field, 12, 100), true, true, false});
        REQUIRE(nextPress.size() == 1);
        CHECK(nextPress.front().coordinate.x == 12);
        CHECK(nextPress.front().coordinate.y == 100);
        CHECK(field.cells() == beforeNextPress);
    }

    TEST_CASE("Input router starts a fresh segment after overlay ownership") {
        auto fieldResult = Field::create(1280, 720);
        REQUIRE(fieldResult);
        auto& field = fieldResult.value();
        InputRouter router;
        const auto firstPoint = pointAt(field, 900, 100);
        const auto reentryPoint = pointAt(field, 1279, 100);
        const auto toolbar = lifeGame::presentation::Toolbar::calculateLayout(1280, 720);
        const auto toolbarPoint = LogicalPoint{toolbar.panel.x + 1.0F, toolbar.panel.y + 1.0F};

        execute(field, router.sample(field, 1280, 720,
                                     PointerSample{firstPoint, true, true, false}));
        CHECK(router.sample(field, 1280, 720,
                            PointerSample{toolbarPoint, false, true, false})
                  .empty());
        const auto reentry = router.sample(
            field, 1280, 720, PointerSample{reentryPoint, false, true, false});

        REQUIRE(reentry.size() == 1);
        CHECK(reentry.front().coordinate.x == 1279);
        CHECK(reentry.front().coordinate.y == 100);
        CHECK_FALSE(field.isLive(lifeGame::domain::CellCoordinate{960, 100}));
    }

    TEST_CASE("Input router clears a gesture when the release edge is lost") {
        auto fieldResult = Field::create(50, 50);
        REQUIRE(fieldResult);
        auto& field = fieldResult.value();
        InputRouter router;

        execute(field, router.sample(field, 1280, 720,
                                     PointerSample{pointAt(field, 1, 1), true, true, false}));
        CHECK(router.sample(field, 1280, 720,
                            PointerSample{pointAt(field, 3, 1), false, false, false})
                  .empty());

        const auto nextPress = router.sample(
            field, 1280, 720, PointerSample{pointAt(field, 8, 1), true, true, false});
        REQUIRE(nextPress.size() == 1);
        CHECK(nextPress.front().coordinate.x == 8);
        CHECK(nextPress.front().coordinate.y == 1);
    }

    TEST_CASE("Input router paints every cell on a diagonal drag") {
        auto fieldResult = Field::create(50, 50);
        REQUIRE(fieldResult);
        auto& field = fieldResult.value();
        InputRouter router;

        execute(field, router.sample(field, 1280, 720,
                                     PointerSample{pointAt(field, 1, 1), true, true, false}));
        execute(field, router.sample(field, 1280, 720,
                                     PointerSample{pointAt(field, 4, 3), false, true, false}));

        const auto expected = std::array<lifeGame::domain::CellCoordinate, 4>{
            lifeGame::domain::CellCoordinate{1, 1},
            lifeGame::domain::CellCoordinate{2, 2},
            lifeGame::domain::CellCoordinate{3, 2},
            lifeGame::domain::CellCoordinate{4, 3},
        };
        for (const auto coordinate : expected) {
            CHECK(field.isLive(coordinate));
        }
    }

    TEST_CASE("Input router emits only the rasterized cells for a diagonal drag") {
        auto fieldResult = Field::create(50, 50);
        REQUIRE(fieldResult);
        auto& field = fieldResult.value();
        InputRouter router;

        static_cast<void>(router.sample(field, 1280, 720,
                                         PointerSample{pointAt(field, 1, 1), true, true, false}));
        const auto commands = router.sample(
            field, 1280, 720, PointerSample{pointAt(field, 4, 3), false, true, false});
        const auto expected = std::array<lifeGame::domain::CellCoordinate, 4>{
            lifeGame::domain::CellCoordinate{1, 1},
            lifeGame::domain::CellCoordinate{2, 2},
            lifeGame::domain::CellCoordinate{3, 2},
            lifeGame::domain::CellCoordinate{4, 3},
        };

        REQUIRE(commands.size() == expected.size());
        for (std::size_t index = 0; index < commands.size(); ++index) {
            CHECK(commands[index].coordinate.x == expected[index].x);
            CHECK(commands[index].coordinate.y == expected[index].y);
        }
    }

    TEST_CASE("Field command executor ignores invalid coordinates") {
        auto fieldResult = Field::create(4, 3);
        REQUIRE(fieldResult);
        auto& field = fieldResult.value();
        const auto before = field.cells();

        FieldCommandExecutor::execute(
            field, lifeGame::application::PaintLiveCommand{lifeGame::domain::CellCoordinate{4, 0}});
        FieldCommandExecutor::execute(
            field, lifeGame::application::PaintLiveCommand{lifeGame::domain::CellCoordinate{0, 3}});

        CHECK(field.cells() == before);
    }

} // namespace
