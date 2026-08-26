#pragma once

#include <array>

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

        void render(int viewportWidth, int viewportHeight) const;
    };

} // namespace lifeGame::presentation
