#include <presentation/rendering/field-renderer.hpp>

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <limits>

namespace lifeGame::presentation {

    namespace {

        constexpr Color DEAD_CELL{0, 0, 0, 255};
        constexpr Color LIVE_CELL{255, 255, 255, 255};
        constexpr Color OUT_OF_FIELD{128, 128, 128, 255};
        constexpr Color GRID_LINE{90, 90, 90, 255};
        constexpr int FIELD_MARGIN = 32;

        struct BaseFieldGeometry {
            int cellSize;
            Vector2 origin;
        };

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
            const auto fieldWidth = static_cast<long long>(field.width()) * cellSize;
            const auto fieldHeight = static_cast<long long>(field.height()) * cellSize;
            const auto safeWidth = static_cast<long long>(std::max(viewportWidth, 1));
            const auto safeHeight = static_cast<long long>(std::max(viewportHeight, 1));
            const auto originX = fieldWidth + FIELD_MARGIN <= safeWidth ? FIELD_MARGIN : 0;
            const auto originY = std::max((safeHeight - fieldHeight) / 2LL, 0LL);

            return Vector2{static_cast<float>(originX), static_cast<float>(originY)};
        }

        [[nodiscard]] auto baseFieldGeometry(const domain::Field& field, int viewportWidth,
                                             int viewportHeight) noexcept -> BaseFieldGeometry {
            const auto cellSize = cellSizeFor(field, viewportWidth, viewportHeight);
            return BaseFieldGeometry{cellSize,
                                     fieldOrigin(field, viewportWidth, viewportHeight, cellSize)};
        }

        [[nodiscard]] auto clampCameraAxis(float camera, float origin, float fieldSize,
                                           int viewportSize, int cellSize) noexcept -> float {
            if (!std::isfinite(camera)) {
                camera = 0.0F;
            }

            const auto size = static_cast<float>(cellSize);
            const auto logicalFieldOrigin = origin / size;
            const auto logicalFieldSize = fieldSize / size;
            // The rectangle is snapped down to display pixels before it is used for drawing or
            // input mapping. Keep one display pixel of the finite field inside the viewport at
            // either extreme so that the snapped rectangle still has a non-empty intersection.
            const auto minimum = (origin - static_cast<float>(viewportSize - 1)) / size;
            const auto maximum = (origin + fieldSize - 1.0F) / size;

            if (minimum <= maximum) {
                auto clamped = std::clamp(camera, minimum, maximum);
                const auto hasVisibleIntersection = [origin, fieldSize, viewportSize,
                                                     size](float value) noexcept {
                    const auto snappedOrigin = std::floor(origin - value * size);
                    return snappedOrigin < static_cast<float>(viewportSize) &&
                           snappedOrigin + fieldSize > 0.0F;
                };

                if (hasVisibleIntersection(clamped)) {
                    return clamped;
                }

                const auto direction = clamped >= maximum ? -std::numeric_limits<float>::infinity()
                                                          : std::numeric_limits<float>::infinity();
                for (int attempt = 0; attempt < 8; ++attempt) {
                    clamped = std::nextafter(clamped, direction);
                    if (hasVisibleIntersection(clamped)) {
                        return clamped;
                    }
                }

                return minimum + (maximum - minimum) / 2.0F;
            }

            return (logicalFieldOrigin + logicalFieldSize) / 2.0F;
        }

        [[nodiscard]] auto clampCamera(const domain::Field& field, int viewportWidth,
                                       int viewportHeight, CameraState camera) noexcept
            -> CameraState {
            const auto safeWidth = std::max(viewportWidth, 1);
            const auto safeHeight = std::max(viewportHeight, 1);
            const auto geometry = baseFieldGeometry(field, safeWidth, safeHeight);
            const auto fieldWidth = static_cast<float>(field.width() *
                                                       static_cast<std::size_t>(geometry.cellSize));
            const auto fieldHeight = static_cast<float>(
                field.height() * static_cast<std::size_t>(geometry.cellSize));

            return CameraState{
                clampCameraAxis(camera.x, geometry.origin.x, fieldWidth, safeWidth,
                                geometry.cellSize),
                clampCameraAxis(camera.y, geometry.origin.y, fieldHeight, safeHeight,
                                geometry.cellSize),
            };
        }

