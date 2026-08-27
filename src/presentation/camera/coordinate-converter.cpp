#include <presentation/camera/coordinate-converter.hpp>

#include <cmath>

#include <presentation/rendering/field-renderer.hpp>

namespace lifeGame::presentation {

    auto CoordinateConverter::toCell(const domain::Field& field, LogicalPoint point,
                                     int viewportWidth, int viewportHeight) noexcept
        -> std::optional<domain::CellCoordinate> {
        if (!std::isfinite(point.x) || !std::isfinite(point.y)) {
            return std::nullopt;
        }

        if (viewportWidth <= 0 || viewportHeight <= 0 || point.x < 0.0F ||
            point.x >= static_cast<float>(viewportWidth) || point.y < 0.0F ||
            point.y >= static_cast<float>(viewportHeight)) {
            return std::nullopt;
        }

        const auto plan = FieldRenderer::calculateRenderPlan(field, viewportWidth, viewportHeight);
        const auto& rectangle = plan.fieldRectangle;
        const auto right = rectangle.x + rectangle.width;
        const auto bottom = rectangle.y + rectangle.height;
        if (point.x < rectangle.x || point.x >= right || point.y < rectangle.y ||
            point.y >= bottom) {
            return std::nullopt;
        }

        const auto cellSize = static_cast<float>(plan.cellSize);
        const auto x = static_cast<std::size_t>((point.x - rectangle.x) / cellSize);
        const auto y = static_cast<std::size_t>((point.y - rectangle.y) / cellSize);
        const auto coordinate = domain::CellCoordinate{x, y};
        if (!field.contains(coordinate)) {
            return std::nullopt;
        }

        return coordinate;
    }

    auto CoordinateConverter::toLogicalCellCenter(const domain::Field& field,
                                                  domain::CellCoordinate coordinate,
                                                  int viewportWidth, int viewportHeight) noexcept
        -> std::optional<LogicalPoint> {
        if (viewportWidth <= 0 || viewportHeight <= 0 || !field.contains(coordinate)) {
            return std::nullopt;
        }

        const auto plan = FieldRenderer::calculateRenderPlan(field, viewportWidth, viewportHeight);
        const auto cellSize = static_cast<float>(plan.cellSize);
        return LogicalPoint{
            plan.fieldRectangle.x + (static_cast<float>(coordinate.x) + 0.5F) * cellSize,
            plan.fieldRectangle.y + (static_cast<float>(coordinate.y) + 0.5F) * cellSize,
        };
    }

} // namespace lifeGame::presentation
