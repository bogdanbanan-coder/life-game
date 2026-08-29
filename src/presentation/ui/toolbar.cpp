#include <presentation/ui/toolbar.hpp>

#include <algorithm>
#include <exception>

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

        struct PaintModeControl {
            application::PaintMode mode;
            std::size_t controlIndex;
            std::string_view label;
            std::string_view runningStatus;
            std::string_view pausedStatus;
        };

        constexpr std::array PAINT_MODE_CONTROLS{
            PaintModeControl{application::PaintMode::Live, Toolbar::LIVE_CONTROL_INDEX, "Live",
                             "Active: Live | Running", "Active: Live | Paused"},
            PaintModeControl{application::PaintMode::Die, Toolbar::DIE_CONTROL_INDEX, "Die",
                             "Active: Die | Running", "Active: Die | Paused"},
            PaintModeControl{application::PaintMode::Move, Toolbar::MOVE_CONTROL_INDEX, "Move",
                             "Active: Move | Running", "Active: Move | Paused"},
        };

        [[nodiscard]] auto paintModeControl(application::PaintMode paintMode) noexcept
            -> const PaintModeControl& {
            for (const auto& control : PAINT_MODE_CONTROLS) {
                if (control.mode == paintMode) {
                    return control;
                }
            }
            std::terminate();
        }

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
            DrawRectangleLinesEx(bounds, Toolbar::ACTIVE_OUTLINE_WIDTH, WHITE);
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

    auto Toolbar::activePaintModeControl(application::PaintMode paintMode) noexcept
        -> std::size_t {
        return paintModeControl(paintMode).controlIndex;
    }

    auto Toolbar::paintModeLabel(application::PaintMode paintMode) noexcept -> std::string_view {
        return paintModeControl(paintMode).label;
    }

    auto Toolbar::activeStatusLabel(application::PaintMode paintMode,
                                     application::RunState runState) noexcept
        -> std::string_view {
        const auto& control = paintModeControl(paintMode);
        switch (runState) {
        case application::RunState::Running:
            return control.runningStatus;
        case application::RunState::Paused:
            return control.pausedStatus;
        }
        std::terminate();
    }

    auto Toolbar::calculateRenderPlan(application::PaintMode paintMode,
                                       application::RunState runState) noexcept
        -> ToolbarRenderPlan {
        ToolbarRenderPlan plan{
            {{ToolbarControlRender{paintModeLabel(application::PaintMode::Live),
                                   ToolbarButtonStyle::Rest},
              ToolbarControlRender{paintModeLabel(application::PaintMode::Die),
                                   ToolbarButtonStyle::Rest},
              ToolbarControlRender{pauseControlLabel(runState), ToolbarButtonStyle::Rest},
              ToolbarControlRender{"Highlight", ToolbarButtonStyle::Rest},
              ToolbarControlRender{"Bank", ToolbarButtonStyle::Rest},
              ToolbarControlRender{"Move", ToolbarButtonStyle::Rest},
              ToolbarControlRender{"+", ToolbarButtonStyle::Rest},
              ToolbarControlRender{"-", ToolbarButtonStyle::Rest},
              ToolbarControlRender{"Exit", ToolbarButtonStyle::Rest}}},
            activeStatusLabel(paintMode, runState)};
        plan.controls[activePaintModeControl(paintMode)].style = ToolbarButtonStyle::Active;
        return plan;
    }

    auto Toolbar::pauseControlLabel(application::RunState runState) noexcept -> std::string_view {
        switch (runState) {
        case application::RunState::Running:
            return "Pause";
        case application::RunState::Paused:
            return "Resume";
        }
        std::terminate();
    }

    auto Toolbar::runStateLabel(application::RunState runState) noexcept -> std::string_view {
        switch (runState) {
        case application::RunState::Running:
            return "Running";
        case application::RunState::Paused:
            return "Paused";
        }
        std::terminate();
    }

    void Toolbar::render(int viewportWidth, int viewportHeight,
                         application::PaintMode paintMode, application::RunState runState) const {
        configureStyle();
        const auto layout = calculateLayout(viewportWidth, viewportHeight);
        const auto renderPlan = calculateRenderPlan(paintMode, runState);
        GuiPanel(layout.panel, nullptr);

        for (std::size_t index = 0; index < layout.controls.size(); ++index) {
            const auto& control = renderPlan.controls[index];
            if (control.style == ToolbarButtonStyle::Active) {
                drawActiveButton(layout.controls[index], control.label.data());
            } else {
                drawButton(layout.controls[index], control.label.data());
            }
        }

        GuiLabel(layout.status, renderPlan.statusLabel.data());
    }

} // namespace lifeGame::presentation
