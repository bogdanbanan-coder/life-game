#pragma once

#include <raylib.h>

#include <domain/field/field.hpp>

namespace lifeGame::presentation {

    struct FieldRenderPlan {
        Rectangle fieldRectangle;
        int cellSize;
        bool showGrid;
        Color deadCell;
        Color liveCell;
        Color outOfField;
        Color gridLine;
    };

    class FieldRenderer {
      public:
        [[nodiscard]] static auto calculateRenderPlan(const domain::Field& field,
                                                      int viewportWidth,
                                                      int viewportHeight) noexcept -> FieldRenderPlan;

        [[nodiscard]] static auto calculateFieldRectangle(const domain::Field& field,
                                                          int viewportWidth,
                                                          int viewportHeight) noexcept -> Rectangle;

        void render(const domain::Field& field, int viewportWidth, int viewportHeight) const;
    };

} // namespace lifeGame::presentation
