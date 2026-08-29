#include <algorithm>
#include <cmath>
#include <cstddef>
#include <limits>

#include <application/commands/field-command.hpp>
#include <catch2/catch_test_macros.hpp>
#include <domain/field/field.hpp>
#include <presentation/camera/camera-controller.hpp>
#include <presentation/camera/coordinate-converter.hpp>
#include <presentation/input/input-router.hpp>
#include <presentation/rendering/field-renderer.hpp>
#include <presentation/ui/toolbar.hpp>

namespace {

    using lifeGame::application::PaintMode;
    using lifeGame::application::RunState;
    using lifeGame::domain::Field;
    using lifeGame::presentation::CameraController;
    using lifeGame::presentation::CameraState;
    using lifeGame::presentation::CoordinateConverter;
    using lifeGame::presentation::FieldRenderer;
    using lifeGame::presentation::InputRouter;
    using lifeGame::presentation::LogicalPoint;
    using lifeGame::presentation::PointerSample;
    using lifeGame::presentation::Toolbar;
    using lifeGame::presentation::ZoomLevel;

    auto centerOf(const lifeGame::presentation::FieldRenderPlan& plan, std::size_t x,
                  std::size_t y) -> LogicalPoint {
        const auto cellSize = static_cast<float>(plan.cellSize);
        return LogicalPoint{plan.fieldRectangle.x + (static_cast<float>(x) + 0.5F) * cellSize,
                            plan.fieldRectangle.y + (static_cast<float>(y) + 0.5F) * cellSize};
    }

    TEST_CASE("Camera clamps rectangular oversized fields to retain an in-field intersection") {
        const auto field = Field::create(2400, 3);
        REQUIRE(field);
        CameraController camera;

        camera.pan(field.value(), 1280, 720, 10'000.0F, 10'000.0F);
        const auto plan = FieldRenderer::calculateRenderPlan(field.value(), 1280, 720,
                                                               camera.state());

        CHECK(plan.visibleCells.columnCount > 0);
        CHECK(plan.visibleCells.rowCount > 0);
        CHECK(plan.fieldRectangle.x < 1280.0F);
        CHECK(plan.fieldRectangle.x + plan.fieldRectangle.width > 0.0F);
        CHECK(plan.fieldRectangle.y < 720.0F);
        CHECK(plan.fieldRectangle.y + plan.fieldRectangle.height > 0.0F);
        CHECK(std::isfinite(camera.state().x));
        CHECK(std::isfinite(camera.state().y));
        CHECK(camera.state() == plan.camera);

        camera.reset();
        camera.pan(field.value(), 1280, 720, -10'000.0F, -10'000.0F);
        const auto oppositePlan = FieldRenderer::calculateRenderPlan(
            field.value(), 1280, 720, camera.state());
        CHECK(oppositePlan.visibleCells.columnCount > 0);
        CHECK(oppositePlan.visibleCells.rowCount > 0);
        CHECK(oppositePlan.fieldRectangle.x < 1280.0F);
        CHECK(oppositePlan.fieldRectangle.x + oppositePlan.fieldRectangle.width > 0.0F);
        CHECK(oppositePlan.fieldRectangle.y < 720.0F);
        CHECK(oppositePlan.fieldRectangle.y + oppositePlan.fieldRectangle.height > 0.0F);
        CHECK(std::isfinite(camera.state().x));
        CHECK(std::isfinite(camera.state().y));
        CHECK(camera.state() == oppositePlan.camera);
    }

    TEST_CASE("Camera snapping retains a pixel for thin fields at a supported viewport") {
        const auto field = Field::create(1, 1);
        REQUIRE(field);
        CameraController camera;

        camera.pan(field.value(), 960, 570, 10'000.0F, 10'000.0F);
        const auto plan = FieldRenderer::calculateRenderPlan(field.value(), 960, 570,
                                                               camera.state());

        CHECK(plan.visibleCells.columnCount > 0);
        CHECK(plan.visibleCells.rowCount > 0);
        CHECK(plan.fieldRectangle.x < 960.0F);
        CHECK(plan.fieldRectangle.x + plan.fieldRectangle.width > 0.0F);
        CHECK(plan.fieldRectangle.y < 570.0F);
        CHECK(plan.fieldRectangle.y + plan.fieldRectangle.height > 0.0F);
    }

