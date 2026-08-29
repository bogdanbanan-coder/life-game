#pragma once

#include <array>
#include <cstddef>
#include <string_view>

#include <application/commands/field-command.hpp>
#include <application/commands/run-command.hpp>
#include <raylib.h>

namespace lifeGame::presentation {

    struct ToolbarLayout {
        static constexpr std::size_t CONTROL_COUNT = 9;

        Rectangle panel;
        std::array<Rectangle, CONTROL_COUNT> controls;
        Rectangle status;
    };

    enum class ToolbarButtonStyle {
        Rest,
        Active,
    };

    struct ToolbarControlRender {
        std::string_view label;
        ToolbarButtonStyle style;
    };

    struct ToolbarRenderPlan {
        std::array<ToolbarControlRender, ToolbarLayout::CONTROL_COUNT> controls;
        std::string_view statusLabel;
    };

    class Toolbar {
      public:
        static constexpr int MINIMUM_CONTROL_HEIGHT = 32;
        static constexpr float ACTIVE_OUTLINE_WIDTH = 2.0F;
        static constexpr std::size_t LIVE_CONTROL_INDEX = 0;
        static constexpr std::size_t DIE_CONTROL_INDEX = 1;
        static constexpr std::size_t RUN_CONTROL_INDEX = 2;
        static constexpr std::size_t MOVE_CONTROL_INDEX = 5;
        static constexpr std::size_t EXIT_CONTROL_INDEX = 8;

        [[nodiscard]] static auto calculateLayout(int viewportWidth,
                                                   int viewportHeight) noexcept -> ToolbarLayout;

        [[nodiscard]] static auto activePaintModeControl(application::PaintMode paintMode) noexcept
            -> std::size_t;
        [[nodiscard]] static auto paintModeLabel(application::PaintMode paintMode) noexcept
            -> std::string_view;
        [[nodiscard]] static auto activeStatusLabel(application::PaintMode paintMode,
                                                     application::RunState runState) noexcept
            -> std::string_view;
        [[nodiscard]] static auto calculateRenderPlan(application::PaintMode paintMode,
                                                       application::RunState runState) noexcept
            -> ToolbarRenderPlan;
        [[nodiscard]] static auto pauseControlLabel(application::RunState runState) noexcept
            -> std::string_view;
        [[nodiscard]] static auto runStateLabel(application::RunState runState) noexcept
            -> std::string_view;

        void render(int viewportWidth, int viewportHeight,
                    application::PaintMode paintMode,
                    application::RunState runState) const;
    };

} // namespace lifeGame::presentation
