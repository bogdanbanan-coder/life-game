#include <presentation/input/input-router.hpp>

#include <presentation/input/drag-rasterizer.hpp>
#include <presentation/ui/toolbar.hpp>

namespace lifeGame::presentation {

    namespace {

        [[nodiscard]] auto contains(const Rectangle& rectangle, LogicalPoint point) noexcept
            -> bool {
            return point.x >= rectangle.x && point.x < rectangle.x + rectangle.width &&
                   point.y >= rectangle.y && point.y < rectangle.y + rectangle.height;
        }

    } // namespace

    auto InputRouter::sample(const domain::Field& field, int viewportWidth, int viewportHeight,
                             PointerSample pointer, bool modalOwnsInput)
        -> std::vector<application::PaintLiveCommand> {
        std::vector<application::PaintLiveCommand> commands;

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
                        commands.reserve(rasterized.size());
                        for (const auto coordinate : rasterized) {
                            commands.push_back(application::PaintLiveCommand{coordinate});
                        }
                    }
                } else {
                    commands.push_back(application::PaintLiveCommand{*cell});
                }
                lastCell_ = cell;
            }

            if (pointer.released) {
                clearGesture();
            }
            return commands;
        }

        if (!pointer.pressed || !pointer.down || modalOwnsInput ||
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
        commands.push_back(application::PaintLiveCommand{*cell});
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
