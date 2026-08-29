#include <presentation/input/input-router.hpp>

#include <algorithm>
#include <cmath>
#include <variant>

#include <presentation/input/drag-rasterizer.hpp>
#include <presentation/rendering/field-renderer.hpp>
#include <presentation/ui/toolbar.hpp>

namespace lifeGame::presentation {

    namespace {

        [[nodiscard]] auto contains(const Rectangle& rectangle, LogicalPoint point) noexcept
            -> bool {
            return point.x >= rectangle.x && point.x < rectangle.x + rectangle.width &&
                   point.y >= rectangle.y && point.y < rectangle.y + rectangle.height;
        }

        void appendPaintCommand(InputCommands& commands, application::PaintMode paintMode,
                                domain::CellCoordinate coordinate) {
            if (paintMode == application::PaintMode::Die) {
                commands.paintCommands.emplace_back(application::PaintDeadCommand{coordinate});
                return;
            }

            commands.paintCommands.emplace_back(application::PaintLiveCommand{coordinate});
        }

        void appendSelectedMode(InputCommands& commands, application::PaintMode mode) {
            commands.selectedPaintMode = mode;
        }

        [[nodiscard]] auto isInViewport(LogicalPoint point, int viewportWidth,
                                        int viewportHeight) noexcept -> bool {
            return viewportWidth > 0 && viewportHeight > 0 && point.x >= 0.0F &&
                   point.x < static_cast<float>(viewportWidth) && point.y >= 0.0F &&
                   point.y < static_cast<float>(viewportHeight);
        }

        [[nodiscard]] auto viewportCenter(int viewportWidth, int viewportHeight) noexcept
            -> LogicalPoint {
            return LogicalPoint{static_cast<float>(std::max(viewportWidth, 1)) / 2.0F,
                                static_cast<float>(std::max(viewportHeight, 1)) / 2.0F};
        }

    } // namespace

    auto InputRouter::sample(const domain::Field& field, int viewportWidth, int viewportHeight,
                             PointerSample pointer, bool modalOwnsInput)
        -> std::vector<application::PaintLiveCommand> {
        const auto sampled = sample(field, viewportWidth, viewportHeight, pointer,
                                    application::PaintMode::Live, modalOwnsInput);
        std::vector<application::PaintLiveCommand> commands;
        commands.reserve(sampled.paintCommands.size());
        for (const auto& command : sampled.paintCommands) {
            if (const auto* liveCommand = std::get_if<application::PaintLiveCommand>(&command)) {
                commands.push_back(*liveCommand);
            }
        }
        return commands;
    }

    auto InputRouter::sample(const domain::Field& field, int viewportWidth, int viewportHeight,
                             PointerSample pointer, application::PaintMode paintMode,
                             bool modalOwnsInput) -> InputCommands {
        return sampleForMode(field, viewportWidth, viewportHeight, pointer, paintMode,
                             application::RunState::Running, CameraController::defaultState(),
                             modalOwnsInput);
    }

    auto InputRouter::sample(const domain::Field& field, int viewportWidth, int viewportHeight,
                             PointerSample pointer, application::PaintMode paintMode,
                             application::RunState runState, bool modalOwnsInput) -> InputCommands {
        return sampleForMode(field, viewportWidth, viewportHeight, pointer, paintMode, runState,
                             CameraController::defaultState(), modalOwnsInput);
    }

    auto InputRouter::sample(const domain::Field& field, int viewportWidth, int viewportHeight,
                             PointerSample pointer, application::PaintMode paintMode,
                             application::RunState runState, CameraState camera,
                             bool modalOwnsInput) -> InputCommands {
        return sampleForMode(field, viewportWidth, viewportHeight, pointer, paintMode, runState,
                             camera, modalOwnsInput);
    }

    void InputRouter::reset() noexcept { clearGesture(); }