    TEST_CASE("Camera rejects non-finite pan input without changing its bounded state") {
        const auto field = Field::create(100, 100);
        REQUIRE(field);
        CameraController camera;
        camera.pan(field.value(), 1280, 720, 4.0F, -3.0F);
        const auto before = camera.state();

        camera.pan(field.value(), 1280, 720, std::numeric_limits<float>::quiet_NaN(), 0.0F);
        CHECK(camera.state() == before);
        camera.pan(field.value(), 1280, 720, std::numeric_limits<float>::infinity(), 0.0F);
        CHECK(camera.state() == before);
    }

    TEST_CASE("Camera changes through the approved discrete zoom levels and clamps endpoints") {
        const auto field = Field::create(100, 100);
        REQUIRE(field);
        CameraController camera;

        CHECK(camera.state().zoomLevel == ZoomLevel::Percent100);
        camera.zoom(field.value(), 1280, 720, true, 640.0F, 360.0F);
        CHECK(camera.state().zoomLevel == ZoomLevel::Percent150);
        camera.zoom(field.value(), 1280, 720, true, 640.0F, 360.0F);
        CHECK(camera.state().zoomLevel == ZoomLevel::Percent200);
        camera.zoom(field.value(), 1280, 720, true, 640.0F, 360.0F);
        CHECK(camera.state().zoomLevel == ZoomLevel::Percent300);
        camera.zoom(field.value(), 1280, 720, true, 640.0F, 360.0F);
        CHECK(camera.state().zoomLevel == ZoomLevel::Percent400);

        const auto atMaximum = camera.state();
        camera.zoom(field.value(), 1280, 720, true, 640.0F, 360.0F);
        CHECK(camera.state() == atMaximum);

        camera.zoom(field.value(), 1280, 720, false, 640.0F, 360.0F);
        CHECK(camera.state().zoomLevel == ZoomLevel::Percent300);
        camera.zoom(field.value(), 1280, 720, false, 640.0F, 360.0F);
        camera.zoom(field.value(), 1280, 720, false, 640.0F, 360.0F);
        camera.zoom(field.value(), 1280, 720, false, 640.0F, 360.0F);
        camera.zoom(field.value(), 1280, 720, false, 640.0F, 360.0F);
        camera.zoom(field.value(), 1280, 720, false, 640.0F, 360.0F);
        CHECK(camera.state().zoomLevel == ZoomLevel::Percent50);

        const auto atMinimum = camera.state();
        camera.zoom(field.value(), 1280, 720, false, 640.0F, 360.0F);
        CHECK(camera.state() == atMinimum);
    }

    TEST_CASE("Zoom scales the fit-to-viewport cell extent for every approved level") {
        const auto field = Field::create(50, 50);
        REQUIRE(field);
        const auto base = FieldRenderer::calculateRenderPlan(field.value(), 1280, 720);
        REQUIRE(base.cellSize == 14.0F);

        CHECK(FieldRenderer::calculateRenderPlan(
                  field.value(), 1280, 720, CameraState{0.0F, 0.0F, ZoomLevel::Percent50})
                  .cellSize == 7.0F);
        CHECK(FieldRenderer::calculateRenderPlan(
                  field.value(), 1280, 720, CameraState{0.0F, 0.0F, ZoomLevel::Percent75})
                  .cellSize == 10.5F);
        CHECK(FieldRenderer::calculateRenderPlan(
                  field.value(), 1280, 720, CameraState{0.0F, 0.0F, ZoomLevel::Percent100})
                  .cellSize == 14.0F);
        CHECK(FieldRenderer::calculateRenderPlan(
                  field.value(), 1280, 720, CameraState{0.0F, 0.0F, ZoomLevel::Percent150})
                  .cellSize == 21.0F);
        CHECK(FieldRenderer::calculateRenderPlan(
                  field.value(), 1280, 720, CameraState{0.0F, 0.0F, ZoomLevel::Percent200})
                  .cellSize == 28.0F);
        CHECK(FieldRenderer::calculateRenderPlan(
                  field.value(), 1280, 720, CameraState{0.0F, 0.0F, ZoomLevel::Percent300})
                  .cellSize == 42.0F);
        CHECK(FieldRenderer::calculateRenderPlan(
                  field.value(), 1280, 720, CameraState{0.0F, 0.0F, ZoomLevel::Percent400})
                  .cellSize == 56.0F);

        const auto invalidLevel = static_cast<ZoomLevel>(999);
        const auto normalized = FieldRenderer::calculateRenderPlan(
            field.value(), 1280, 720, CameraState{0.0F, 0.0F, invalidLevel});
        CHECK(normalized.camera.zoomLevel == ZoomLevel::Percent100);
        CHECK(normalized.cellSize == 14.0F);
    }