        [[nodiscard]] auto clampIndex(long long value, std::size_t limit) noexcept -> std::size_t {
            if (value <= 0) {
                return 0;
            }

            const auto maximum = static_cast<long long>(limit);
            if (value >= maximum) {
                return limit;
            }

            return static_cast<std::size_t>(value);
        }

        [[nodiscard]] auto visibleCellRange(const Rectangle& fieldRectangle, int cellSize,
                                            std::size_t fieldWidth, std::size_t fieldHeight,
                                            int viewportWidth, int viewportHeight) noexcept
            -> VisibleCellRange {
            const auto size = static_cast<float>(cellSize);
            const auto firstColumn = clampIndex(
                static_cast<long long>(std::floor(-fieldRectangle.x / size)), fieldWidth);
            const auto firstRow = clampIndex(
                static_cast<long long>(std::floor(-fieldRectangle.y / size)), fieldHeight);
            const auto lastColumn = clampIndex(
                static_cast<long long>(std::ceil((static_cast<float>(viewportWidth) -
                                                  fieldRectangle.x) /
                                                 size)),
                fieldWidth);
            const auto lastRow = clampIndex(
                static_cast<long long>(std::ceil((static_cast<float>(viewportHeight) -
                                                  fieldRectangle.y) /
                                                 size)),
                fieldHeight);

            return VisibleCellRange{
                firstColumn,
                firstRow,
                lastColumn > firstColumn ? lastColumn - firstColumn : 0,
                lastRow > firstRow ? lastRow - firstRow : 0,
            };
        }

