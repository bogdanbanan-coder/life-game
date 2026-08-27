#include <array>
#include <cstddef>
#include <cstdint>
#include <variant>
#include <vector>

#include <catch2/catch_test_macros.hpp>
#include <application/field-command-executor.hpp>
#include <domain/field/field.hpp>
#include <presentation/input/input-router.hpp>
#include <presentation/rendering/field-renderer.hpp>
#include <presentation/ui/toolbar.hpp>

namespace {

    using lifeGame::application::FieldCommandExecutor;
    using lifeGame::application::PaintDeadCommand;
    using lifeGame::application::PaintMode;
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

    void execute(Field& field, const lifeGame::presentation::InputCommands& commands) {
        for (const auto& command : commands.paintCommands) {
            FieldCommandExecutor::execute(field, command);
        }
    }

    TEST_CASE("Input router emits a typed Die command for an in-bounds press") {
        auto fieldResult = Field::create(50, 50);
        REQUIRE(fieldResult);
        auto& field = fieldResult.value();
        REQUIRE(field.setLive({2, 3}, true));
        REQUIRE(field.setLive({3, 3}, true));
        const auto before = field.cells();
        InputRouter router;

        const auto commands = router.sample(
            field, 1280, 720, PointerSample{pointAt(field, 2, 3), true, true, false},
            PaintMode::Die);
        REQUIRE(commands.paintCommands.size() == 1);
        REQUIRE(std::holds_alternative<PaintDeadCommand>(commands.paintCommands.front()));
        CHECK(std::get<PaintDeadCommand>(commands.paintCommands.front()).coordinate.x == 2);
        CHECK(std::get<PaintDeadCommand>(commands.paintCommands.front()).coordinate.y == 3);
        execute(field, commands);

        CHECK_FALSE(field.isLive({2, 3}));
        CHECK(field.isLive({3, 3}));
        for (std::size_t index = 0; index < field.cells().size(); ++index) {
            const auto expected = index == 3 * field.width() + 2 ? std::uint8_t{0} : before[index];
            CHECK(field.cells()[index] == expected);
        }
    }

    TEST_CASE("Input router fills a captured sparse Die drag") {
        auto fieldResult = Field::create(50, 50);
        REQUIRE(fieldResult);
        auto& field = fieldResult.value();
        for (std::size_t x = 1; x <= 7; ++x) {
            REQUIRE(field.setLive({x, 1}, true));
        }
        REQUIRE(field.setLive({10, 10}, true));
        InputRouter router;

        const auto start = router.sample(
            field, 1280, 720, PointerSample{pointAt(field, 1, 1), true, true, false},
            PaintMode::Die);
        REQUIRE(start.paintCommands.size() == 1);
        REQUIRE(std::holds_alternative<PaintDeadCommand>(start.paintCommands.front()));
        CHECK(std::get<PaintDeadCommand>(start.paintCommands.front()).coordinate.x == 1);
        CHECK(std::get<PaintDeadCommand>(start.paintCommands.front()).coordinate.y == 1);
        execute(field, start);

        const auto drag = router.sample(
            field, 1280, 720, PointerSample{pointAt(field, 6, 1), false, true, false},
            PaintMode::Die);
        REQUIRE(drag.paintCommands.size() == 6);
        for (std::size_t index = 0; index < drag.paintCommands.size(); ++index) {
            REQUIRE(std::holds_alternative<PaintDeadCommand>(drag.paintCommands[index]));
            const auto coordinate =
                std::get<PaintDeadCommand>(drag.paintCommands[index]).coordinate;
            CHECK(coordinate.x == index + 1);
            CHECK(coordinate.y == 1);
        }
        execute(field, drag);

        CHECK(router.sample(field, 1280, 720,
                            PointerSample{pointAt(field, 6, 1), false, true, false},
                            PaintMode::Die)
                  .paintCommands.empty());

        const auto release = router.sample(
            field, 1280, 720, PointerSample{pointAt(field, 7, 1), false, false, true},
            PaintMode::Die);
        REQUIRE(release.paintCommands.size() == 2);
        execute(field, release);

        for (std::size_t x = 1; x <= 7; ++x) {
            CHECK_FALSE(field.isLive(lifeGame::domain::CellCoordinate{x, 1}));
        }
        CHECK(field.isLive({10, 10}));
    }