    TEST_CASE("Zoom preserves fractional scale for fields fitted to one display pixel") {
        const auto field = Field::create(2400, 3);
        REQUIRE(field);

        CHECK(FieldRenderer::calculateRenderPlan(
                  field.value(), 1280, 720, CameraState{0.0F, 0.0F, ZoomLevel::Percent50})
                  .cellSize == 0.5F);
        CHECK(FieldRenderer::calculateRenderPlan(
                  field.value(), 1280, 720, CameraState{0.0F, 0.0F, ZoomLevel::Percent75})
                  .cellSize == 0.75F);
        CHECK(FieldRenderer::calculateRenderPlan(
                  field.value(), 1280, 720, CameraState{0.0F, 0.0F, ZoomLevel::Percent100})
                  .cellSize == 1.0F);
        CHECK(FieldRenderer::calculateRenderPlan(
                  field.value(), 1280, 720, CameraState{0.0F, 0.0F, ZoomLevel::Percent150})
                  .cellSize == 1.5F);
        CHECK(FieldRenderer::calculateRenderPlan(
                  field.value(), 1280, 720, CameraState{0.0F, 0.0F, ZoomLevel::Percent200})
                  .cellSize == 2.0F);
        CHECK(FieldRenderer::calculateRenderPlan(
                  field.value(), 1280, 720, CameraState{0.0F, 0.0F, ZoomLevel::Percent300})
                  .cellSize == 3.0F);
        CHECK(FieldRenderer::calculateRenderPlan(
                  field.value(), 1280, 720, CameraState{0.0F, 0.0F, ZoomLevel::Percent400})
                  .cellSize == 4.0F);
    }

    TEST_CASE("Zoom keeps an in-field pointer cell anchored") {
        const auto field = Field::create(100, 100);
        REQUIRE(field);
        CameraController camera;
        const auto before = FieldRenderer::calculateRenderPlan(field.value(), 1280, 720);
        const auto point = centerOf(before, 20, 20);
        const auto beforeCell = CoordinateConverter::toCell(field.value(), point, 1280, 720,
                                                             camera.state());
        REQUIRE(beforeCell);

        camera.zoom(field.value(), 1280, 720, true, point.x, point.y);
        const auto after = FieldRenderer::calculateRenderPlan(field.value(), 1280, 720,
                                                               camera.state());
        const auto afterCell = CoordinateConverter::toCell(field.value(), point, 1280, 720,
                                                            camera.state());

        REQUIRE(afterCell);
        CHECK(afterCell->x == beforeCell->x);
        CHECK(afterCell->y == beforeCell->y);
        CHECK(std::abs(after.fieldRectangle.x +
                       (static_cast<float>(beforeCell->x) + 0.5F) * after.cellSize - point.x) <=
              after.cellSize / 2.0F);
        CHECK(std::abs(after.fieldRectangle.y +
                       (static_cast<float>(beforeCell->y) + 0.5F) * after.cellSize - point.y) <=
              after.cellSize / 2.0F);

        camera.zoom(field.value(), 1280, 720, false, point.x, point.y);
        const auto returnedCell = CoordinateConverter::toCell(field.value(), point, 1280, 720,
                                                                camera.state());
        REQUIRE(returnedCell);
        CHECK(returnedCell->x == beforeCell->x);
        CHECK(returnedCell->y == beforeCell->y);
    }