        [[nodiscard]] auto cellScreenPosition(float fieldOrigin, std::size_t coordinate,
                                              int cellSize) noexcept -> int {
            const auto position = fieldOrigin + static_cast<float>(coordinate) *
                                                      static_cast<float>(cellSize);
            return static_cast<int>(std::floor(position));
        }

    } // namespace

    auto FieldRenderer::calculateRenderPlan(const domain::Field& field, int viewportWidth,
                                            int viewportHeight) noexcept -> FieldRenderPlan {
        return calculateRenderPlan(field, viewportWidth, viewportHeight,
                                   CameraController::defaultState());
    }

    auto FieldRenderer::calculateRenderPlan(const domain::Field& field, int viewportWidth,
                                            int viewportHeight, CameraState camera) noexcept
        -> FieldRenderPlan {
        const auto safeWidth = std::max(viewportWidth, 1);
        const auto safeHeight = std::max(viewportHeight, 1);
        const auto geometry = baseFieldGeometry(field, safeWidth, safeHeight);
        const auto clampedCamera = clampCamera(field, safeWidth, safeHeight, camera);
        const auto origin = Vector2{
            geometry.origin.x - clampedCamera.x * static_cast<float>(geometry.cellSize),
            geometry.origin.y - clampedCamera.y * static_cast<float>(geometry.cellSize),
        };
        const auto fieldWidth =
            static_cast<float>(field.width() * static_cast<std::size_t>(geometry.cellSize));
        const auto fieldHeight =
            static_cast<float>(field.height() * static_cast<std::size_t>(geometry.cellSize));
        const auto fieldRectangle = Rectangle{
            static_cast<float>(std::floor(origin.x)),
            static_cast<float>(std::floor(origin.y)),
            fieldWidth,
            fieldHeight,
        };

        return FieldRenderPlan{
            fieldRectangle,
            geometry.cellSize,
            geometry.cellSize >= 4,
            DEAD_CELL,
            LIVE_CELL,
            OUT_OF_FIELD,
            GRID_LINE,
            clampedCamera,
            visibleCellRange(fieldRectangle, geometry.cellSize, field.width(), field.height(),
                             safeWidth, safeHeight),
        };
    }

    auto FieldRenderer::calculateFieldRectangle(const domain::Field& field, int viewportWidth,
                                                int viewportHeight) noexcept -> Rectangle {
        return calculateRenderPlan(field, viewportWidth, viewportHeight).fieldRectangle;
    }

    auto FieldRenderer::calculateFieldRectangle(const domain::Field& field, int viewportWidth,
                                                int viewportHeight,
                                                CameraState camera) noexcept -> Rectangle {
        return calculateRenderPlan(field, viewportWidth, viewportHeight, camera).fieldRectangle;
    }

    void FieldRenderer::render(const domain::Field& field, int viewportWidth,
                               int viewportHeight) const {
        render(field, viewportWidth, viewportHeight, CameraController::defaultState());
    }

    void FieldRenderer::render(const domain::Field& field, int viewportWidth, int viewportHeight,
                               CameraState camera) const {
        const auto safeWidth = std::max(viewportWidth, 1);
        const auto safeHeight = std::max(viewportHeight, 1);
        const auto plan = calculateRenderPlan(field, safeWidth, safeHeight, camera);
        DrawRectangle(0, 0, safeWidth, safeHeight, plan.outOfField);

        const auto cellSize = plan.cellSize;
        const auto& visible = plan.visibleCells;
        const auto lastColumn = visible.firstColumn + visible.columnCount;
        const auto lastRow = visible.firstRow + visible.rowCount;

        for (std::size_t y = visible.firstRow; y < lastRow; ++y) {
            for (std::size_t x = visible.firstColumn; x < lastColumn; ++x) {
                const auto coordinate = domain::CellCoordinate{x, y};
                const auto color = field.isLive(coordinate) ? plan.liveCell : plan.deadCell;
                DrawRectangle(cellScreenPosition(plan.fieldRectangle.x, x, cellSize),
                              cellScreenPosition(plan.fieldRectangle.y, y, cellSize), cellSize,
                              cellSize, color);
            }
        }

        if (!plan.showGrid) {
            return;
        }

        const auto gridLeft = std::max(
            0, cellScreenPosition(plan.fieldRectangle.x, visible.firstColumn, cellSize));
        const auto gridTop = std::max(
            0, cellScreenPosition(plan.fieldRectangle.y, visible.firstRow, cellSize));
        const auto gridRight = std::min(
            safeWidth, cellScreenPosition(plan.fieldRectangle.x, lastColumn, cellSize));
        const auto gridBottom = std::min(
            safeHeight, cellScreenPosition(plan.fieldRectangle.y, lastRow, cellSize));

        for (std::size_t x = visible.firstColumn; x < lastColumn; ++x) {
            const auto screenX = cellScreenPosition(plan.fieldRectangle.x, x, cellSize);
            if (screenX >= 0 && screenX < safeWidth && gridBottom > gridTop) {
                DrawRectangle(screenX, gridTop, 1, gridBottom - gridTop, plan.gridLine);
            }
        }

        for (std::size_t y = visible.firstRow; y < lastRow; ++y) {
            const auto screenY = cellScreenPosition(plan.fieldRectangle.y, y, cellSize);
            if (screenY >= 0 && screenY < safeHeight && gridRight > gridLeft) {
                DrawRectangle(gridLeft, screenY, gridRight - gridLeft, 1, plan.gridLine);
            }
        }

        const auto fieldRight = cellScreenPosition(plan.fieldRectangle.x, field.width(), cellSize);
        if (fieldRight > 0 && fieldRight <= safeWidth && gridBottom > gridTop) {
            DrawRectangle(fieldRight - 1, gridTop, 1, gridBottom - gridTop, plan.gridLine);
        }

        const auto fieldBottom = cellScreenPosition(plan.fieldRectangle.y, field.height(), cellSize);
        if (fieldBottom > 0 && fieldBottom <= safeHeight && gridRight > gridLeft) {
            DrawRectangle(gridLeft, fieldBottom - 1, gridRight - gridLeft, 1, plan.gridLine);
        }
    }

} // namespace lifeGame::presentation
