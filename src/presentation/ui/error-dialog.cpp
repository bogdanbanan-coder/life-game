#include <presentation/ui/error-dialog.hpp>

#include <algorithm>

#include <raygui.h>

namespace lifeGame::presentation {

    namespace {

        constexpr int PANEL_WIDTH = 440;
        constexpr int PANEL_HEIGHT = 176;
        constexpr int BUTTON_WIDTH = 128;
        constexpr int BUTTON_HEIGHT = 36;

        [[nodiscard]] auto contains(Rectangle bounds, LogicalPoint point) noexcept -> bool {
            return point.x >= bounds.x && point.x < bounds.x + bounds.width &&
                   point.y >= bounds.y && point.y < bounds.y + bounds.height;
        }

    } // namespace

    auto ErrorDialog::calculateLayout(int viewportWidth, int viewportHeight) noexcept
        -> ErrorDialogLayout {
        const auto safeWidth = std::max(viewportWidth, 1);
        const auto safeHeight = std::max(viewportHeight, 1);
        const auto panelX = std::max((safeWidth - PANEL_WIDTH) / 2, 0);
        const auto panelY = std::max((safeHeight - PANEL_HEIGHT) / 2, 0);
        const Rectangle panel{static_cast<float>(panelX), static_cast<float>(panelY),
                              static_cast<float>(PANEL_WIDTH), static_cast<float>(PANEL_HEIGHT)};
        const auto buttonY = panelY + PANEL_HEIGHT - BUTTON_HEIGHT - 12;
        return ErrorDialogLayout{
            panel,
            Rectangle{static_cast<float>(panelX + 16), static_cast<float>(panelY + 20),
                      static_cast<float>(PANEL_WIDTH - 32), 52.0F},
            Rectangle{static_cast<float>(panelX + PANEL_WIDTH - 2 * BUTTON_WIDTH - 24),
                      static_cast<float>(buttonY), static_cast<float>(BUTTON_WIDTH),
                      static_cast<float>(BUTTON_HEIGHT)},
            Rectangle{static_cast<float>(panelX + PANEL_WIDTH - BUTTON_WIDTH - 12),
                      static_cast<float>(buttonY), static_cast<float>(BUTTON_WIDTH),
                      static_cast<float>(BUTTON_HEIGHT)}};
    }

    auto ErrorDialog::messageFor(foundation::ErrorCode error) noexcept -> std::string_view {
        switch (error) {
        case foundation::ErrorCode::PersistenceOpenFailed:
            return "The saved data could not be opened.";
        case foundation::ErrorCode::PersistenceReadFailed:
            return "The saved data could not be read.";
        case foundation::ErrorCode::PersistenceWriteFailed:
            return "The settings could not be saved.";
        case foundation::ErrorCode::AlreadyExists:
            return "That name is already in use.";
        case foundation::ErrorCode::NotFound:
            return "The requested session was not found.";
        case foundation::ErrorCode::CapacityExceeded:
            return "The maximum number of sessions has been reached.";
        case foundation::ErrorCode::InvalidArgument:
            return "The entered value is invalid.";
        case foundation::ErrorCode::InvalidState:
            return "That action is not available right now.";
        case foundation::ErrorCode::InternalFailure:
            return "The operation could not be completed.";
        }
        return "The operation could not be completed.";
    }

    void ErrorDialog::open(foundation::ErrorCode error) noexcept {
        error_ = error;
        open_ = true;
        pendingAction_ = ErrorDialogAction::None;
    }

    void ErrorDialog::close() noexcept {
        open_ = false;
        pendingAction_ = ErrorDialogAction::None;
    }

    bool ErrorDialog::isOpen() const noexcept { return open_; }

    foundation::ErrorCode ErrorDialog::error() const noexcept { return error_; }

    ErrorDialogAction ErrorDialog::handleInput(int viewportWidth, int viewportHeight,
                                               PointerSample pointer) noexcept {
        if (!open_ || !pointer.pressed || !pointer.down) {
            return ErrorDialogAction::None;
        }

        const auto layout = calculateLayout(viewportWidth, viewportHeight);
        if (contains(layout.retry, pointer.position)) {
            pendingAction_ = ErrorDialogAction::Retry;
            return pendingAction_;
        }
        if (contains(layout.cancel, pointer.position)) {
            pendingAction_ = ErrorDialogAction::Cancel;
            return pendingAction_;
        }
        return ErrorDialogAction::None;
    }

    ErrorDialogAction ErrorDialog::consumeAction() noexcept {
        const auto action = pendingAction_;
        pendingAction_ = ErrorDialogAction::None;
        return action;
    }

    void ErrorDialog::render(int viewportWidth, int viewportHeight) const {
        if (!open_) {
            return;
        }
        const auto layout = calculateLayout(viewportWidth, viewportHeight);
        GuiPanel(layout.panel, nullptr);
        GuiLabel(layout.message, messageFor(error_).data());
        GuiButton(layout.retry, "Retry");
        GuiButton(layout.cancel, "Cancel");
    }

} // namespace lifeGame::presentation