    TEST_CASE("Zoom preserves an edge cell when finite bounds do not require clamping") {
        const auto field = Field::create(100, 100);
        REQUIRE(field);
        CameraController camera;
        camera.pan(field.value(), 1280, 720, 10.0F, 0.0F);
        const auto before = FieldRenderer::calculateRenderPlan(field.value(), 1280, 720,
                                                                 camera.state());
        const auto x = before.visibleCells.firstColumn + 1;
        const auto y = before.visibleCells.firstRow + 1;
        REQUIRE(x < field.value().width());
        REQUIRE(y < field.value().height());
        const auto point = centerOf(before, x, y);
        REQUIRE(point.x >= 0.0F);
        REQUIRE(point.x < 1280.0F);
        REQUIRE(point.y >= 0.0F);
        REQUIRE(point.y < 720.0F);

        const auto beforeCell = CoordinateConverter::toCell(field.value(), point, 1280, 720,
                                                             camera.state());
        REQUIRE(beforeCell);
        camera.zoom(field.value(), 1280, 720, true, point.x, point.y);
        const auto afterCell = CoordinateConverter::toCell(field.value(), point, 1280, 720,
                                                            camera.state());

        REQUIRE(afterCell);
        CHECK(afterCell->x == beforeCell->x);
        CHECK(afterCell->y == beforeCell->y);
    }

    TEST_CASE("Zoom uses the viewport center when the pointer is in gray space") {
        const auto field = Field::create(100, 100);
        REQUIRE(field);
        CameraController camera;
        camera.pan(field.value(), 1280, 720, -20.0F, 0.0F);
        const auto center = LogicalPoint{640.0F, 360.0F};
        const auto beforeCenter = CoordinateConverter::toCell(field.value(), center, 1280, 720,
                                                               camera.state());
        REQUIRE(beforeCenter);
        CHECK_FALSE(CoordinateConverter::toCell(field.value(), LogicalPoint{1200.0F, 360.0F},
                                                 1280, 720, camera.state()));

        camera.zoom(field.value(), 1280, 720, true, 1200.0F, 360.0F);
        const auto afterCenter = CoordinateConverter::toCell(field.value(), center, 1280, 720,
                                                              camera.state());
        REQUIRE(afterCenter);
        CHECK(afterCenter->x == beforeCenter->x);
        CHECK(afterCenter->y == beforeCenter->y);
    }

    TEST_CASE("Zoom ignores snapped field padding when it contains no logical cell") {
        const auto field = Field::create(2400, 1);
        REQUIRE(field);
        CameraController pointerCamera;
        CameraController centerCamera;
        pointerCamera.zoom(field.value(), 1280, 720, false, 640.0F, 360.0F);
        pointerCamera.zoom(field.value(), 1280, 720, false, 640.0F, 360.0F);
        centerCamera.zoom(field.value(), 1280, 720, false, 640.0F, 360.0F);
        centerCamera.zoom(field.value(), 1280, 720, false, 640.0F, 360.0F);

        const auto plan = FieldRenderer::calculateRenderPlan(field.value(), 1280, 720,
                                                               pointerCamera.state());
        CHECK(pointerCamera.state().zoomLevel == ZoomLevel::Percent50);
        CHECK(plan.cellSize == 0.5F);
        CHECK(plan.fieldRectangle.height == 1.0F);
        const auto paddingPoint = LogicalPoint{plan.fieldRectangle.x + 0.25F,
                                               plan.fieldRectangle.y + 0.75F};
        REQUIRE_FALSE(CoordinateConverter::toCell(field.value(), paddingPoint, 1280, 720,
                                                   pointerCamera.state()));

        pointerCamera.zoom(field.value(), 1280, 720, true, paddingPoint.x, paddingPoint.y);
        centerCamera.zoom(field.value(), 1280, 720, true, 640.0F, 360.0F);
        CHECK(pointerCamera.state() == centerCamera.state());
    }