    TEST_CASE("Input router reports Die selection without starting a field gesture") {
        auto fieldResult = Field::create(50, 50);
        REQUIRE(fieldResult);
        auto& field = fieldResult.value();
        InputRouter router;
        const auto toolbar = lifeGame::presentation::Toolbar::calculateLayout(1280, 720);
        const auto dieButton = toolbar.controls[1];
        const auto diePoint = LogicalPoint{dieButton.x + dieButton.width / 2.0F,
                                           dieButton.y + dieButton.height / 2.0F};

        const auto commands = router.sample(
            field, 1280, 720, PointerSample{diePoint, true, true, false}, PaintMode::Live);

        REQUIRE(commands.selectedPaintMode);
        CHECK(*commands.selectedPaintMode == PaintMode::Die);
        CHECK(commands.paintCommands.empty());
    }

    TEST_CASE("Input router emits the exact typed Die diagonal path") {
        auto fieldResult = Field::create(50, 50);
        REQUIRE(fieldResult);
        auto& field = fieldResult.value();
        InputRouter router;

        static_cast<void>(router.sample(
            field, 1280, 720, PointerSample{pointAt(field, 1, 1), true, true, false},
            PaintMode::Die));
        const auto commands = router.sample(
            field, 1280, 720, PointerSample{pointAt(field, 4, 3), false, true, false},
            PaintMode::Die);
        const auto expected = std::array<lifeGame::domain::CellCoordinate, 4>{
            lifeGame::domain::CellCoordinate{1, 1},
            lifeGame::domain::CellCoordinate{2, 2},
            lifeGame::domain::CellCoordinate{3, 2},
            lifeGame::domain::CellCoordinate{4, 3},
        };

        REQUIRE(commands.paintCommands.size() == expected.size());
        for (std::size_t index = 0; index < commands.paintCommands.size(); ++index) {
            REQUIRE(std::holds_alternative<PaintDeadCommand>(commands.paintCommands[index]));
            const auto coordinate =
                std::get<PaintDeadCommand>(commands.paintCommands[index]).coordinate;
            CHECK(coordinate.x == expected[index].x);
            CHECK(coordinate.y == expected[index].y);
        }
    }

    TEST_CASE("Input router keeps Die input out of gray space and owned surfaces") {
        auto fieldResult = Field::create(50, 50);
        REQUIRE(fieldResult);
        auto& field = fieldResult.value();
        REQUIRE(field.setLive({4, 4}, true));
        const auto before = field.cells();
        InputRouter router;
        const auto toolbar = lifeGame::presentation::Toolbar::calculateLayout(1280, 720);
        const auto toolbarPoint = LogicalPoint{toolbar.controls[2].x + 1.0F,
                                               toolbar.controls[2].y + 1.0F};
        const auto fieldPoint = pointAt(field, 4, 4);
        const auto grayPoint = LogicalPoint{0.0F, 0.0F};

        CHECK(router.sample(field, 1280, 720,
                            PointerSample{toolbarPoint, true, true, false}, PaintMode::Die)
                  .paintCommands.empty());
        CHECK(router.sample(field, 1280, 720,
                            PointerSample{fieldPoint, true, true, false}, PaintMode::Die, true)
                  .paintCommands.empty());
        CHECK(router.sample(field, 1280, 720,
                            PointerSample{grayPoint, true, true, false}, PaintMode::Die)
                  .paintCommands.empty());
        CHECK(field.cells() == before);

        const auto accepted = router.sample(
            field, 1280, 720, PointerSample{fieldPoint, true, true, false}, PaintMode::Die);
        REQUIRE(accepted.paintCommands.size() == 1);
        execute(field, accepted);
        CHECK_FALSE(field.isLive({4, 4}));
    }

    TEST_CASE("Input router rejects Die toolbar selection outside the logical viewport") {
        auto fieldResult = Field::create(50, 50);
        REQUIRE(fieldResult);
        auto& field = fieldResult.value();
        InputRouter router;
        const auto layout = lifeGame::presentation::Toolbar::calculateLayout(0, 0);
        const auto dieButton = layout.controls[1];
        const auto diePoint = LogicalPoint{dieButton.x + dieButton.width / 2.0F,
                                           dieButton.y + dieButton.height / 2.0F};

        const auto commands = router.sample(
            field, 0, 0, PointerSample{diePoint, true, true, false}, PaintMode::Live);

        CHECK_FALSE(commands.selectedPaintMode);
        CHECK(commands.paintCommands.empty());
    }

