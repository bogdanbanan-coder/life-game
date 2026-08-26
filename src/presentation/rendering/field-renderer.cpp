#include <presentation/rendering/field-renderer.hpp>

#include <algorithm>
#include <cstddef>

namespace lifeGame::presentation {

    namespace {

        constexpr Color DEAD_CELL{0, 0, 0, 255};
        constexpr Color LIVE_CELL{255, 255, 255, 255};
        constexpr Color OUT_OF_FIELD{128, 128, 128, 255};
        constexpr Color GRID_LINE{90, 90, 90, 255};
        constexpr int FIELD_MARGIN = 32;

        [[nodiscard]] auto cellSizeFor(const domain::Field& field, int viewportWidth,
                                       int viewportHeight) noexcept -> int {
            const auto safeWidth = std::max(viewportWidth, 1);
            const auto safeHeight = std::max(viewportHeight, 1);
            const auto width = static_cast<int>(field.width());
            const auto height = static_cast<int>(field.height());
            const auto widthCellSize = safeWidth / std::max(width, 1);
            const auto heightCellSize = safeHeight / std::max(height, 1);

            return std::max(1, std::min(widthCellSize, heightCellSize));
        }

        [[nodiscard]] auto fieldOrigin(const domain::Field& field, int viewportWidth,
                                       int viewportHeight, int cellSize) noexcept -> Vector2 {
            const auto fieldWidth = static_cast<int>(field.width()) * cellSize;
            const auto fieldHeight = static_cast<int>(field.height()) * cellSize;
            const auto safeWidth = std::max(viewportWidth, 1);
            const auto safeHeight = std::max(viewportHeight, 1);
            const auto originX = fieldWidth + FIELD_MARGIN <= safeWidth ? FIELD_MARGIN : 0;
            const auto originY = std::max((safeHeight - fieldHeight) / 2, 0);

            return Vector2{static_cast<float>(originX), static_cast<float>(originY)};
        }

    } // namespace

    auto FieldRenderer::calculateRenderPlan(const domain::Field& field, int viewportWidth,
                                            int viewportHeight) noexcept -> FieldRenderPlan {
        const auto cellSize = cellSizeFor(field, viewportWidth, viewportHeight);
        const auto origin = fieldOrigin(field, viewportWidth, viewportHeight, cellSize);
        const auto fieldWidth =
            static_cast<float>(field.width() * static_cast<std::size_t>(cellSize));
        const auto fieldHeight =
            static_cast<float>(field.height() * static_cast<std::size_t>(cellSize));

        return FieldRenderPlan{
            Rectangle{origin.x, origin.y, fieldWidth, fieldHeight},
            cellSize,
            cellSize >= 4,
            DEAD_CELL,
            LIVE_CELL,
            OUT_OF_FIELD,
            GRID_LINE,
        };
    }

    auto FieldRenderer::calculateFieldRectangle(const domain::Field& field, int viewportWidth,
                                                int viewportHeight) noexcept -> Rectangle {
        return calculateRenderPlan(field, viewportWidth, viewportHeight).fieldRectangle;
    }

    void FieldRenderer::render(const domain::Field& field, int viewportWidth,
                               int viewportHeight) const {
        const auto safeWidth = std::max(viewportWidth, 1);
        const auto safeHeight = std::max(viewportHeight, 1);
        const auto plan = calculateRenderPlan(field, safeWidth, safeHeight);
        DrawRectangle(0, 0, safeWidth, safeHeight, plan.outOfField);

        const auto cellSize = plan.cellSize;
        const auto originX = static_cast<int>(plan.fieldRectangle.x);
        const auto originY = static_cast<int>(plan.fieldRectangle.y);
        const auto width = static_cast<int>(field.width());
        const auto height = static_cast<int>(field.height());
        const auto visibleWidth = std::max(safeWidth - originX, 0);
        const auto visibleHeight = std::max(safeHeight - originY, 0);
        const auto visibleColumns = std::min(width, (visibleWidth + cellSize - 1) / cellSize);
        const auto visibleRows = std::min(height, (visibleHeight + cellSize - 1) / cellSize);

        for (int y = 0; y < visibleRows; ++y) {
            for (int x = 0; x < visibleColumns; ++x) {
                const auto coordinate = domain::CellCoordinate{static_cast<std::size_t>(x),
                                                               static_cast<std::size_t>(y)};
                const auto color = field.isLive(coordinate) ? plan.liveCell : plan.deadCell;
                DrawRectangle(originX + x * cellSize, originY + y * cellSize, cellSize, cellSize,
                              color);
            }
        }

        if (!plan.showGrid) {
            return;
        }

        const auto fieldWidth = width * cellSize;
        const auto fieldHeight = height * cellSize;
        for (int x = 0; x < visibleColumns; ++x) {
            DrawRectangle(originX + x * cellSize, originY, 1, fieldHeight, plan.gridLine);
        }
        for (int y = 0; y < visibleRows; ++y) {
            DrawRectangle(originX, originY + y * cellSize, fieldWidth, 1, plan.gridLine);
        }
        DrawRectangle(originX + fieldWidth - 1, originY, 1, fieldHeight, plan.gridLine);
        DrawRectangle(originX, originY + fieldHeight - 1, fieldWidth, 1, plan.gridLine);
    }

} // namespace lifeGame::presentation