    auto InputRouter::sampleForMode(const domain::Field& field, int viewportWidth,
                                    int viewportHeight, PointerSample pointer,
                                    application::PaintMode paintMode,
                                    application::RunState runState, CameraState camera,
                                    bool modalOwnsInput)
        -> InputCommands {
        InputCommands commands;

        if (!modalOwnsInput && pointer.pressed && pointer.down &&
            isInViewport(pointer.position, viewportWidth, viewportHeight) &&
            isToolbarOwner(pointer.position, viewportWidth, viewportHeight)) {
            clearGesture();
            const auto layout = Toolbar::calculateLayout(viewportWidth, viewportHeight);
            if (contains(layout.controls[Toolbar::LIVE_CONTROL_INDEX], pointer.position)) {
                appendSelectedMode(commands, application::PaintMode::Live);
                return commands;
            }
            if (contains(layout.controls[Toolbar::DIE_CONTROL_INDEX], pointer.position)) {
                appendSelectedMode(commands, application::PaintMode::Die);
                return commands;
            }
            if (contains(layout.controls[Toolbar::MOVE_CONTROL_INDEX], pointer.position)) {
                appendSelectedMode(commands, application::PaintMode::Move);
                return commands;
            }
            if (contains(layout.controls[Toolbar::ZOOM_IN_CONTROL_INDEX], pointer.position)) {
                const auto center = viewportCenter(viewportWidth, viewportHeight);
                commands.zoomRequest = application::ZoomCameraCommand{
                    application::ZoomDirection::In, center.x, center.y};
                return commands;
            }
            if (contains(layout.controls[Toolbar::ZOOM_OUT_CONTROL_INDEX], pointer.position)) {
                const auto center = viewportCenter(viewportWidth, viewportHeight);
                commands.zoomRequest = application::ZoomCameraCommand{
                    application::ZoomDirection::Out, center.x, center.y};
                return commands;
            }
            if (contains(layout.controls[Toolbar::RUN_CONTROL_INDEX], pointer.position)) {
                if (runState == application::RunState::Paused) {
                    commands.resumeRequest = application::ResumeCommand{};
                } else {
                    commands.pauseRequest = application::PauseCommand{};
                }
                return commands;
            }
            if (contains(layout.controls[Toolbar::EXIT_CONTROL_INDEX], pointer.position)) {
                commands.exitRequest = application::ExitSessionCommand{};
                return commands;
            }
            return commands;
        }

        if (gestureKind_ != GestureKind::None) {
            if (modalOwnsInput || isToolbarOwner(pointer.position, viewportWidth, viewportHeight)) {
                lastCell_.reset();
                lastPointer_.reset();
                if (!pointer.down || pointer.released) {
                    clearGesture();
                }
                return commands;
            }

            if (gestureKind_ == GestureKind::Paint && paintMode == application::PaintMode::Move) {
                clearGesture();
                return commands;
            }

            if (!pointer.down && !pointer.released) {
                clearGesture();
                return commands;
            }

            if (gestureKind_ == GestureKind::Move) {
                if (!isInViewport(pointer.position, viewportWidth, viewportHeight)) {
                    lastPointer_.reset();
                    if (pointer.released) {
                        clearGesture();
                    }
                    return commands;
                }

                if (lastPointer_) {
                    const auto deltaX = pointer.position.x - lastPointer_->x;
                    const auto deltaY = pointer.position.y - lastPointer_->y;
                    const auto plan = FieldRenderer::calculateRenderPlan(
                        field, viewportWidth, viewportHeight, camera);
                    const auto cellSize = static_cast<float>(plan.cellSize);
                    if (std::isfinite(deltaX) && std::isfinite(deltaY) &&
                        (deltaX != 0.0F || deltaY != 0.0F)) {
                        commands.panCommands.emplace_back(
                            application::PanCameraCommand{-deltaX / cellSize, -deltaY / cellSize});
                    }
                }
                lastPointer_ = pointer.position;
                if (pointer.released) {
                    clearGesture();
                }
                return commands;
            }

            const auto cell = CoordinateConverter::toCell(field, pointer.position, viewportWidth,
                                                           viewportHeight, camera);
            if (cell) {
                if (lastCell_) {
                    if (lastCell_->x != cell->x || lastCell_->y != cell->y) {
                        const auto rasterized = DragRasterizer::rasterize(*lastCell_, *cell);
                        commands.paintCommands.reserve(rasterized.size());
                        for (const auto coordinate : rasterized) {
                            const auto cellCenter = CoordinateConverter::toLogicalCellCenter(
                                field, coordinate, viewportWidth, viewportHeight, camera);
                            if (!cellCenter ||
                                isToolbarOwner(*cellCenter, viewportWidth, viewportHeight)) {
                                continue;
                            }
                            appendPaintCommand(commands, paintMode, coordinate);
                        }
                    }
                } else {
                    appendPaintCommand(commands, paintMode, *cell);
                }
                lastCell_ = cell;
            }

            if (pointer.released) {
                clearGesture();
            }
            return commands;
        }

        if (modalOwnsInput) {
            return commands;
        }

        if (!pointer.pressed || !pointer.down ||
            isToolbarOwner(pointer.position, viewportWidth, viewportHeight)) {
            return commands;
        }

        const auto cell = CoordinateConverter::toCell(field, pointer.position, viewportWidth,
                                                       viewportHeight, camera);
        if (!cell) {
            return commands;
        }

        if (paintMode == application::PaintMode::Move) {
            gestureKind_ = GestureKind::Move;
            lastPointer_ = pointer.position;
            if (pointer.released) {
                clearGesture();
            }
            return commands;
        }

        gestureKind_ = GestureKind::Paint;
        lastCell_ = cell;
        appendPaintCommand(commands, paintMode, *cell);
        if (pointer.released) {
            clearGesture();
        }
        return commands;
    }

    bool InputRouter::isToolbarOwner(LogicalPoint point, int viewportWidth,
                                     int viewportHeight) const noexcept {
        if (viewportWidth <= 0 || viewportHeight <= 0) {
            return false;
        }

        return contains(Toolbar::calculateLayout(viewportWidth, viewportHeight).panel, point);
    }

    void InputRouter::clearGesture() noexcept {
        gestureKind_ = GestureKind::None;
        lastCell_.reset();
        lastPointer_.reset();
    }

} // namespace lifeGame::presentation