    TEST_CASE("Zoomed camera extremes retain an in-field intersection") {
        const auto field = Field::create(2400, 3);
        REQUIRE(field);
        CameraController camera;
        for (int index = 0; index < 4; ++index) {
            camera.zoom(field.value(), 1280, 720, true, 640.0F, 360.0F);
        }
        camera.pan(field.value(), 1280, 720, 100'000.0F, 100'000.0F);
        const auto plan = FieldRenderer::calculateRenderPlan(field.value(), 1280, 720,
                                                               camera.state());

        CHECK(plan.camera.zoomLevel == ZoomLevel::Percent400);
        CHECK(plan.visibleCells.columnCount > 0);
        CHECK(plan.visibleCells.rowCount > 0);
        CHECK(plan.fieldRectangle.x < 1280.0F);
        CHECK(plan.fieldRectangle.x + plan.fieldRectangle.width > 0.0F);
        CHECK(plan.fieldRectangle.y < 720.0F);
        CHECK(plan.fieldRectangle.y + plan.fieldRectangle.height > 0.0F);
    }

    TEST_CASE("Minimum zoom keeps a one-row field visible after camera clamping") {
        const auto field = Field::create(2400, 1);
        REQUIRE(field);
        const auto plan = FieldRenderer::calculateRenderPlan(
            field.value(), 1280, 720,
            CameraState{100'000.0F, 100'000.0F, ZoomLevel::Percent50});

        CHECK(plan.camera.zoomLevel == ZoomLevel::Percent50);
        CHECK(plan.visibleCells.columnCount > 0);
        CHECK(plan.visibleCells.rowCount > 0);
        CHECK(plan.fieldRectangle.width > 0.0F);
        CHECK(plan.fieldRectangle.height > 0.0F);
    }

    TEST_CASE("Camera render geometry is shared by cell mapping and center projection") {
        const auto field = Field::create(100, 40);
        REQUIRE(field);
        CameraController camera;
        camera.pan(field.value(), 1280, 720, 8.3F, -2.2F);
        const auto plan = FieldRenderer::calculateRenderPlan(field.value(), 1280, 720,
                                                               camera.state());
        REQUIRE(plan.visibleCells.columnCount > 0);
        REQUIRE(plan.visibleCells.rowCount > 0);

        const auto unsnappedX = 32.0F - plan.camera.x * static_cast<float>(plan.cellSize);
        const auto unsnappedY = 120.0F - plan.camera.y * static_cast<float>(plan.cellSize);
        CHECK(plan.fieldRectangle.x == std::floor(unsnappedX));
        CHECK(plan.fieldRectangle.y == std::floor(unsnappedY));

        const auto x = std::min(plan.visibleCells.firstColumn + 1,
                                field.value().width() - 1);
        const auto y = plan.visibleCells.firstRow;
        const auto center = centerOf(plan, x, y);
        const auto mapped = CoordinateConverter::toCell(field.value(), center, 1280, 720,
                                                         camera.state());

        REQUIRE(mapped);
        CHECK(mapped->x == x);
        CHECK(mapped->y == y);

        const auto projected = CoordinateConverter::toLogicalCellCenter(
            field.value(), {x, y}, 1280, 720, camera.state());
        REQUIRE(projected);
        CHECK(projected->x == center.x);
        CHECK(projected->y == center.y);

        const auto rightBoundary = LogicalPoint{
            plan.fieldRectangle.x + (static_cast<float>(x) + 1.0F) * plan.cellSize, center.y};
        const auto nextCell = CoordinateConverter::toCell(field.value(), rightBoundary, 1280, 720,
                                                           camera.state());
        if (x + 1 < field.value().width()) {
            REQUIRE(nextCell);
            CHECK(nextCell->x == x + 1);
        }
    }

