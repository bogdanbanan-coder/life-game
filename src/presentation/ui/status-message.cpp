#include <presentation/ui/status-message.hpp>

#include <utility>

#include <raygui.h>

namespace lifeGame::presentation {

    void StatusMessage::show(StatusMessageKind kind, std::string message) {
        kind_ = kind;
        text_ = std::move(message);
    }

    void StatusMessage::showInformation(std::string message) {
        show(StatusMessageKind::Information, std::move(message));
    }

    void StatusMessage::showSuccess(std::string message) {
        show(StatusMessageKind::Success, std::move(message));
    }

    void StatusMessage::showWarning(std::string message) {
        show(StatusMessageKind::Warning, std::move(message));
    }

    void StatusMessage::showError(std::string message) {
        show(StatusMessageKind::Error, std::move(message));
    }

    void StatusMessage::clear() noexcept {
        kind_ = StatusMessageKind::None;
        text_.clear();
    }

    bool StatusMessage::visible() const noexcept { return kind_ != StatusMessageKind::None; }

    StatusMessageKind StatusMessage::kind() const noexcept { return kind_; }

    const std::string& StatusMessage::text() const noexcept { return text_; }

    void StatusMessage::render(Rectangle bounds) const {
        if (visible()) {
            GuiLabel(bounds, text_.c_str());
        }
    }

} // namespace lifeGame::presentation
