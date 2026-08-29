#pragma once

#include <cstddef>

#include <raylib.h>

#include <domain/field/field.hpp>
#include <presentation/camera/camera-controller.hpp>

namespace lifeGame::presentation {

    struct VisibleCellRange {
        std::size_t firstColumn = 0;
        std::size_t firstRow = 0;
        std::size_t columnCount = 0;
        std::size_t rowCount = 0;
    };

    struct FieldRenderPlan {
        Rectangle fieldRectangle;
        float cellSize;
        bool showGrid;
        Color deadCell;
        Color liveCell;
        Color outOfField;
        Color gridLine;
        CameraState camera;
        VisibleCellRange visibleCells;
    };

    class FieldRenderer {
      public:
        [[nodiscard]] static auto calculateRenderPlan(const domain::Field& field,
                                                      int viewportWidth,
                                                      int viewportHeight) noexcept -> FieldRenderPlan;

        [[nodiscard]] static auto calculateRenderPlan(const domain::Field& field,
                                                      int viewportWidth, int viewportHeight,
                                                      CameraState camera) noexcept
            -> FieldRenderPlan;

        [[nodiscard]] static auto calculateFieldRectangle(const domain::Field& field,
                                                          int viewportWidth,
                                                          int viewportHeight) noexcept -> Rectangle;

        [[nodiscard]] static auto calculateFieldRectangle(const domain::Field& field,
                                                          int viewportWidth, int viewportHeight,
                                                          CameraState camera) noexcept -> Rectangle;

        void render(const domain::Field& field, int viewportWidth, int viewportHeight) const;
        void render(const domain::Field& field, int viewportWidth, int viewportHeight,
                    CameraState camera) const;
    };

} // namespace lifeGame::presentation