    TEST_CASE("Input router resumes a Die stroke after gray and toolbar ownership") {
        auto fieldResult = Field::create(1280, 720);
        REQUIRE(fieldResult);
        auto& field = fieldResult.value();
        InputRouter router;
        constexpr int VIEWPORT_WIDTH = 1400;
        constexpr int VIEWPORT_HEIGHT = 800;
        const auto plan =
            FieldRenderer::calculateRenderPlan(field, VIEWPORT_WIDTH, VIEWPORT_HEIGHT);
        const auto cellPoint = [&](std::size_t x, std::size_t y) {
            const auto cellSize = static_cast<float>(plan.cellSize);
            return LogicalPoint{plan.fieldRectangle.x + (static_cast<float>(x) + 0.5F) * cellSize,
                                plan.fieldRectangle.y + (static_cast<float>(y) + 0.5F) * cellSize};
        };
        const auto toolbar = lifeGame::presentation::Toolbar::calculateLayout(VIEWPORT_WIDTH,
                                                                                 VIEWPORT_HEIGHT);
        const auto toolbarPoint = LogicalPoint{toolbar.controls[2].x + 1.0F,
                                               toolbar.controls[2].y + 1.0F};

        const auto start = router.sample(field, VIEWPORT_WIDTH, VIEWPORT_HEIGHT,
                                         PointerSample{cellPoint(900, 300), true, true, false},
                                         PaintMode::Die);
        REQUIRE(start.paintCommands.size() == 1);
        execute(field, start);

        CHECK(router.sample(field, VIEWPORT_WIDTH, VIEWPORT_HEIGHT,
                            PointerSample{LogicalPoint{0.0F, 0.0F}, false, true, false},
                            PaintMode::Die)
                  .paintCommands.empty());
        const auto reentry = router.sample(
            field, VIEWPORT_WIDTH, VIEWPORT_HEIGHT,
            PointerSample{cellPoint(904, 300), false, true, false}, PaintMode::Die);
        REQUIRE(reentry.paintCommands.size() == 5);
        execute(field, reentry);

        CHECK(router.sample(field, VIEWPORT_WIDTH, VIEWPORT_HEIGHT,
                            PointerSample{toolbarPoint, false, true, false}, PaintMode::Die)
                  .paintCommands.empty());
        const auto afterToolbar = router.sample(
            field, VIEWPORT_WIDTH, VIEWPORT_HEIGHT,
            PointerSample{cellPoint(910, 300), false, true, false}, PaintMode::Die);
        REQUIRE(afterToolbar.paintCommands.size() == 1);
        execute(field, afterToolbar);

        for (std::size_t x = 900; x <= 904; ++x) {
            CHECK_FALSE(field.isLive({x, 300}));
        }
        CHECK_FALSE(field.isLive({910, 300}));
        CHECK(router.sample(field, VIEWPORT_WIDTH, VIEWPORT_HEIGHT,
                            PointerSample{cellPoint(910, 300), false, false, true},
                            PaintMode::Die)
                  .paintCommands.empty());
    }

    TEST_CASE("Field command executor applies only valid typed Die edits") {
        auto fieldResult = Field::create(4, 3);
        REQUIRE(fieldResult);
        auto& field = fieldResult.value();
        REQUIRE(field.setLive({1, 1}, true));
        REQUIRE(field.setLive({2, 1}, true));
        const auto before = field.cells();

        FieldCommandExecutor::execute(field, PaintDeadCommand{{1, 1}});
        FieldCommandExecutor::execute(field, PaintDeadCommand{{0, 0}});
        FieldCommandExecutor::execute(field, PaintDeadCommand{{4, 0}});
        FieldCommandExecutor::execute(field, PaintDeadCommand{{0, 3}});

        for (std::size_t index = 0; index < field.cells().size(); ++index) {
            const auto expected = index == 1 * field.width() + 1 ? std::uint8_t{0} : before[index];
            CHECK(field.cells()[index] == expected);
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

    TEST_CASE("Input router does not rasterize Die through the toolbar") {
        auto fieldResult = Field::create(1280, 720);
        REQUIRE(fieldResult);
        auto& field = fieldResult.value();
        for (std::size_t x = 900; x < 1280; ++x) {
            REQUIRE(field.setLive({x, 100}, true));
        }
        InputRouter router;

        execute(field, router.sample(field, 1280, 720,
                                     PointerSample{pointAt(field, 900, 100), true, true, false},
                                     PaintMode::Die));
        const auto drag = router.sample(
            field, 1280, 720,
            PointerSample{pointAt(field, 1279, 100), false, true, false}, PaintMode::Die);

        REQUIRE(drag.paintCommands.size() == 76);
        execute(field, drag);

        CHECK_FALSE(field.isLive({959, 100}));
        CHECK(field.isLive({960, 100}));
        CHECK(field.isLive({1263, 100}));
        CHECK_FALSE(field.isLive({1264, 100}));
        CHECK_FALSE(field.isLive({1279, 100}));
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
