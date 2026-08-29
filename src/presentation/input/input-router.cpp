#include <presentation/input/input-router.hpp>

#include <variant>

#include <presentation/input/drag-rasterizer.hpp>
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

        [[nodiscard]] auto isInViewport(LogicalPoint point, int viewportWidth,
                                        int viewportHeight) noexcept -> bool {
            return viewportWidth > 0 && viewportHeight > 0 && point.x >= 0.0F &&
                   point.x < static_cast<float>(viewportWidth) && point.y >= 0.0F &&
                   point.y < static_cast<float>(viewportHeight);
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
                             application::RunState::Running, modalOwnsInput);
    }

    auto InputRouter::sample(const domain::Field& field, int viewportWidth, int viewportHeight,
                             PointerSample pointer, application::PaintMode paintMode,
                             application::RunState runState, bool modalOwnsInput) -> InputCommands {
        return sampleForMode(field, viewportWidth, viewportHeight, pointer, paintMode, runState,
                             modalOwnsInput);
    }

    auto InputRouter::sampleForMode(const domain::Field& field, int viewportWidth,
                                    int viewportHeight, PointerSample pointer,
                                    application::PaintMode paintMode,
                                    application::RunState runState, bool modalOwnsInput)
        -> InputCommands {
        InputCommands commands;

        if (fieldGestureActive_) {
            if (modalOwnsInput || isToolbarOwner(pointer.position, viewportWidth, viewportHeight)) {
                if (!pointer.down || pointer.released) {
                    clearGesture();
                } else {
                    lastCell_.reset();
                }
                return commands;
            }

            if (!pointer.down && !pointer.released) {
                clearGesture();
                return commands;
            }

            const auto cell = CoordinateConverter::toCell(field, pointer.position, viewportWidth,
                                                           viewportHeight);
            if (cell) {
                if (lastCell_) {
                    if (lastCell_->x != cell->x || lastCell_->y != cell->y) {
                        const auto rasterized = DragRasterizer::rasterize(*lastCell_, *cell);
                        commands.paintCommands.reserve(rasterized.size());
                        for (const auto coordinate : rasterized) {
                            const auto cellCenter = CoordinateConverter::toLogicalCellCenter(
                                field, coordinate, viewportWidth, viewportHeight);
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

        if (pointer.pressed && pointer.down &&
            isInViewport(pointer.position, viewportWidth, viewportHeight)) {
            const auto layout = Toolbar::calculateLayout(viewportWidth, viewportHeight);
            if (contains(layout.controls[Toolbar::LIVE_CONTROL_INDEX], pointer.position)) {
                commands.selectedPaintMode = application::PaintMode::Live;
                return commands;
            }
            if (contains(layout.controls[Toolbar::DIE_CONTROL_INDEX], pointer.position)) {
                commands.selectedPaintMode = application::PaintMode::Die;
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
        }

        if (!pointer.pressed || !pointer.down ||
            isToolbarOwner(pointer.position, viewportWidth, viewportHeight)) {
            return commands;
        }

        const auto cell = CoordinateConverter::toCell(field, pointer.position, viewportWidth,
                                                       viewportHeight);
        if (!cell) {
            return commands;
        }

        fieldGestureActive_ = true;
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
        fieldGestureActive_ = false;
        lastCell_.reset();
    }

} // namespace lifeGame::presentation