    TEST_CASE("Move input emits camera pan only and preserves field bytes") {
        auto fieldResult = Field::create(100, 100);
        REQUIRE(fieldResult);
        auto& field = fieldResult.value();
        REQUIRE(field.setLive({3, 4}, true));
        const auto before = field.cells();
        InputRouter router;

        const CameraState camera{4.25F, -1.75F, ZoomLevel::Percent200};
        const auto cameraPlan = FieldRenderer::calculateRenderPlan(field, 1280, 720, camera);
        const auto cameraStart = centerOf(cameraPlan, 20, 20);
        const auto cameraFinish = LogicalPoint{
            cameraStart.x + static_cast<float>(cameraPlan.cellSize) * 2.0F,
            cameraStart.y + static_cast<float>(cameraPlan.cellSize),
        };

        const auto press = router.sample(field, 1280, 720,
                                         PointerSample{cameraStart, true, true, false}, PaintMode::Move,
                                         RunState::Running, camera);
        CHECK(press.paintCommands.empty());
        CHECK(press.panCommands.empty());

        const auto drag = router.sample(field, 1280, 720,
                                        PointerSample{cameraFinish, false, true, false}, PaintMode::Move,
                                        RunState::Running, camera);
        CHECK(drag.paintCommands.empty());
        REQUIRE(drag.panCommands.size() == 1);
        CHECK(drag.panCommands.front().deltaX == -2.0F);
        CHECK(drag.panCommands.front().deltaY == -1.0F);
        CHECK(field.cells() == before);
    }

