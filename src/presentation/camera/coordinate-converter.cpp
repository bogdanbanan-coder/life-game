#include <presentation/camera/coordinate-converter.hpp>

#include <cmath>

#include <presentation/rendering/field-renderer.hpp>

namespace lifeGame::presentation {

    namespace {

        [[nodiscard]] auto snappedCellIndex(float point, float origin, float cellSize,
                                             float trailingEdge, std::size_t cellCount) noexcept
            -> std::optional<std::size_t> {
            if (cellCount == 0) {
                return std::nullopt;
            }

            std::size_t low = 0;
            auto high = cellCount;
            while (low < high) {
                const auto middle = low + (high - low) / 2;
                const auto edge = std::floor(
                    static_cast<long double>(origin) +
                    static_cast<long double>(middle) * cellSize);
                if (edge <= static_cast<long double>(point)) {
                    low = middle + 1;
                } else {
                    high = middle;
                }
            }

            if (low == 0) {
                return std::nullopt;
            }

            const auto coordinate = low - 1;
            const auto nextEdge = coordinate + 1 == cellCount
                                      ? static_cast<long double>(trailingEdge)
                                      : std::floor(static_cast<long double>(origin) +
                                                   static_cast<long double>(coordinate + 1) *
                                                       cellSize);
            if (static_cast<long double>(point) >= nextEdge) {
                return std::nullopt;
            }
            return coordinate;
        }

    } // namespace

    auto CoordinateConverter::toCell(const domain::Field& field, LogicalPoint point,
                                     int viewportWidth, int viewportHeight) noexcept
        -> std::optional<domain::CellCoordinate> {
        return toCell(field, point, viewportWidth, viewportHeight,
                      CameraController::defaultState());
    }

    auto CoordinateConverter::toCell(const domain::Field& field, LogicalPoint point,
                                     int viewportWidth, int viewportHeight,
                                     CameraState camera) noexcept
        -> std::optional<domain::CellCoordinate> {
        if (!std::isfinite(point.x) || !std::isfinite(point.y)) {
            return std::nullopt;
        }

        if (viewportWidth <= 0 || viewportHeight <= 0 || point.x < 0.0F ||
            point.x >= static_cast<float>(viewportWidth) || point.y < 0.0F ||
            point.y >= static_cast<float>(viewportHeight)) {
            return std::nullopt;
        }

        const auto plan =
            FieldRenderer::calculateRenderPlan(field, viewportWidth, viewportHeight, camera);
        const auto& rectangle = plan.fieldRectangle;
        const auto right = rectangle.x + rectangle.width;
        const auto bottom = rectangle.y + rectangle.height;
        const auto logicalRight =
            rectangle.x + static_cast<float>(field.width()) * plan.cellSize;
        const auto logicalBottom =
            rectangle.y + static_cast<float>(field.height()) * plan.cellSize;
        if (point.x < rectangle.x || point.x >= right || point.x >= logicalRight ||
            point.y < rectangle.y || point.y >= bottom || point.y >= logicalBottom) {
            return std::nullopt;
        }

        const auto cellSize = static_cast<float>(plan.cellSize);
        const auto x = snappedCellIndex(point.x, rectangle.x, cellSize,
                                        rectangle.x + rectangle.width, field.width());
        const auto y = snappedCellIndex(point.y, rectangle.y, cellSize,
                                        rectangle.y + rectangle.height, field.height());
        if (!x || !y) {
            return std::nullopt;
        }

        const auto coordinate = domain::CellCoordinate{*x, *y};
        if (!field.contains(coordinate)) {
            return std::nullopt;
        }

        return coordinate;
    }

    auto CoordinateConverter::toLogicalCellCenter(const domain::Field& field,
                                                  domain::CellCoordinate coordinate,
                                                  int viewportWidth, int viewportHeight) noexcept
        -> std::optional<LogicalPoint> {
        return toLogicalCellCenter(field, coordinate, viewportWidth, viewportHeight,
                                   CameraController::defaultState());
    }

    auto CoordinateConverter::toLogicalCellCenter(const domain::Field& field,
                                                  domain::CellCoordinate coordinate,
                                                  int viewportWidth, int viewportHeight,
                                                  CameraState camera) noexcept
        -> std::optional<LogicalPoint> {
        if (viewportWidth <= 0 || viewportHeight <= 0 || !field.contains(coordinate)) {
            return std::nullopt;
        }

        const auto plan =
            FieldRenderer::calculateRenderPlan(field, viewportWidth, viewportHeight, camera);
        const auto cellSize = static_cast<float>(plan.cellSize);
        return LogicalPoint{
            plan.fieldRectangle.x + (static_cast<float>(coordinate.x) + 0.5F) * cellSize,
            plan.fieldRectangle.y + (static_cast<float>(coordinate.y) + 0.5F) * cellSize,
        };
    }

} // namespace lifeGame::presentation
