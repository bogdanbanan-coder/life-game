#pragma once

#include <array>
#include <string_view>

#include <application/commands/field-command.hpp>
#include <application/commands/run-command.hpp>
#include <raylib.h>

namespace lifeGame::presentation {

    struct ToolbarLayout {
        Rectangle panel;
        std::array<Rectangle, 9> controls;
        Rectangle status;
    };

    class Toolbar {
      public:
        static constexpr int MINIMUM_CONTROL_HEIGHT = 32;

        [[nodiscard]] static auto calculateLayout(int viewportWidth,
                                                   int viewportHeight) noexcept -> ToolbarLayout;

        [[nodiscard]] static auto pauseControlLabel(application::RunState runState) noexcept
            -> std::string_view;
        [[nodiscard]] static auto runStateLabel(application::RunState runState) noexcept
            -> std::string_view;

        void render(int viewportWidth, int viewportHeight,
                    application::PaintMode paintMode,
                    application::RunState runState) const;
    };

} // namespace lifeGame::presentation