    TEST_CASE("Move drag stays captured through gray space and reverses a camera boundary") {
        const auto field = Field::create(2400, 3);
        REQUIRE(field);
        CameraController camera;
        camera.pan(field.value(), 1280, 720, 10'000.0F, 0.0F);
        const auto plan = FieldRenderer::calculateRenderPlan(field.value(), 1280, 720,
                                                               camera.state());
        REQUIRE(plan.visibleCells.columnCount > 0);
        REQUIRE(plan.visibleCells.rowCount > 0);

        const auto inField =
            centerOf(plan, plan.visibleCells.firstColumn, plan.visibleCells.firstRow);
        const auto gray = LogicalPoint{100.0F, inField.y};
        CHECK_FALSE(CoordinateConverter::toCell(field.value(), gray, 1280, 720, camera.state()));

        InputRouter router;
        CHECK(router.sample(field.value(), 1280, 720,
                            PointerSample{gray, true, true, false}, PaintMode::Move,
                            RunState::Running, camera.state())
                  .panCommands.empty());
        CHECK(router.sample(field.value(), 1280, 720,
                            PointerSample{inField, false, true, false}, PaintMode::Move,
                            RunState::Running, camera.state())
                  .panCommands.empty());

        router.reset();
        static_cast<void>(router.sample(field.value(), 1280, 720,
                                        PointerSample{inField, true, true, false}, PaintMode::Move,
                                        RunState::Running, camera.state()));
        const auto before = camera.state();
        const auto drag = router.sample(
            field.value(), 1280, 720, PointerSample{gray, false, true, false}, PaintMode::Move,
            RunState::Running, camera.state());

        REQUIRE(drag.panCommands.size() == 1);
        CHECK(drag.panCommands.front().deltaX < 0.0F);
        CHECK(drag.panCommands.front().deltaY == 0.0F);
        camera.pan(field.value(), 1280, 720, drag.panCommands.front().deltaX,
                   drag.panCommands.front().deltaY);
        CHECK(camera.state().x < before.x);
    }

    TEST_CASE("A Move transition ends a captured paint gesture without painting") {
        const auto field = Field::create(100, 100);
        REQUIRE(field);
        InputRouter router;
        const auto plan = FieldRenderer::calculateRenderPlan(field.value(), 1280, 720);
        const auto start = centerOf(plan, 20, 20);
        const auto finish = LogicalPoint{start.x + static_cast<float>(plan.cellSize) * 2.0F,
                                         start.y + static_cast<float>(plan.cellSize)};

        const auto press = router.sample(field.value(), 1280, 720,
                                         PointerSample{start, true, true, false}, PaintMode::Live,
                                         RunState::Running, {});
        REQUIRE(press.paintCommands.size() == 1);

        const auto transition = router.sample(
            field.value(), 1280, 720, PointerSample{finish, false, true, false}, PaintMode::Move,
            RunState::Running, {});
        CHECK(transition.paintCommands.empty());
        CHECK(transition.panCommands.empty());
    }

    TEST_CASE("Move capture respects modal, toolbar, gray-space, and release ownership") {
        auto fieldResult = Field::create(100, 100);
        REQUIRE(fieldResult);
        auto& field = fieldResult.value();
        const auto plan = FieldRenderer::calculateRenderPlan(field, 1280, 720);
        const auto start = centerOf(plan, 20, 20);
        const auto toolbar = Toolbar::calculateLayout(1280, 720);
        const auto toolbarPoint = LogicalPoint{toolbar.panel.x + 1.0F, toolbar.panel.y + 1.0F};
        InputRouter router;

        static_cast<void>(router.sample(field, 1280, 720,
                                        PointerSample{start, true, true, false}, PaintMode::Move,
                                        RunState::Running, {}));
        CHECK(router.sample(field, 1280, 720,
                            PointerSample{toolbarPoint, false, true, false}, PaintMode::Move,
                            RunState::Running, {})
                  .panCommands.empty());
        CHECK(router.sample(field, 1280, 720,
                            PointerSample{start, false, true, false}, PaintMode::Move,
                                          RunState::Running, true)
                  .panCommands.empty());
        CHECK(router.sample(field, 1280, 720,
                            PointerSample{LogicalPoint{0.0F, 0.0F}, false, true, false},
                            PaintMode::Move, RunState::Running, {})
                  .panCommands.empty());
        CHECK(router.sample(field, 1280, 720,
                            PointerSample{LogicalPoint{0.0F, 0.0F}, false, false, true},
                            PaintMode::Move, RunState::Running, {})
                  .panCommands.empty());

        const auto nextPress = router.sample(
            field, 1280, 720, PointerSample{start, true, true, false}, PaintMode::Move,
            RunState::Running, {});
        CHECK(nextPress.paintCommands.empty());
        CHECK(nextPress.panCommands.empty());
    }

    TEST_CASE("Move capture honors toolbar ownership over in-field coordinates") {
        const auto field = Field::create(1280, 720);
        REQUIRE(field);
        const auto plan = FieldRenderer::calculateRenderPlan(field.value(), 1280, 720);
        const auto start = centerOf(plan, 100, 100);
        const auto toolbar = Toolbar::calculateLayout(1280, 720);
        const auto toolbarPoint = LogicalPoint{toolbar.panel.x + 1.0F, toolbar.panel.y + 1.0F};
        InputRouter router;

        static_cast<void>(router.sample(field.value(), 1280, 720,
                                        PointerSample{start, true, true, false}, PaintMode::Move,
                                        RunState::Running, {}));
        const auto drag = router.sample(
            field.value(), 1280, 720, PointerSample{toolbarPoint, false, true, false},
            PaintMode::Move, RunState::Running, {});

        CHECK(drag.paintCommands.empty());
        CHECK(drag.panCommands.empty());
    }

    TEST_CASE("Toolbar identifies Move as the persistent active field mode") {
        const auto plan = Toolbar::calculateRenderPlan(PaintMode::Move, RunState::Running);
        REQUIRE(Toolbar::MOVE_CONTROL_INDEX < plan.controls.size());
        CHECK(plan.controls[Toolbar::MOVE_CONTROL_INDEX].style ==
              lifeGame::presentation::ToolbarButtonStyle::Active);
        CHECK(plan.controls[Toolbar::MOVE_CONTROL_INDEX].label == "Move");
        CHECK(plan.statusLabel == "Active: Move | Running");
    }

    TEST_CASE("Move toolbar selection crosses the typed application boundary") {
        auto fieldResult = Field::create(50, 50);
        REQUIRE(fieldResult);
        const auto layout = Toolbar::calculateLayout(1280, 720);
        const auto move = layout.controls[Toolbar::MOVE_CONTROL_INDEX];
        const auto point = LogicalPoint{move.x + move.width / 2.0F,
                                        move.y + move.height / 2.0F};
        InputRouter router;

        const auto commands = router.sample(
            fieldResult.value(), 1280, 720, PointerSample{point, true, true, false},
            PaintMode::Live, RunState::Running, {});

        REQUIRE(commands.selectedPaintMode);
        CHECK(*commands.selectedPaintMode == PaintMode::Move);
        CHECK(commands.paintCommands.empty());
        CHECK(commands.panCommands.empty());
    }

} // namespace
