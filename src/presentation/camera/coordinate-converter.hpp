#pragma once

#include <optional>

#include <domain/field/cell-coordinate.hpp>
#include <domain/field/field.hpp>
#include <presentation/camera/camera-controller.hpp>

namespace lifeGame::presentation {

    struct LogicalPoint {
        float x;
        float y;
    };

    class CoordinateConverter {
      public:
        [[nodiscard]] static auto toCell(const domain::Field& field, LogicalPoint point,
                                         int viewportWidth, int viewportHeight) noexcept
            -> std::optional<domain::CellCoordinate>;

        [[nodiscard]] static auto toCell(const domain::Field& field, LogicalPoint point,
                                         int viewportWidth, int viewportHeight,
                                         CameraState camera) noexcept
            -> std::optional<domain::CellCoordinate>;

        [[nodiscard]] static auto toLogicalCellCenter(
            const domain::Field& field, domain::CellCoordinate coordinate, int viewportWidth,
            int viewportHeight) noexcept -> std::optional<LogicalPoint>;

        [[nodiscard]] static auto toLogicalCellCenter(
            const domain::Field& field, domain::CellCoordinate coordinate, int viewportWidth,
            int viewportHeight, CameraState camera) noexcept -> std::optional<LogicalPoint>;
    };

} // namespace lifeGame::presentation
