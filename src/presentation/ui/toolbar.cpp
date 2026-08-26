#include <presentation/ui/toolbar.hpp>

#include <algorithm>

#include <raygui.h>
#include <raylib.h>

namespace lifeGame::presentation {

    namespace {

        constexpr int PANEL_WIDTH = 304;
        constexpr int PANEL_HEIGHT = 156;
        constexpr int PANEL_MARGIN = 16;
        constexpr int PANEL_PADDING = 4;
        constexpr int CONTROL_GAP = 4;
        constexpr int COLOR_BORDER = static_cast<int>(0x8A8A8AFFU);
        constexpr int COLOR_SURFACE_BASE = static_cast<int>(0x202020FFU);
        constexpr int COLOR_CONTROL_REST = static_cast<int>(0x666666FFU);
        constexpr int COLOR_CONTROL_HOVER = static_cast<int>(0x777777FFU);
        constexpr int COLOR_CONTROL_ACTIVE = static_cast<int>(0x4A4A4AFFU);
        constexpr int COLOR_TEXT = static_cast<int>(0xFFFFFFFFU);

        void configureStyle() {
            GuiSetStyle(DEFAULT, BORDER_COLOR_NORMAL, COLOR_BORDER);
            GuiSetStyle(DEFAULT, LINE_COLOR, COLOR_BORDER);
            GuiSetStyle(DEFAULT, BACKGROUND_COLOR, COLOR_SURFACE_BASE);
            GuiSetStyle(DEFAULT, TEXT_SIZE, 14);

            GuiSetStyle(BUTTON, BORDER_COLOR_NORMAL, COLOR_BORDER);
            GuiSetStyle(BUTTON, BASE_COLOR_NORMAL, COLOR_CONTROL_REST);
            GuiSetStyle(BUTTON, TEXT_COLOR_NORMAL, COLOR_TEXT);
            GuiSetStyle(BUTTON, BORDER_COLOR_FOCUSED, COLOR_BORDER);
            GuiSetStyle(BUTTON, BASE_COLOR_FOCUSED, COLOR_CONTROL_HOVER);
            GuiSetStyle(BUTTON, TEXT_COLOR_FOCUSED, COLOR_TEXT);
            GuiSetStyle(BUTTON, BORDER_COLOR_PRESSED, COLOR_TEXT);
            GuiSetStyle(BUTTON, BASE_COLOR_PRESSED, COLOR_CONTROL_ACTIVE);
            GuiSetStyle(BUTTON, TEXT_COLOR_PRESSED, COLOR_TEXT);
            GuiSetStyle(BUTTON, BORDER_WIDTH, 1);
            GuiSetStyle(LABEL, TEXT_COLOR_NORMAL, COLOR_TEXT);
        }

        void drawButton(Rectangle bounds, const char* text) {
            static_cast<void>(GuiButton(bounds, text));
        }

        void drawActiveButton(Rectangle bounds, const char* text) {
            GuiSetStyle(BUTTON, BASE_COLOR_NORMAL, COLOR_CONTROL_ACTIVE);
            GuiSetStyle(BUTTON, BASE_COLOR_FOCUSED, COLOR_CONTROL_ACTIVE);
            static_cast<void>(GuiButton(bounds, text));
            GuiSetStyle(BUTTON, BASE_COLOR_NORMAL, COLOR_CONTROL_REST);
            GuiSetStyle(BUTTON, BASE_COLOR_FOCUSED, COLOR_CONTROL_HOVER);
            DrawRectangleLinesEx(bounds, 2.0F, WHITE);
        }

        [[nodiscard]] auto makeControl(int buttonX, int buttonWidth, int firstRowY, int column,
                                       int row) noexcept -> Rectangle {
            const auto x = buttonX + column * (buttonWidth + CONTROL_GAP);
            const auto y = firstRowY + row * (Toolbar::MINIMUM_CONTROL_HEIGHT + CONTROL_GAP);
            return Rectangle{static_cast<float>(x), static_cast<float>(y),
                             static_cast<float>(buttonWidth),
                             static_cast<float>(Toolbar::MINIMUM_CONTROL_HEIGHT)};
        }

    } // namespace

    auto Toolbar::calculateLayout(int viewportWidth, int viewportHeight) noexcept -> ToolbarLayout {
        const auto safeWidth = std::max(viewportWidth, 1);
        const auto safeHeight = std::max(viewportHeight, 1);
        const auto panelX = std::max(safeWidth - PANEL_WIDTH - PANEL_MARGIN, PANEL_MARGIN);
        const auto panelY =
            std::max(std::min(PANEL_MARGIN, safeHeight - PANEL_HEIGHT - PANEL_MARGIN), 0);
        const Rectangle panel{static_cast<float>(panelX), static_cast<float>(panelY),
                              static_cast<float>(PANEL_WIDTH), static_cast<float>(PANEL_HEIGHT)};

        const auto contentWidth = PANEL_WIDTH - 2 * PANEL_PADDING;
        const auto buttonWidth = (contentWidth - 2 * CONTROL_GAP) / 3;
        const auto buttonX = panelX + PANEL_PADDING;
        const auto firstRowY = panelY + PANEL_PADDING;
        const auto secondRowY = firstRowY + Toolbar::MINIMUM_CONTROL_HEIGHT + CONTROL_GAP;
        const auto thirdRowY = secondRowY + Toolbar::MINIMUM_CONTROL_HEIGHT + CONTROL_GAP;
        return ToolbarLayout{
            panel,
            {makeControl(buttonX, buttonWidth, firstRowY, 0, 0),
             makeControl(buttonX, buttonWidth, firstRowY, 1, 0),
             makeControl(buttonX, buttonWidth, firstRowY, 2, 0),
             makeControl(buttonX, buttonWidth, firstRowY, 0, 1),
             makeControl(buttonX, buttonWidth, firstRowY, 1, 1),
             makeControl(buttonX, buttonWidth, firstRowY, 2, 1),
             makeControl(buttonX, buttonWidth, firstRowY, 0, 2),
             makeControl(buttonX, buttonWidth, firstRowY, 1, 2),
             makeControl(buttonX, buttonWidth, firstRowY, 2, 2)},
            Rectangle{static_cast<float>(buttonX),
                      static_cast<float>(thirdRowY + Toolbar::MINIMUM_CONTROL_HEIGHT + CONTROL_GAP),
                      static_cast<float>(contentWidth), 20.0F},
        };
    }

    void Toolbar::render(int viewportWidth, int viewportHeight) const {
        configureStyle();
        const auto layout = calculateLayout(viewportWidth, viewportHeight);
        GuiPanel(layout.panel, nullptr);

        constexpr const char* LABELS[] = {"Live", "Die", "Pause", "Highlight", "Bank",
                                          "Move", "+",   "-",     "Exit"};
        drawActiveButton(layout.controls[0], LABELS[0]);
        for (std::size_t index = 1; index < layout.controls.size(); ++index) {
            drawButton(layout.controls[index], LABELS[index]);
        }

        GuiLabel(layout.status, "Running");
    }

} // namespace lifeGame::presentation
